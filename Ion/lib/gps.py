"""u-blox MAX-M10S GNSS driver for the BeagleBone Green (I2C / DDC port, address 0x42).

How it works
------------
The module's DDC port is a byte stream: registers 0xFD/0xFE hold the number of bytes
waiting (big-endian u16) and register 0xFF yields the stream itself.
_DdcStream wraps that behaviour in a file-like object with a blocking read(n), which is
the stream contract pyubx2's UBXReader expects.
Transfers use raw i2c_rdwr calls, which bypass SMBus' 32-byte block limit. There are no
threads: all I2C traffic happens inside the caller's read_* / configure calls.

Example::

    with GnssDriver(bus=1) as gnss:
        gnss.configure()
        while (fix := gnss.read_fix()) is not None:
            print(fix.lat, fix.lon, fix.num_sv, fix.valid)
"""
from __future__ import annotations

import logging
import time
from contextlib import ExitStack
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Protocol, Sequence

from pyubx2 import ERR_LOG, SET_LAYER_RAM, TXN_NONE, UBX_PROTOCOL, UBXMessage, UBXReader
from smbus3 import SMBus, i2c_msg

logger = logging.getLogger(__name__)

_ADDR = 0x42
_REG_AVAIL = 0xFD  # 0xFD (MSB) / 0xFE (LSB): bytes waiting in the module's output buffer
_REG_STREAM = 0xFF  # reading here pops bytes from the output buffer
_CHUNK = 250  # arbitrary cap per I2C transfer, just to keep each transaction short
_CFG_VALSET = (0x06, 0x8A)  # (class, id) of UBX-CFG-VALSET, used to match its ACK/NAK


class I2CBus(Protocol):
    """Structural type for the I2C bus: anything with ``i2c_rdwr`` (e.g. ``smbus3.SMBus`` or a mock)."""

    def i2c_rdwr(self, *i2c_msgs: i2c_msg) -> None: ...


class GnssConfigError(RuntimeError):
    """Raised when the module rejects a configuration command (UBX-ACK-NAK)."""


@dataclass(frozen=True, slots=True)
class GnssFix:
    """Position/velocity/time solution decoded from a UBX-NAV-PVT message.

    Attributes:
        lat, lon: WGS84 position in degrees.
        alt_msl_m: Height above mean sea level in metres.
        speed_ms: Ground speed in m/s.
        heading_deg: Heading of motion in degrees (only meaningful while moving).
        h_acc_m, v_acc_m: Estimated horizontal / vertical accuracy in metres.
        pdop: Position dilution of precision.
        fix_type: 0 none, 1 dead-reckoning, 2 2D, 3 3D, 4 GNSS+DR, 5 time only.
        num_sv: Number of satellites used in the solution.
        valid: True if the module flags the fix as OK and the position as valid.
        utc_time: UTC timestamp, or None if the module reports date/time as invalid.
    """

    lat: float
    lon: float
    alt_msl_m: float
    speed_ms: float
    heading_deg: float
    h_acc_m: float
    v_acc_m: float
    pdop: float
    fix_type: int
    num_sv: int
    valid: bool
    utc_time: datetime | None

    @classmethod
    def from_nav_pvt(cls, msg: UBXMessage) -> GnssFix:
        """Build a fix from a parsed UBX-NAV-PVT message, converting mm and mm/s to SI units."""
        utc_time = None
        if msg.validDate and msg.validTime:
            # second can be 60 during a leap second; datetime cannot represent that.
            utc_time = datetime(msg.year, msg.month, msg.day, msg.hour, msg.min,
                                min(msg.second, 59), tzinfo=timezone.utc)
        return cls(
            lat=msg.lat, lon=msg.lon,
            alt_msl_m=msg.hMSL / 1000, speed_ms=msg.gSpeed / 1000, heading_deg=msg.headMot,
            h_acc_m=msg.hAcc / 1000, v_acc_m=msg.vAcc / 1000, pdop=msg.pDOP,
            fix_type=msg.fixType, num_sv=msg.numSV,
            valid=bool(msg.gnssFixOk and not msg.invalidLlh),
            utc_time=utc_time,
        )


class _DdcStream:
    """File-like adapter exposing the module's DDC output as a blocking byte stream."""

    def __init__(self, bus: I2CBus, address: int, timeout: float, poll_s: float) -> None:
        self._bus, self._addr, self._timeout, self._poll_s = bus, address, timeout, poll_s
        self._buf = bytearray()

    def available(self) -> int:
        """Return how many bytes the module has waiting. Raises OSError on I2C failure."""
        count = i2c_msg.read(self._addr, 2)
        self._bus.i2c_rdwr(i2c_msg.write(self._addr, [_REG_AVAIL]), count)
        return int.from_bytes(bytes(count), "big")

    def read(self, n: int) -> bytes:
        """Return exactly ``n`` bytes, or fewer if the module stays silent for ``timeout`` s.

        A short (or empty) result is how pyubx2 detects end-of-stream, so a quiet module
        surfaces as "no message" rather than blocking forever.
        """
        deadline = time.monotonic() + self._timeout
        while len(self._buf) < n:
            waiting = self.available()
            if waiting:
                self._fetch(min(waiting, _CHUNK))
            elif time.monotonic() >= deadline:
                break
            else:
                time.sleep(self._poll_s)
        data, self._buf = bytes(self._buf[:n]), self._buf[n:]
        return data

    def write(self, data: bytes) -> None:
        """Send raw bytes to the module (DDC writes carry no register byte)."""
        for i in range(0, len(data), _CHUNK):
            self._bus.i2c_rdwr(i2c_msg.write(self._addr, list(data[i:i + _CHUNK])))

    def _fetch(self, n: int) -> None:
        """Pop ``n`` bytes from the module into the local buffer."""
        data = i2c_msg.read(self._addr, n)
        self._bus.i2c_rdwr(i2c_msg.write(self._addr, [_REG_STREAM]), data)
        self._buf += bytes(data)


class GnssDriver:
    """UBX driver for a MAX-M10S on I2C. Must be used as a context manager.

    The bus is only opened in ``__enter__`` and released in ``__exit__``. If a bus
    *object* is passed in, the caller keeps ownership and it is not closed here.
    """

    def __init__(self, bus: int | I2CBus = 1, address: int = _ADDR,
                 timeout: float = 2.0, poll_s: float = 0.01) -> None:
        """Store settings; no hardware is touched until ``__enter__``.

        Args:
            bus: I2C bus number (1 for /dev/i2c-1) or an object satisfying ``I2CBus``.
            address: 7-bit I2C address of the module.
            timeout: Seconds without data before a ``read_*`` call gives up.
            poll_s: Sleep between polls while the module has nothing to send.
        """
        self._bus_arg, self._addr, self._timeout, self._poll_s = bus, address, timeout, poll_s
        self._cleanup = ExitStack()
        self._stream: _DdcStream | None = None
        self._reader: UBXReader | None = None

    def __enter__(self) -> GnssDriver:
        with ExitStack() as stack:  # releases the bus automatically if setup fails midway
            bus = self._bus_arg
            if isinstance(bus, int):
                bus = stack.enter_context(SMBus(bus))
            self._stream = _DdcStream(bus, self._addr, self._timeout, self._poll_s)
            self._reader = UBXReader(self._stream, protfilter=UBX_PROTOCOL, quitonerror=ERR_LOG)
            self._cleanup = stack.pop_all()
        logger.info("GnssDriver: opened bus %s, addr 0x%02X", self._bus_arg, self._addr)
        return self

    def __exit__(self, *exc: object) -> None:
        self._cleanup.close()
        self._stream = self._reader = None

    def _open(self) -> tuple[_DdcStream, UBXReader]:
        """Return the live stream and reader, or raise if the driver is not entered."""
        if self._stream is None or self._reader is None:
            raise RuntimeError("GnssDriver must be used as: with GnssDriver(...) as gnss")
        return self._stream, self._reader

    def configure(self, rate_ms: int = 1000, messages: Sequence[str] = ("NAV_PVT", "NAV_SAT"),
                  layers: int = SET_LAYER_RAM) -> None:
        """Configure the module with UBX-CFG-VALSET and wait for its acknowledgement.

        Turns NMEA off and UBX on for the DDC port, sets the measurement period, and
        enables the requested periodic UBX messages (one per navigation epoch).

        Args:
            rate_ms: Measurement period in ms (1000 = 1 Hz).
            messages: UBX message names as used in u-blox config keys, e.g. "NAV_PVT",
                "NAV_SAT", "NAV_DOP". "NAV_PVT" is required for ``read_fix``.
            layers: pyubx2 ``SET_LAYER_*`` flags (OR-able). RAM is lost on power cycle;
                add ``SET_LAYER_BBR`` to persist while the backup supply is present.

        Raises:
            GnssConfigError: The module rejected the command (UBX-ACK-NAK).
            TimeoutError: No acknowledgement arrived in time.
            OSError: I2C failure.
        """
        stream, _ = self._open()
        settings = [("CFG_I2COUTPROT_NMEA", 0), ("CFG_I2COUTPROT_UBX", 1), ("CFG_RATE_MEAS", rate_ms)]
        settings += [(f"CFG_MSGOUT_UBX_{name}_I2C", 1) for name in messages]
        stream.write(UBXMessage.config_set(layers, TXN_NONE, settings).serialize())
        self._wait_ack(*_CFG_VALSET)

    def read_message(self) -> UBXMessage | None:
        """Return the next UBX message (check ``.identity``, e.g. "NAV-SAT"), or None on timeout.

        Use this for the full GNSS data set (satellite info, DOP, ...). Malformed frames
        are logged and skipped by pyubx2. Raises OSError on I2C failure.
        """
        _, reader = self._open()
        _, msg = reader.read()
        return msg if isinstance(msg, UBXMessage) else None

    def read_fix(self) -> GnssFix | None:
        """Return the next position fix, or None if no NAV-PVT arrives within ``timeout``.

        Other message types received in the meantime are discarded; use
        ``read_message`` if you need them. Raises OSError on I2C failure.
        """
        deadline = time.monotonic() + self._timeout
        while time.monotonic() < deadline:
            msg = self.read_message()
            if msg is None:
                return None
            if msg.identity == "NAV-PVT":
                return GnssFix.from_nav_pvt(msg)
        return None

    def ping(self) -> bool:
        """Return True if the module answers a read of its DDC byte-count register."""
        stream, _ = self._open()
        try:
            stream.available()
            return True
        except OSError:
            return False

    def _wait_ack(self, cls_id: int, msg_id: int) -> None:
        """Consume messages until the ACK/NAK for (cls_id, msg_id) arrives."""
        deadline = time.monotonic() + self._timeout
        while time.monotonic() < deadline:
            msg = self.read_message()
            if msg is None:
                break
            if msg.identity in ("ACK-ACK", "ACK-NAK") and (msg.clsID, msg.msgID) == (cls_id, msg_id):
                if msg.identity == "ACK-NAK":
                    raise GnssConfigError("module rejected the configuration (UBX-ACK-NAK)")
                return
        raise TimeoutError("no acknowledgement from the module")

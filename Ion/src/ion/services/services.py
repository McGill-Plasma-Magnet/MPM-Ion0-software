from olaf import Service

"""OLAF wrappers for the systemd services"""

class BatteryHeaterService(Service):
    def __init__(self):
        super().__init__()
        self.systemd_unit = "batteryheater.service"

class LogsMainService(Service):
    def __init__(self):
        super().__init__()
        self.systemd_unit = "logsmain.service"

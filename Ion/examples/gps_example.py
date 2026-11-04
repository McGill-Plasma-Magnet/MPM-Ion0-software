from gps import *

with GnssDriver(bus=1) as gnss:
    gnss.configure()
    while (fix := gnss.read_fix()) is not None:
        print(fix.lat, fix.lon, fix.num_sv, fix.valid)


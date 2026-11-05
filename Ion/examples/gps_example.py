from ion.lib.gps import *

with GnssDriver(bus=1) as gnss:
    gnss.configure()
    while (fix := gnss.read_fix()) is not None:
        # ping to separate reception from HW issues
        if gnss.ping():
            print("chip responding!")
        else:
            print("unresponsive")
        # latitude, longitude, number of satelites, valid signal?
        print(fix.lat, fix.lon, fix.num_sv, fix.valid)


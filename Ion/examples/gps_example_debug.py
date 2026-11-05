from gps import GnssDriver

def main():
    try:
        # Use bus=1 for BeagleBone Green / Black I2C-1
        with GnssDriver(bus=1) as gnss:
            # Request a broad set of messages to see if the chip is alive
            # NAV_PVT: Position/Velocity/Time
            # NAV_SAT: Satellite status (critical for debugging locks)
            # NAV_DOP: Dilution of Precision
            print("Configuring GNSS module...")
            gnss.configure(messages=("NAV_PVT", "NAV_SAT", "NAV_DOP"))
            
            print("\n--- Debug Mode: Listening for raw UBX messages ---")
            print("Press Ctrl+C to stop\n")
            
            while True:
                msg = gnss.read_message()
                if msg is not None:
                    # .identity gives the name (e.g., 'NAV-PVT')
                    # .payload contains the raw binary data
                    print(f"[{msg.identity}]")
                    print(f"  Payload: {msg.payload.hex(' ')}")
                    
                    # Special check for NAV-SAT to see if any satellites are being tracked
                    if msg.identity == "NAV-SAT":
                        # In a real scenario, we'd parse this, but for debug 
                        # just knowing we get this message means the antenna is working.
                        print("  Status: Receiving satellite data! (Antenna is working)")
                    
                    print("-" * 40)
                    
    except KeyboardInterrupt:
        print("\nDebug stopped by user.")
    except Exception as e:
        print(f"\nAn error occurred: {e}")

if __name__ == "__main__":
    main()

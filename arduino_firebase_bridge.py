"""
ITFOS - Arduino to Firebase Bridge
DSATM BEEXC208 - 2025-26

This script reads sensor distance data from Arduino via Serial port
and pushes it live to Firebase Realtime Database, so the ITFOS
web dashboard can display real hardware sensor readings.

BEFORE RUNNING:
1. Update SERIAL_PORT below to match your Arduino's COM port
2. Update FIREBASE_KEY_PATH to point to your downloaded service account JSON file
3. Update DATABASE_URL to match your Firebase project
4. Make sure Arduino is connected and running the sensor code
   (the one that prints ROAD1:xx / ROAD2:xx / ROAD3:xx)

TO RUN:
    python arduino_firebase_bridge.py
"""

import serial
import time
import re
import firebase_admin
from firebase_admin import credentials, db

# ============================================================
#  CONFIGURATION - CHANGE THESE VALUES
# ============================================================

SERIAL_PORT = "COM5"          # <-- Change to your Arduino's COM port
BAUD_RATE = 9600

FIREBASE_KEY_PATH = "itfos-6aa1d-firebase-adminsdk-fbsvc-eeb3e978b1.json"   # <-- Path to your downloaded key file
DATABASE_URL = "https://itfos-6aa1d-default-rtdb.asia-southeast1.firebasedatabase.app"

# ============================================================
#  STEP 1: CONNECT TO FIREBASE
# ============================================================

print("Connecting to Firebase...")
cred = credentials.Certificate(FIREBASE_KEY_PATH)
firebase_admin.initialize_app(cred, {
    'databaseURL': DATABASE_URL
})
print("Firebase connected successfully!")

# Reference to where we'll store live sensor data
arduino_ref = db.reference('arduino_live')

# ============================================================
#  STEP 2: CONNECT TO ARDUINO VIA SERIAL PORT
# ============================================================

print(f"Connecting to Arduino on {SERIAL_PORT}...")
arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Wait for Arduino to reset after serial connection opens
print("Arduino connected successfully!")
print("Reading sensor data... (Press Ctrl+C to stop)")
print("-" * 50)

# ============================================================
#  STEP 3: READ SERIAL DATA AND PUSH TO FIREBASE
# ============================================================

# We'll store the latest reading for each road here
latest_readings = {
    "road1": None,
    "road2": None,
    "road3": None,
    "priority": None,
    "timestamp": None
}

# Matches lines like:
# "R1 = 22.15 cm    R2 = 999.00 cm    R3 = 33.86 cm    GREEN: Road 1"
LINE_PATTERN = re.compile(
    r"R1\s*=\s*([\d.]+)\s*cm\s+R2\s*=\s*([\d.]+)\s*cm\s+R3\s*=\s*([\d.]+)\s*cm\s+GREEN:\s*Road\s*(\d)"
)

try:
    while True:
        line = arduino.readline().decode('utf-8', errors='ignore').strip()

        if not line:
            continue  # skip empty lines

        print(f"Arduino says: {line}")

        # Try to match the full combined line format your Arduino prints
        match = LINE_PATTERN.search(line)
        if match:
            latest_readings["road1"] = float(match.group(1))
            latest_readings["road2"] = float(match.group(2))
            latest_readings["road3"] = float(match.group(3))
            latest_readings["priority"] = f"Road {match.group(4)}"
            latest_readings["timestamp"] = int(time.time() * 1000)  # milliseconds

            arduino_ref.set(latest_readings)
            print(f"✅ Pushed to Firebase: Road1={latest_readings['road1']}cm, "
                  f"Road2={latest_readings['road2']}cm, "
                  f"Road3={latest_readings['road3']}cm, "
                  f"Priority={latest_readings['priority']}")
            print("-" * 50)

except KeyboardInterrupt:
    print("\n\nStopping bridge... Closing connections.")
    arduino.close()
    print("Done. Goodbye!")

except serial.SerialException as e:
    print(f"\n❌ Serial connection error: {e}")
    print("Check that:")
    print("  1. Arduino is properly connected via USB")
    print("  2. The COM port number is correct")
    print("  3. Arduino IDE Serial Monitor is CLOSED (only one program can use the port at a time)")

except Exception as e:
    print(f"\n❌ Error: {e}")

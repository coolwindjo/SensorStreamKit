#!/usr/bin/env python3
import subprocess
import time
import sys
import os

def run_test():
    print("=== Starting Integration Test ===")
    
    # Paths to executables (adjust based on your build directory structure)
    build_dir = "./build/examples"
    broker_exe = os.path.join(build_dir, "broker")
    sub_exe = os.path.join(build_dir, "simple_subscriber")
    pub_exe = os.path.join(build_dir, "simple_publisher")

    # 1. Start Broker
    print(f"Starting Broker: {broker_exe}")
    broker = subprocess.Popen([broker_exe], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    time.sleep(1) # Wait for broker to bind ports

    # 2. Start Subscriber
    print(f"Starting Subscriber: {sub_exe}")
    sub = subprocess.Popen([sub_exe], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    time.sleep(1) # Wait for sub to connect

    # 3. Start Publisher
    print(f"Starting Publisher: {pub_exe}")
    pub = subprocess.Popen([pub_exe], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    # 4. Let them run for a few seconds
    print("Running for 5 seconds...")
    time.sleep(5)

    # 5. Terminate processes
    pub.terminate()
    sub.terminate()
    broker.terminate()

    # 6. Check output
    sub_out, sub_err = sub.communicate() # stdout is captured here because of PIPE
    
    if "Received frame" in sub_out:
        print("\nSUCCESS: Subscriber received messages from Publisher via Broker.")
    else:
        print("\nFAILURE: Subscriber did not receive expected messages.")
        print(f"Subscriber STDOUT:\n{sub_out}")
        print(f"Subscriber STDERR:\n{sub_err}")
        sys.exit(1)

if __name__ == "__main__":
    run_test()
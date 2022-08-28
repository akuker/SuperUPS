import time
import smbus
import sys

i2c_ch = 1

i2c_address = 0x59

# Register addresses
reg_temp = 0x00
reg_config = 0x01


# Initialize I2C (SMBus)
bus = smbus.SMBus(i2c_ch)

# Flush the buffer
#for x in range(130):
#    val = bus.read_byte_data(i2c_address, reg_config)

while True:
    val = bus.read_byte_data(i2c_address, reg_config)
    if val == 0xff:
        sys.stdout.flush()
        #print(".",end="")
        continue
        #time.sleep(0.01)
    else:
        print(chr(val), end="")


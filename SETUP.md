# How to setup SuperUPS

The following assumes you're starting with the base RaSCSI release image from https://github.com/akuker/RaSCSI

## Enable I2C
```
sudo raspi-config
```
Navigate to Interfaces Options -> I2C
When asked if you want I2C to be enabled, select "Yes"

## Install i2c-tools
```
sudo apt install i2c-tools -y
```

## Check if the SuperUPS board is working
```
pi@rascsi:~ $ sudo i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- 52 -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --                         
pi@rascsi:~ $ 
```

The SuperUPS should be at i2c address 0x52

Now, check that you can properly read from it. Registers 16 and 17 are the resistor values of the voltage divider on the board. They should match the example below.
```
pi@rascsi:~ $ i2cget -y 1 0x52 16
0x64
pi@rascsi:~ $ i2cget -y 1 0x52 17
0x16
pi@rascsi:~ $ 
```

## Download the SuperUPS Github repo
```
pi@rascsi:~ $ git clone https://github.com/akuker/SuperUPS.git
```

## Install Python packages
```
sudo pip3 install python-daemon
sudo pip3 install smbus
```

## To manually start the UPS monitor
```
cd ~/SuperUPS/host_monitor/
python3 ./upsmonitor.py
```

## To dump all of the registers from the UPS controller
```
python3 ups.py
```


## To setup the UPS monitor to run automatically
```
sudo cp SuperUPS/host_monitor/ups*.py /usr/local/bin/
sudo cp SuperUPS/host_monitor/superups.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable superups.service
sudo systemctl start superups
```


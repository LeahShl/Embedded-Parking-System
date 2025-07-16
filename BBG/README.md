# BeagleBone Black/Green Parking System Client
This client is part of a larger smart parking system and is responsible for interfacing with an STM32-based GPS module and a TCP server. I developed it on a BeagleBone Green (BBG) and it handles the reception of parking events (START and STOP) via I2C, then transmits them to a remote server over Ethernet.

## Overview
The client program installs itself on its first run, then runs as a daemon and restarts on startup. It reads messages from I2C and forwards them to a TCP server over Ethernet.

### File structure
key:
* File
* \[Directory\]
```
[BBG]
├── [Inc]
│   ├── config.h          # Config file operations header
│   ├── eth_process.h     # Ethernet process header
│   ├── gps_msg.h         # GPS message definition
│   └── i2c_process.h     # I2C process header
├── Makefile              # Compile with make all
├── parksys               # Executable
├── README.md             # <--- This document
└── [Src]
    ├── config.c          # Config file operations source
    ├── eth_process.c     # Ethernet process source
    ├── gps_msg.c         # GPS message functions
    ├── i2c_process.c     # I2C process source
    └── main.c            # Main source file

```
### Setup
The BBG is connected:
1. To power/PC via usb
2. To router via Ethernet, using DHCP
3. To STM32 via I2C-1, STM32 works as a slave @0x10

## Build Program
A precompiled version is available here, though it's not guerenteed it will work on your machine. A makefile is provided too, and it's meant for cross-compilation with `arm-linux-gnueabihf-gcc`.

### Compile
1. `cd` into `BBG` directory
2. run `make all`

### Upload compiled file to BBG
1. `cd` into `BBG` directory
2. run `make upload`
3. input password

### Clean
1. `cd` into `BBG` directory
2. run `make clean`

## Run Program
### Install
1. Connect to BBG using `ssh`
2. From debian's home folder, run `ls parksys` to ensure the executable was uploaded.
3. Run `sudo ./parksys`. On the first run, the program will be uploaded to system.
4. Run `sudo systemctl start parksys.service`
5. Check if the service is active with `./parksys status`

### Change Service Status
```
sudo systemctl start parksys     # Start service
sudo systemctl stop parksys      # Stop service
sudo systemctl restart parksys   # Restart service
sudo systemctl status parksys    # Read status (more verbose than "./parksys status")
```

### Read Logs
```
journalctl -u parksys -f         # Read all logs
journalctl -t parksys-i2c -f     # Read only i2c process logs
journalctl -t parksys-eth -f     # Read only eth process logs
```

### Change configuration
1. Edit the file `/etc/parksys/parksys.config` to change configurtion.
2. Restart the service with `sudo systemctl restart parksys`

Default configuration is:
```
i2c_bus=/dev/i2c-1
i2c_addr=0x10
server_ip=192.168.1.71
server_port=12321
log_path=/var/log/parksys/parksys.log
service_name=parksys.service
service_path=/etc/systemd/system/parksys.service
```

### Uninstall
From debian's home folder:
1. Run `./parksys uninstall`
2. Run `rm parksys`

If you already deleted `parksys` from the home folder, but still wish to uninstall the program from the system, simply run:
```
sudo /usr/sbin/parksys uninstall
```

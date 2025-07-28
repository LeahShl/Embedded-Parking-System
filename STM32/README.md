# Embedded Parking System - STM32 GPS Simulator
This firmware runs on an STM32 microcontroller and simulates a vehicle parking system by periodically sending messages over I2C to a master (such as a BeagleBone Green, which was used in this project). The messages represent parking events (START parking, STOP parking and IDLE) from vehicles at random intervals and locations within Israel.

## How it Works
The program uses FreeRTOS for multitasking.

1. A GPS simulator generates events every one second, randomally choosing coordinates and parking events.
2. The events are sent to a message queue.
3. An I2C task reads events from the message queue and sends them via I2C.

## Message Format
- 1 byte: Type (0 = IDLE, 1 = START, 2 = STOP)
- 4 bytes: License ID (000-00-000 - 999-99-999)
- 4 bytes: UTC seconds (uint32)
- 4 bytes: Latitude (float)
- 4 bytes: Longitude (float)

## File Structure
Key:
- regular file
- \[Directory\]

Automatically generated unchanged files skipped.
```
[STM32]
├── [Core]
│   ├── [Src]
│   │   ├── main.c          # Main file containing FreeRTOS code
├── [GPS_sim]
│   ├── [Inc]
│   │   └── gps_sim.h       # GPS Simulator header file
│   └── [Src]
│       ├── gps_sim.c       # GPS simulator task code
│       └── i2c_send.c      # I2C sender task code
├── GPS_Simulator.ioc       # CubeMX configuration
```
## Build
This is a STM32CubeIDE project, meant to be built and run using STM32CubeIDE.

1. Save this directory (`STM32`) on your filesystem
2. Open STM32CubeIDE
3. Click `File > Open Projects from Filesystem...`
4. Next to `Import source` choose `Directory..`
5. Choose `STM32CUBE` from your filesystem
6. Click `Finish`
7. Connect your STM32 board to your PC through the ST-Link USB port.
8. Build the project.

## Run
Running the program using STM32CubeIDE is pretty much straightforward, but there are some requirements for the program to run correctly.

Messages are send through an I2C slave, therefore no messages will be sent unless connected to a master. Default configurations are:

- I2C1_SCL - `PB8`
- I2C1_SDA - `PB9`
- I2C1 slave address - `0x10`

Once a master is connected and set to receive messages from slave, running the program will send messages.

## Configure
Hardware configuration is done using `GPS_Simulator.ioc`.
Firmware configuration is done using `gps_sim.h`.

## Logging
Logging is done through the ST-Link port (`PD8`&`PD9`). Output can be redirected to a file, for example, like this:
```
sudo cat /dev/ttyACM0 >> output.log
```

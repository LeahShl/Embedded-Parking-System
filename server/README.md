# Embedded Parking System - Server
A TCP-based parking event server that tracks vehicle start and stop events. It uses a shared memory SQLite database for real-time performance and persists changes to disk.

## File Structure
key:
* File
* \[Directory\]
```
[server]
├── [Inc]
│   ├── conf.hpp            # Server configuration constants
│   ├── db.hpp              # Database interface
│   └── server.hpp          # TCP server interface
├── init_db_example.sh      # Bash script to populate example city and lot data
├── Makefile                # Compile both executables
├── parksys-price-updater   # Updating parking lot prices executable
├── parksys-server-main     # Main server executable
├── README.md               # <--- This file
└── [Src]
    ├── db.cpp              # Database logic implementation
    ├── main.cpp            # Entry point for server
    ├── price_updater.cpp   # Price updater logic
    └── server.cpp          # TCP server implementation
```

## Build
Compilation is done with `Makefile`.
### Make all components
```
make all
```
### Clean
```
make clean
```

## Run Server
### Example Database
The server has no meaning without a populated database with parking lots, so I provided a bash script to populate it with example data. The script will not override an existing database, it will only create a new database if the database doesn't exist yet. The script uses the `parksys-price-updater` utility to create and populate the database.

To initialize and populate the database with example data, run:
```
./init_db_example.sh
```

### Running Server
To run the server, simply run:
```
./parksys-main-server
```

On initial run, you might see the output of a large batch of messages, followed by a slower output of new messages. This is an expected behavior and is caused by the client holding requests until a successful connection is made. The first burst of messages is the past requests that were held until the server was run.

## Use Price Updater Utility
...

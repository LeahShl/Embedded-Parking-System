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

## Server Logic

1. The server listens on a TCP port and spawns a thread per client.
2. Clients send binary requests containing type, license ID, location, and timestamp.
3. For START and STOP requests:
   - The server identifies the closest parking lot to the given GPS location.
   - It inserts a new `Log` entry for START, or updates an existing one for STOP.
   - Price is calculated based on duration and lot configuration.
4. All database writes go to shared memory and are flushed to disk.

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
The `parksys-price-updater` utility allows you to add, remove, or update cities and parking lots in the database before and during server runtime.

### Usage

```
./parksys-price-updater <command>
```

### Commands

#### Add City
```
./parksys-price-updater add city "City Name"
```
Adds a new city with the given name.

#### Remove City
```
./parksys-price-updater remove city <city_id>
```
Deletes the city by ID and also removes all associated parking lots.

#### Add Lot
```
./parksys-price-updater add lot "Lot Name" <city_id> <lat> <lon> <h|d> <price> [max_daily]
```
Adds a parking lot to the specified city.

- `h` = hourly pricing mode
- `d` = fixed daily pricing
- `price` = hourly or daily price
- `max_daily` = optional max for hourly mode

Example:
```
./parksys-price-updater add lot "Great Parking Lot" 1 32.1234 34.5678 h 24.0 120.0
```

#### Remove Lot
```
./parksys-price-updater remove lot <lot_id>
```
Removes the specified parking lot.

#### Update Lot Price
```
./parksys-price-updater update lot <lot_id> price <price> [<max_daily>]
```
Changes the hourly or daily price of a lot.

Example:
```
./parksys-price-updater update lot 2 price 20.0 80.0
```

#### Update Lot Type
```
./parksys-price-updater update lot <lot_id> type <h|d>
```
Switches the lot pricing mode between hourly (`h`) and daily (`d`).

Example:
```
./parksys-price-updater update lot 3 type d
```

### Notes

- The tool connects directly to the shared memory database at `/dev/shm/parksys.db`
- Changes are immediately flushed to the persistent file at `$HOME/.parksys.db`
- All components must be run under the same user


## Database Schema
### City

| Column     | Type    | Description           |
|------------|---------|-----------------------|
| city_id    | INTEGER | Primary key           |
| city_name  | TEXT    | Unique city name      |

### Lot

| Column          | Type    | Description                        |
|-----------------|---------|------------------------------------|
| lot_id          | INTEGER | Primary key                        |
| city_id         | INTEGER | Foreign key to City                |
| lot_name        | TEXT    | Descriptive name                   |
| latitude        | REAL    | GPS latitude                       |
| longitude       | REAL    | GPS longitude                      |
| is_hourly       | INTEGER | 1 = hourly, 0 = fixed rate         |
| price           | REAL    | Hourly or fixed price              |
| max_daily_price | REAL    | Maximum daily price (optional)     |

### Log

| Column       | Type    | Description                           |
|--------------|---------|---------------------------------------|
| log_id       | INTEGER | Primary key                           |
| lot_id       | INTEGER | Foreign key to Lot                    |
| customer_id  | INTEGER | License ID of the customer            |
| start_time   | INTEGER | Start UTC timestamp                   |
| end_time     | INTEGER | End UTC timestamp (nullable)          |
| duration_sec | INTEGER | Duration in seconds (nullable)        |
| total_price  | REAL    | Calculated parking price (nullable)   |

## Database Storage

- Shared memory location: `/dev/shm/parksys.db`
- Persistent backup location: `$HOME/.parksys.db`

If the shared memory file is not present, it is created on startup. If the backup exists, it is restored into shared memory.

### Notes

- Shared memory is volatile and cleared on reboot. It's also cleared by `parksys-server-main` on server failure.
- All components must be run under the same user to access `/dev/shm/parksys.db`.

## Logging
The server logs to std::cout each request it gets. More advance logging will be implemnted later.

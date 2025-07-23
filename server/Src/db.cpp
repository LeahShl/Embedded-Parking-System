#include "db.hpp"
#include <iostream>

namespace Parksys
{
    static bool backup(sqlite3 *src, sqlite3 *dest);

    Database::Database(const std::string &path)
        : runtime_db(nullptr), disk_db(nullptr), disk_ok(false)
    {
        // Open runtime database
        if (sqlite3_open(":memory:", &runtime_db) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to open memory DB: "
                      << sqlite3_errmsg(runtime_db) << std::endl;
            sqlite3_close(runtime_db);
            runtime_db = nullptr;
            return;
        }

        // Open database file from filesystem
        if (sqlite3_open(path.c_str(), &disk_db) == SQLITE_OK)
        {
            disk_ok = true;
            if (!backup(disk_db, runtime_db))
            {
                std::cerr << "[DB] Backup disk->mem failed\n";
            }

            const char *sql_create_tables =
            "CREATE TABLE IF NOT EXISTS City ( "
                "city_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "city_name TEXT NOT NULL UNIQUE "
            ");"
            " "
            "CREATE TABLE IF NOT EXISTS Lot ( "
                "lot_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "city_id INTEGER NOT NULL, "
                "lot_name TEXT NOT NULL, "
                "latitude REAL NOT NULL, "
                "longitude REAL NOT NULL, "
                "is_hourly INTEGER NOT NULL, "
                "price REAL NOT NULL, "
                "max_daily_price REAL NOT NULL, "
                "FOREIGN KEY(city_id) REFERENCES City(city_id) "
            ");"
            " "
            "CREATE TABLE IF NOT EXISTS Log ( "
                "log_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "lot_id INTEGER NOT NULL, "
                "customer_id INTEGER NOT NULL, "
                "start_time INTEGER NOT NULL, "
                "end_time INTEGER, "
                "duration_sec INTEGER, "
                "total_price REAL, "
                "FOREIGN KEY(lot_id) REFERENCES Lot(lot_id) "
            ");";

            char *errmsg = nullptr;
            if (sqlite3_exec(runtime_db, sql_create_tables, nullptr, nullptr, &errmsg) != SQLITE_OK)
            {
                std::string err = errmsg ? errmsg : "Unknown error";
                sqlite3_free(errmsg);
                throw std::runtime_error("Failed to create tables: " + err);
            }
        }
        else
        {
            std::cerr << "[DB] Could not open disk DB, continuing in memory only\n";
            sqlite3_close(disk_db);
            disk_db = nullptr;
        }
    }

    Database::~Database()
    {
        if (disk_ok)
        {
            flushToDisk();
            sqlite3_close(disk_db);
        }
        if (runtime_db)
        {
            sqlite3_close(runtime_db);
        }
    }

    pdbStatus Database::flushToDisk()
    {
        if (!disk_ok) return pdbStatus::PDB_ERR;

        if (!backup(runtime_db, disk_db))
        {
            std::cerr << "[DB] Backup mem->disk failed\n";
            return pdbStatus::PDB_ERR;
        }
        return pdbStatus::PDB_OK;
    }

    pdbStatus Database::startParking(uint32_t lot_id, uint32_t customer_id, uint32_t timestamp)
    {
        const char *sql =
            "INSERT INTO Log(lot_id, customer_id, start_time) "
            "VALUES(?, ?, ?);";
        
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(runtime_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare startParking: " << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }
        
        sqlite3_bind_int(stmt, 1, lot_id);
        sqlite3_bind_int(stmt, 2, customer_id);
        sqlite3_bind_int(stmt, 3, timestamp);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE)
        {
            flushToDisk();
            return pdbStatus::PDB_OK;
        }

        return pdbStatus::PDB_ERR;
    }

    pdbStatus Database::endParking(uint32_t customer_id, uint32_t end_time)
    {
        // Find last log_id with this customer_id that has no end_time
        const char *find_sql =
            "SELECT log_id, start_time, lot_id FROM Log "
            "WHERE customer_id = ? AND end_time IS NULL "
            "ORDER BY log_id DESC LIMIT 1;";
        
        sqlite3_stmt *find = nullptr;
        if (sqlite3_prepare_v2(runtime_db, find_sql, -1, &find, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare endParking find: " << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }
        
        sqlite3_bind_int(find, 1, customer_id);

        int rc = sqlite3_step(find);
        if (rc != SQLITE_ROW)
        {
            sqlite3_finalize(find);
            return pdbStatus::PDB_ERR;
        }

        int log_id = sqlite3_column_int(find, 0);
        int start_time = sqlite3_column_int(find, 1);
        int lot_id = sqlite3_column_int(find, 2);
        sqlite3_finalize(find);

        // calculate duration and price
        int duration = int(end_time) - start_time;
        double total = calculatePrice(duration, lot_id); 

        // Update the same record
        const char *upd_sql =
            "UPDATE Log SET end_time=?, duration_sec=?, total_price=? "
            "WHERE log_id = ?;";
        
        sqlite3_stmt *upd = nullptr;
        if (sqlite3_prepare_v2(runtime_db, upd_sql, -1, &upd, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare endParking write: " << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }
        
        sqlite3_bind_int(upd, 1, end_time);
        sqlite3_bind_int(upd, 2, duration);
        sqlite3_bind_double(upd, 3, total);
        sqlite3_bind_int(upd, 4, log_id);

        rc = sqlite3_step(upd);
        sqlite3_finalize(upd);

        if (rc == SQLITE_DONE)
        {
            flushToDisk();
            return pdbStatus::PDB_OK;
        }

        return pdbStatus::PDB_ERR;
    }

    double Database::calculatePrice(int duration_sec, uint32_t lot_id)
    {
        const char *sql = "SELECT is_hourly, price, max_daily_price "
                          "FROM Lot WHERE lot_id = ?;";

        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(runtime_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare calculatePrice: "
                      << sqlite3_errmsg(runtime_db) << std::endl;
            return 0.0;
        }

        sqlite3_bind_int(stmt, 1, lot_id);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            std::cerr << "[DB] No such lot_id: " << lot_id << " for price calculation\n";
            return 0.0;
        }

        int is_hourly = sqlite3_column_int(stmt, 0);
        double price = sqlite3_column_double(stmt, 1);
        double max_daily = sqlite3_column_double(stmt, 2);

        sqlite3_finalize(stmt);

        if (!is_hourly)
            return price;

        int duration_hours = (std::max(duration_sec, 0) + 3599) / 3600;  // rounded up
        double total = duration_hours * price;
        return std::min(total, max_daily);
    }

    pdbStatus Database::addCity(const std::string &name)
    {
        const char *sql = "INSERT INTO City(city_name) VALUES(?);";

        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(runtime_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare addCity: " << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE)
        {
            flushToDisk();
            return pdbStatus::PDB_OK;
        }

        return pdbStatus::PDB_ERR;
    }

    pdbStatus Database::removeCity(uint32_t city_id)
    {
        // Delete all lots first (FK)
        const char *del_lots = "DELETE FROM Lot WHERE city_id = ?;";
        sqlite3_stmt *st1 = nullptr;
        if (sqlite3_prepare_v2(runtime_db, del_lots, -1, &st1, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare remove lots in removeCity: "
                      << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }
        sqlite3_bind_int(st1, 1, city_id);
        sqlite3_step(st1);
        sqlite3_finalize(st1);

        // Proceed to remove city
        const char *del_city = "DELETE FROM City WHERE city_id = ?;";
        sqlite3_stmt *st2 = nullptr;
        if (sqlite3_prepare_v2(runtime_db, del_city, -1, &st2, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare removeCity: " << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }
        sqlite3_bind_int(st2, 1, city_id);
        int rc = sqlite3_step(st2);
        sqlite3_finalize(st2);

        if (rc == SQLITE_DONE)
        {
            flushToDisk();
            return pdbStatus::PDB_OK;
        }

        return pdbStatus::PDB_ERR;
    }

    pdbStatus Database::addLot(const std::string &name, uint32_t city_id,
                               float latitude, float longitude,
                               bool is_hourly, double price, double max_daily)
    {
        const char *sql =
        "INSERT INTO Lot(city_id, lot_name, latitude, longitude, is_hourly, price, max_daily_price) "
        "VALUES(?, ?, ?, ?, ?, ?, ?);";

        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(runtime_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare addLot: " << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }

        sqlite3_bind_int(stmt, 1, city_id);
        sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, latitude);
        sqlite3_bind_double(stmt, 4, longitude);
        sqlite3_bind_int(stmt, 5, is_hourly ? 1 : 0);
        sqlite3_bind_double(stmt, 6, price);
        sqlite3_bind_double(stmt, 7, max_daily);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE)
        {
            flushToDisk();
            return pdbStatus::PDB_OK;
        }

        return pdbStatus::PDB_ERR;
    }

    pdbStatus Database::removeLot(uint32_t lot_id)
    {
        const char *sql = "DELETE FROM Lot WHERE lot_id = ?;";

        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(runtime_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare removeLot: " << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }

        sqlite3_bind_int(stmt, 1, lot_id);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE)
        {
            flushToDisk();
            return pdbStatus::PDB_OK;
        }

        return pdbStatus::PDB_ERR;
    }

    pdbStatus Database::updateLotPrice(uint32_t lot_id, double price, double max_daily)
    {
        const char* sql_hourly = "UPDATE Lot SET price = ? WHERE lot_id = ?;";
        const char* sql_daily  = "UPDATE Lot SET price = ?, max_daily_price = ? WHERE lot_id = ?;";

        sqlite3_stmt *stmt = nullptr;

        if (max_daily > 0)
        {
            if (sqlite3_prepare_v2(runtime_db, sql_daily, -1, &stmt, nullptr) != SQLITE_OK)
            {
                std::cerr << "[DB] Failed to prepare updateLotPrice: " << sqlite3_errmsg(runtime_db) << std::endl;
                return pdbStatus::PDB_ERR;
            }
            
            sqlite3_bind_double(stmt, 1, price);
            sqlite3_bind_double(stmt, 2, max_daily);
            sqlite3_bind_int(stmt, 3, lot_id);
        }
        else
        {
            if (sqlite3_prepare_v2(runtime_db, sql_hourly, -1, &stmt, nullptr) != SQLITE_OK)
            {
                std::cerr << "[DB] Failed to prepare updateLotPrice: " << sqlite3_errmsg(runtime_db) << std::endl;
                return pdbStatus::PDB_ERR;
            }

            sqlite3_bind_double(stmt, 1, price);
            sqlite3_bind_int(stmt, 2, lot_id);
        }

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE)
        {
            flushToDisk();
            return pdbStatus::PDB_OK;
        }

        return pdbStatus::PDB_ERR;
    }

    pdbStatus Database::setLotType(uint32_t lot_id, bool is_hourly)
    {
        const char *sql = "UPDATE Lot SET is_hourly = ? WHERE lot_id = ?;";

        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(runtime_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[DB] Failed to prepare setLotType: " << sqlite3_errmsg(runtime_db) << std::endl;
            return pdbStatus::PDB_ERR;
        }

        sqlite3_bind_int(stmt, 1, is_hourly ? 1 : 0);
        sqlite3_bind_int(stmt, 2, lot_id);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE)
        {
            flushToDisk();
            return pdbStatus::PDB_OK;
        }

        return pdbStatus::PDB_ERR;
    }

    static bool backup(sqlite3 *src, sqlite3 *dest)
    {
        sqlite3_backup *b = sqlite3_backup_init(dest, "main", src, "main");
        if (!b) return false;

        if (sqlite3_backup_step(b, -1) != SQLITE_DONE)
        {
            sqlite3_backup_finish(b);
            return false;
        }

        sqlite3_backup_finish(b);
        return true;
    }
}

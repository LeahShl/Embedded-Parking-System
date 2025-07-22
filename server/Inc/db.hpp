#pragma once
#include <sqlite3.h>
#include <string>

namespace Parksys
{
    enum class pdbStatus
    {
        PDB_OK = 0,         // Success
        PDB_ERR             // General error occured
    };

    class Database
    {
    public:
        Database(const std::string &path);
        ~Database();

        // Logging

        pdbStatus startParking(uint32_t lot_id, uint32_t customer_id, uint32_t timestamp);
        pdbStatus endParking(uint32_t customer_id, uint32_t timestamp);

        // Price and data management 

        pdbStatus addCity(const std::string &name);
        pdbStatus removeCity(uint32_t city_id);
        pdbStatus addLot(const std::string &name, uint32_t city_id,
                         float latitude, float longitude,
                         bool is_hourly, double price, double max_daily);
        pdbStatus removeLot(uint32_t lot_id);
        pdbStatus updateLotPrice(uint32_t lot_id, double price, double max_daily = 0.0);
        pdbStatus setLotType(uint32_t lot_id, bool is_hourly);

    private:
        sqlite3 *runtime_db;
        sqlite3 *disk_db;
        bool disk_ok;

        /**
         * @brief Backup runtime database to disk
         * 
         * @return pdbStatus 
         */
        pdbStatus flushToDisk();

        double calculatePrice();
    };
}
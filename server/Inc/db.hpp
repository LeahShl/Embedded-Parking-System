#pragma once
#include <sqlite3.h>
#include <string>

namespace Parksys
{
    /**
     * @brief Status codes returned by database operations
     * 
     */
    enum class pdbStatus
    {
        PDB_OK = 0,         // Success
        PDB_ERR             // General error occured
    };

    /**
     * @brief A wrapper class for handling parking system database operations
     * 
     * This class manages a runtime SQLite database in runtime memory and 
     * periodically flushes it to disk. It provides APIs to log parking 
     * sessions and manage city and parking lot records.
     * 
     */
    class Database
    {
    public:
        /**
         * @brief Construct a new Database object
         * 
         * @param path Path to the .db file on disk. If the file does not exist, it will be created.
         */
        Database(const std::string &path);

        /**
         * @brief Destroy the Database object.
         * 
         * Flushes the in-memory runtime database to disk (if valid), and releases resources.
         */
        ~Database();

        // ------------------ Logging -------------------

        /**
         * @brief Start a parking session for a given customer at a specific lot.
         * 
         * @param lot_id ID of the lot where parking starts
         * @param customer_id Customer's unique ID. for example, their license
         * @param timestamp UTC timestamp when the parking starts (in seconds)
         * @return pdbStatus Status of the operation
         */
        pdbStatus startParking(uint32_t lot_id, uint32_t customer_id, uint32_t timestamp);

        /**
         * @brief End a parking session for a given customer.
         * 
         * Calculates total price based on lot type and duration, and updates the session record.
         * 
         * @param customer_id Unique ID of the customer
         * @param timestamp UTC timestamp when the parking ends (in seconds)
         * @return pdbStatus Status of the operation
         */
        pdbStatus endParking(uint32_t customer_id, uint32_t timestamp);

        // ---------- Price and data management ----------

        /**
         * @brief Add a new city to the database.
         * 
         * @param name Name of the city
         * @return pdbStatus Status of the operation
         */
        pdbStatus addCity(const std::string &name);

        /**
         * @brief Remove a city and all its lots from the database.
         * 
         * @param city_id ID of the city to remove
         * @return pdbStatus Status of the operation
         */
        pdbStatus removeCity(uint32_t city_id);

        /**
         * @brief Add a new parking lot to a given city.
         * 
         * @param name Name of the lot
         * @param city_id ID of the city to which the lot belongs
         * @param latitude GPS latitude
         * @param longitude GPS longitude
         * @param is_hourly Whether the lot charges hourly (true) or fixed price (false)
         * @param price Price per hour or fixed rate (depending on is_hourly)
         * @param max_daily Maximum daily price (only relevant for hourly lots)
         * @return pdbStatus Status of the operation
         */
        pdbStatus addLot(const std::string &name, uint32_t city_id,
                         float latitude, float longitude,
                         bool is_hourly, double price, double max_daily);

        /**
         * @brief Remove a parking lot from the database.
         * 
         * @param lot_id ID of the lot to remove
         * @return pdbStatus Status of the operation
         */
        pdbStatus removeLot(uint32_t lot_id);

        /**
         * @brief Update the price and max daily rate of a lot.
         * 
         * @param lot_id ID of the lot to update
         * @param price New price (per hour or fixed)
         * @param max_daily New max daily price (optional, relevant only for hourly lots)
         * @return pdbStatus Status of the operation
         */
        pdbStatus updateLotPrice(uint32_t lot_id, double price, double max_daily = 0.0);

        /**
         * @brief Set whether a parking lot is hourly or fixed-rate.
         * 
         * @param lot_id ID of the lot to modify
         * @param is_hourly True for hourly pricing, false for fixed
         * @return pdbStatus Status of the operation
         */
        pdbStatus setLotType(uint32_t lot_id, bool is_hourly);

    private:
        sqlite3 *runtime_db;
        sqlite3 *disk_db;
        bool disk_ok;

        /**
         * @brief Backup the in-memory runtime database to disk.
         * 
         * @return pdbStatus Status of the operation
         */
        pdbStatus flushToDisk();

        /**
         * @brief Calculate the total price for a parking session.
         * 
         * @param duration_sec Duration of the parking session in seconds
         * @param lot_id ID of the lot where the session took place
         * @return double Calculated price
         */
        double calculatePrice(int duration_sec, uint32_t lot_id);
    };
}
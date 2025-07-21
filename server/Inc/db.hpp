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
        Database(std::string path);
        Database(Database *pdb);
        ~Database();

        // Logging

        pdbStatus startParking();
        pdbStatus endParking();

        // Price and data management 

        double calculatePrice();
        pdbStatus addCity();
        pdbStatus removeCity();
        pdbStatus addLot();
        pdbStatus removeLot();
        pdbStatus changePrice();

    private:
        sqlite3 *db;
    };
}
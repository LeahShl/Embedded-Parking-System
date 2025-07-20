#pragma once
#include <sqlite3.h>
#include <string>

enum class pdbStatus
{
    PDB_OK = 0,         // Success
    PDB_ERR             // General error occured
};

class ParkingDB
{
public:
    ParkingDB(std::string path);
    ParkingDB(ParkingDB *pdb);
    ~ParkingDB();

    pdbStatus startParking();
    pdbStatus endParking();
    double calculatePrice();
    pdbStatus addCity();
    pdbStatus removeCity();
    pdbStatus addLot();
    pdbStatus removeLot();
    pdbStatus changePrice();

private:
    sqlite3 *db;
};
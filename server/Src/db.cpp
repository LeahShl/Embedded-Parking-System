#include "db.hpp"

namespace Parksys
{
    Database::Database(std::string path)
    {
        db = nullptr;
    }

    Database::Database(Database *pdb)
    {
        db = pdb->db;
    }

    Database::~Database()
    {
    }

    pdbStatus Database::startParking()
    {
        return pdbStatus::PDB_OK;
    }

    pdbStatus Database::endParking()
    {
        return pdbStatus::PDB_OK;
    }

    double Database::calculatePrice()
    {
        return 0.0;
    }

    pdbStatus Database::addCity()
    {
        return pdbStatus::PDB_OK;
    }

    pdbStatus Database::removeCity()
    {
        return pdbStatus::PDB_OK;
    }

    pdbStatus Database::addLot()
    {
        return pdbStatus::PDB_OK;
    }

    pdbStatus Database::removeLot()
    {
        return pdbStatus::PDB_OK;
    }

    pdbStatus Database::changePrice()
    {
        return pdbStatus::PDB_OK;
    }
}

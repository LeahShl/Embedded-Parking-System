#include "db.hpp"
#include "conf.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace Parksys;

static void print_usage()
{
    std::cout <<
    "Usage:\n"
    "  parksys-price-updater add city \"City Name\"\n"
    "  parksys-price-updater remove city <city_id>\n"
    "  parksys-price-updater add lot \"Lot Name\" <city_id> <lat> <lon> <h|d> <price> [max_daily]\n"
    "  parksys-price-updater remove lot <lot_id>\n"
    "  parksys-price-updater update lot <lot_id> price <price> [<max_daily>]\n"
    "  parksys-price-updater update lot <lot_id> type <h|d>\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        print_usage();
        return 1;
    }

    Database db(std::string(std::getenv("HOME")) + "/" + DB_PATH);

    std::string command = argv[1];
    std::string object = argv[2];

    if (command == "add" && object == "city" && argc == 4)
    {
        std::string name = argv[3];
        if (db.addCity(name) == pdbStatus::PDB_OK)
        {
            std::cout << "City added: " << name << std::endl;
        }
        else
        {
            std::cerr << "Failed to add city. Name may exist." << std::endl;
            return 1;
        }

    }
    else if (command == "remove" && object == "city" && argc == 4)
    {
        uint32_t cid = std::stoul(argv[3]);
        std::cout << "Remove city " << cid << "? [y/N] ";
        char c; std::cin >> c;
        if (c=='y'||c=='Y')
        {
            db.removeCity(cid);
            std::cout << "City removed.\n";
        }

    }
    else if (command == "add" && object == "lot" && argc >= 9)
    {
        std::string name = argv[3];
        uint32_t city_id = std::stoul(argv[4]);
        float lat        = std::stof(argv[5]);
        float lon        = std::stof(argv[6]);
        bool is_hourly   = (argv[7][0]=='h');
        double price     = std::stod(argv[8]);
        double max_daily = (argc==10 ? std::stod(argv[9]) : 0.0);

        if (db.addLot(name, city_id, lat, lon, is_hourly, price, max_daily) == pdbStatus::PDB_OK)
        {
            std::cout << "Lot added: " << name << std::endl;
        }
        else
        {
            std::cerr << "Failed to add lot\n";
            return 1;
        }

    }
    else if (command == "remove" && object == "lot" && argc == 4)
    {
        uint32_t lid = std::stoul(argv[3]);
        std::cout << "Removing a lot won't remove previous logs.\n"
                  << "Remove lot " << lid << "? [y/N] ";
        char c; std::cin >> c;

        if (c=='y'||c=='Y')
        {
            db.removeLot(lid);
            std::cout << "Lot removed.\n";
        }

    }
    else if (command == "update" && object == "lot" && argc >= 6)
    {
        uint32_t lid = std::stoul(argv[3]);
        std::string sub = argv[4];

        if (sub == "price")
        {
            double price     = std::stod(argv[5]);
            double max_daily = (argc==7 ? std::stod(argv[6]) : 0.0);

            if (max_daily>0 && max_daily<price)
            {
                std::cerr << "Warning: max_daily < price. aborting\n";
                return 1;
            }

            db.updateLotPrice(lid, price, max_daily);

        }
        else if (sub == "type" && argc==6)
        {
            bool is_hourly = (argv[5][0]=='h');
            db.setLotType(lid, is_hourly);

        }
        else
        {
            print_usage();
            return 1;
        }

    }
    else
    {
        print_usage();
        return 1;
    }

    return 0;
}

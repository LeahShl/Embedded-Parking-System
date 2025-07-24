#include "conf.hpp"
#include "server.hpp"
#include "db.hpp"
#include <string>

int main()
{
    Parksys::Database pdb (std::string(std::getenv("HOME")) + "/" + DB_PATH);
    Parksys::Server server(SERVER_IP, SERVER_PORT, &pdb);
    server.run(); 
    return 0;
}
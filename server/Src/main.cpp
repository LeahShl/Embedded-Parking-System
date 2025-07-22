#include "conf.hpp"
#include "server.hpp"
#include "db.hpp"
#include <string>

int main()
{
    Parksys::Database pdb (std::string(DB_PATH));
    Parksys::Server server(SERVER_IP, SERVER_PORT, &pdb);
    server.run();
    return 0;
}
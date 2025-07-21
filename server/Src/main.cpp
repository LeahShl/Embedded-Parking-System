#include "conf.hpp"
#include "server.hpp"
#include "db.hpp"

int main()
{
    Parksys::Database pdb (DB_PATH);
    Parksys::Server server(SERVER_IP, SERVER_PORT, &pdb);
    server.run();
    return 0;
}
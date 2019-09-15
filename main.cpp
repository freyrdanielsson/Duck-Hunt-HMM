#include "Player.hpp"
#include "Client.hpp"
#include "GameServer.hpp"

#include <iostream>
#include <fstream>

bool gVerbose = false;

int main(int argc,char **argv)
{
    // Parse parameters
    bool lCreateServer = false;
    std::string lLoadFilename = "SouthEmissions.in";

    for (int i = 1; i < argc; ++i)
    {
        std::string param(argv[i]);
        if (param == "server" || param == "s")
        {
            lCreateServer = true;
        }
        else if (param == "verbose" || param == "v")
        {
            gVerbose = true;
        }
        else if (param == "load" || param == "l")
        {
            ++i;
            if (i < argc)
                lLoadFilename = argv[i];
            else
            {
                std::cerr << "Observations file must be given as an argument" << std::endl;
                exit(-1);
            }
        }
        else
        {
            std::cerr << "Unknown parameter: '" << argv[i] << "'" << std::endl;
            exit(-1);
        }
    }

    /**
     * Start the program either as a server or a client
     */
    if (lCreateServer)
    {
        // Create a server
        ducks::GameServer lGameServer(std::cin, std::cout);

        if (!lLoadFilename.empty())
        {
            if (gVerbose)
                std::cerr << "Loading '" << lLoadFilename << "'" << std::endl;
            std::ifstream lFile(lLoadFilename);
            lGameServer.load(lFile);
        }

        // Run the server
        lGameServer.run();
    }
    else
    {
        // Create the player
        ducks::Player lPlayer;

        // Create a client with the player
        ducks::Client lClient(lPlayer, std::cin, std::cout);

        // Run the client
        lClient.run();
    }
    return 0;
}

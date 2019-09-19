#ifndef _DUCKS_CLIENT_HPP_
#define _DUCKS_CLIENT_HPP_

#include "Player.hpp"
#include <vector>
#include <string>
#include <iostream>

namespace ducks
{

/**
 * Encapsulates client functionality (except agent intelligence)
 */
class Client
{
public:
    /**
     * Create a client with a player
     *
     * The client is connected to the server through streams
     */
    Client(Player &pPlayer, std::istream &pInputStream, std::ostream &pOutputStream);
    Client(const Client &) = delete;
    Client & operator = (const Client &) = delete;

    /**
     * Run the client
     */
    void run();

private:
    bool processMessage();

    Player &mPlayer;
    GameState mState;
    std::istream &mInputStream;
    std::ostream &mOutputStream;
};

} /*namespace ducks*/

#endif

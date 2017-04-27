// Filename:    AdminShell.cpp
// Author:      Joseph DeVictoria
// Date:        Mar_6_2016
// Purpose:     Provide an interface for the administrator to interact with
//              the server.  This class will handle all server admin commands.

#include "AdminShell.h"
#include "Utils.h"
#include "Packets.h"
#include <iostream>
#include <sstream>
#include <string>

AdminShell::AdminShell(Server * server, SQLConnector * sql, GameState * gameState) {
    this->server = server;
    this->sql = sql;
    this->gameState = gameState;
    gethostname(serverHostname, HOST_NAME_MAX);
}

AdminShell::~AdminShell() {
    return;
}

void AdminShell::operator()() {
    Run();
}

void AdminShell::Run() {
    std::string adminCommand;
    std::cout << "Starting Server Administrator Shell.\n";
    PrintLogo();
    while(true) {
	    do
	    {
            std::cout << "OldentideAdmin@" << serverHostname << ": ";
            getline(std::cin,adminCommand);
	    }while(adminCommand.empty());
        std::vector<std::string> adminTokens = utils::Tokenfy(adminCommand, ' ');
        if (adminTokens[0] == "/shutdown") {
            std::cout << "Oldentide Dedicated Server is shutting down..." << std::endl;
            exit(EXIT_SUCCESS);
            return;
        }
        else if (adminTokens[0] == "/list") {
            if (adminTokens.size() == 2) {
                std::cout << adminTokens[1];
                if (adminTokens[1] == "accounts") {
                    sql->ListAccounts();
                }
                if (adminTokens[1] == "players") {
                    std::set<Player> players = gameState->getPlayers();
                    for (std::set<Player>::iterator it = players.begin(); it != players.end(); ++it) {
                        Player temp = *it;
                        std::cout << temp.GetName() << " " << temp.GetLastname() << std::endl;
                    }
                }
                if (adminTokens[1] == "npcs") {
                    std::cout << "NPCSSSSS" << std::endl;
                }
                if (adminTokens[1] == "packetqueue" || adminTokens[1] == "pq") {
                    int size = server->GetPacketQueueSize();
                    std::cout << "Instantaneous packetQueue size: " << size << std::endl;
                }
                if (adminTokens[1] == "packets") {
                    std::cout << "\nMAX Packet Size (w/msgpack):" << PACKET_MAX_SIZE << std::endl;;
                    std::cout << "GENERIC (" << (int) GENERIC << "): " << sizeof(PacketGeneric) << std::endl;
                    std::cout << "ACK (" << (int) ACK << "): " << sizeof(PacketAck) << std::endl;
                    std::cout << "CONNECT (" << (int) CONNECT << "): " << sizeof(PacketConnect) << std::endl;
                    std::cout << "DISCONNECT (" << (int) DISCONNECT << "): " << sizeof(PacketDisconnect) << std::endl;
                    std::cout << "LISTCHARACTERS (" << (int) LISTCHARACTERS << "): " << sizeof(PacketListcharacters) << std::endl;
                    std::cout << "SELECTCHARACTER (" << (int) SELECTCHARACTER << "): " << sizeof(PacketSelectcharacter) << std::endl;
                    std::cout << "DELETECHARACTER (" << (int) DELETECHARACTER << "): " << sizeof(PacketDeletecharacter) << std::endl;
                    std::cout << "CREATECHARACTER (" << (int) CREATECHARACTER << "): " << sizeof(PacketCreatecharacter) << std::endl;
                    std::cout << "INITIALIZEGAME (" << (int) INITIALIZEGAME << "): " << sizeof(PacketInitializegame) << std::endl;
                    std::cout << "UPDATEPC (" << (int) UPDATEPC << "): " << sizeof(PacketUpdatepc) << std::endl;
                    std::cout << "UPDATENPC (" << (int) UPDATENPC << "): " << sizeof(PacketUpdatenpc) << std::endl;
                    std::cout << "SENDPLAYERCOMMAND (" << (int) SENDPLAYERCOMMAND << "): " << sizeof(PacketSendplayercommand) << std::endl;
                    std::cout << "SENDPLAYERACTION (" << (int) SENDPLAYERACTION << "): " << sizeof(PacketSendplayeraction) << std::endl;
                    std::cout << "SENDSERVERACTION (" << (int) SENDSERVERACTION << "): " << sizeof(PacketSendserveraction) << std::endl;
                }
            }
            else {
                PrintUsage();
            }
        }
        else if (adminTokens[0] == "/db") {
            std::string cmd = adminCommand.erase(0,4);
            sql->Execute(cmd);
        }
        else {
            PrintUsage();
        }
    }
}

void AdminShell::PrintUsage() {
    std::cout << "Dedicated Server Admin Usage:" << std::endl;
    std::cout << "/shutdown    = Shuts down the server." << std::endl;
    std::cout << "/list <var>  = Lists all entities of given <var> on server, where <var> is [players, npcs, accounts, packets, packetqueue(pq)]." << std::endl;
    std::cout << "/db <query>  = Runs a given sql query on the sqlite3 database." << std::endl;
}

void AdminShell::PrintLogo() {
    std::cout << "  ____           ___   _____         _____  _____  ___   _____" << std::endl;
    std::cout << " /    \\  |      |   \\  |      |   |    |      |    |  \\  |" << std::endl;
    std::cout << "/      \\ |      |    \\ |      |\\  |    |      |    |   \\ |" << std::endl;
    std::cout << "|      | |      |    | |___   | \\ |    |      |    |   | |___" << std::endl;
    std::cout << "\\      / |      |   /  |      |  \\|    |      |    |   / |" << std::endl;
    std::cout << " \\____/  |_____ |__/   |____  |   \\    |    __|__  |__/  |____" << std::endl;
    std::cout << " " << std::endl;
    std::cout << "                              |" << std::endl;
    std::cout << "                             / \\" << std::endl;
    std::cout << "                            /\\_/\\" << std::endl;
    std::cout << "                           / | | \\" << std::endl;
    std::cout << "                           \\ |_| /" << std::endl;
    std::cout << "                            \\/ \\/" << std::endl;
    std::cout << "                             \\ /" << std::endl;
    std::cout << "                              |" << std::endl;
    std::cout << " " << std::endl;
}

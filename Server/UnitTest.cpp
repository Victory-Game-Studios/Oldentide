// Filename:    UnitTest.cpp
// Author:      Joseph DeVictoria
// Date:        September_24_2016
// Purpose:     A non-exhaustive unit testing platform to quickly enable functions.

// Load the testing framework (do it ABOVE the catch include!)
#define CATCH_CONFIG_MAIN   // This tells Catch to provide a main() - only do this in one cpp file

#include <catch.hpp>
#include "AdminShell.h"
#include "GameState.h"
#include "Npc.h"
#include "Packets.h"
#include "Player.h"
#include "Server.h"
#include "SQLConnector.h"
#include "Utils.h"


// TODO: Should we init the db to a blank slate?
// TODO: Make a function to programmatically init the db, so we can use here
TEST_CASE( "Insert Player", "[insert]" ) {
    SQLConnector* sql = new SQLConnector();
    sockaddr_in dummyClient;
    stats_t dummyStats;
    equipment_t dummyEquipment;
    location_t dummyLocation;

    Player p(
        dummyClient,
        "example",
        17,
        1,
        "Poop",
        "Stain",
        "Newcomers_Guild",
        "Human",
        "Male",
        "Scarred",
        "Pale",
        "zone1",
        "Shaman",
        dummyEquipment,
        dummyStats,
        dummyLocation
    );
     // 0, 0, 0, 0, 0, 0, 0, "heady", "chest", "army", "handy", "leggy", "footy", "elven cloak", "necklace", "ring1", "ring2", "lrighthand", "lefthand",

    REQUIRE( sql->InsertPlayer(p) == true );
    delete sql;
}





TEST_CASE( "Insert Account", "[insert]" ) {
    SQLConnector* sql = new SQLConnector();
    REQUIRE( sql->InsertAccount("my_account", "my_email@my.example.com", "deadBEEF019", "deAD1337") == true );
    delete sql;
}


TEST_CASE( "Test instantiating Server on port 7331", "[server]" ) {
    Server * server = new Server(7331);
    delete server;
}


// TODO: What to break out into test cases, and what to break out into sections?


TEST_CASE( "SanitizeAccountName", "[sql]" ) {
    REQUIRE( utils::SanitizeAccountName("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA") == false );
    REQUIRE( utils::SanitizeAccountName("!@#$%!@%@!^#$%@#$") == false );
    REQUIRE( utils::SanitizeAccountName("; drop all tables") == false );
    REQUIRE( utils::SanitizeAccountName("A") == false );
    REQUIRE( utils::SanitizeAccountName("") == false );
    REQUIRE( utils::SanitizeAccountName("        ") == false );
    REQUIRE( utils::SanitizeAccountName("my_ACCOUNT_1234   ") == false );

    REQUIRE( utils::SanitizeAccountName("my_ACCOUNT_1234") == true );


}

TEST_CASE( "CheckAccountNameLength", "[sql]" ) {
    REQUIRE( utils::CheckAccountNameLength("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA") == false );
    REQUIRE( utils::CheckAccountNameLength("A") == false );
    REQUIRE( utils::CheckAccountNameLength("") == false );

    REQUIRE( utils::CheckAccountNameLength("my_ACCOUNT_1234") == true );
}


TEST_CASE( "SanitizeAlphanumeric", "[sql]" ) {
    REQUIRE( utils::SanitizeAlphanumeric("*&!@^*&#@@#$") == false );
    REQUIRE( utils::SanitizeAlphanumeric(";;;;;") == false );
    REQUIRE( utils::SanitizeAlphanumeric("||||") == false );
    REQUIRE( utils::SanitizeAlphanumeric("----") == false );
    REQUIRE( utils::SanitizeAlphanumeric("        ") == false );
    REQUIRE( utils::SanitizeAlphanumeric("my_ACCOUNT_1234   ") == false );
    REQUIRE( utils::SanitizeAlphanumeric("####") == false );

    REQUIRE( utils::SanitizeAlphanumeric("____") == true );
    REQUIRE( utils::SanitizeAlphanumeric("my_ACCOUNT_1234") == true );
}



// TODO: Create tests for all the Util functions, if possible
// TODO: Create Util functions for setting up the IP stuff, so it's easier to call here
// TODO: Create tests

// TODO: Try out some BDD tests - Catch supports them!



// TEST_CASE( "Test instantiating Server on port 7331", "[server]" ) {
//     Server * server = new Server(7331);
//     delete server;
// }




// References:
// How to use Catch:
// https://github.com/philsquared/Catch/blob/master/docs/tutorial.md
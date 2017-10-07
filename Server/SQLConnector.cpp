// Filename:    SQLConnector.cpp
// Author:      Joseph DeVictoria
// Date:        Feb_29_2016
// Purpose:     SQL Connector class to handle SQLite3 queries.

#include "SQLConnector.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "Utils.h"
#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>

// TODO: Make all strings references to reduce copying

// For how to use sqlitecpp, see https://github.com/SRombauts/SQLiteCpp/blob/master/examples/example1/main.cpp

// Note: In c++, you can't just instantiate a class inside the constructor - it needs to be in the initialization list
// This tries to open the database file when instantiated, and set it to write
SQLConnector::SQLConnector() : db("db/Oldentide.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE){
    std::cout << "Oldentide database \"" << db.getFilename() << "\" opened successfully." << std::endl;
}

// TODO: There is no need to free the db, right? Should be automatic?
SQLConnector::~SQLConnector() {}


bool SQLConnector::ExecuteSqlFile(std::string filename) {
    // Extract the contents of the sql file into a stringstream
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    // Execute the sql file
    try {
        db.exec(buffer.str());
        return true;
    }
    catch (std::exception& e) {
        std::cout << "Exception: could not execute sql file:" << std::endl;
        std::cout << e.what() << std::endl;
        return false;
    }
}



// Executes a static SQL statement, and prints out the result
// Returns true if successful, false otherwise
bool SQLConnector::Execute(std::string cmd, bool quiet) {
    try {
        SQLite::Statement query(db, cmd);
        if(!quiet){
            std::cout << "Attempting to execute the following query:" << std::endl;
            std::cout << query.getQuery() << std::endl;
        }
        int i = 0;
        bool printed_columns = false;
        while (query.executeStep()) {
            if(quiet){
                i++;
                continue;
            }
            int col_count = query.getColumnCount();
            if(!printed_columns){
                printed_columns = true;
                std::cout << "Columns: ";
                for (int k = 0; k < col_count; ++k) {
                    // To enable getColumnOriginName(), make sure SQLITE_ENABLE_COLUMN_METADATA is defined during compilation
                    std::cout << std::setw(10) << query.getColumnOriginName(k) << " | ";
                }
                std::cout << std::endl;
            }
            std::cout << "Row " << std::setw(3) << i << ": ";
            for (int j = 0; j < col_count; ++j){
                std::cout << std::setw(10) << query.getColumn(j) << " | ";
            }
            std::cout << std::endl;
            i++;
        }
        return true;
    }
    catch (std::exception& e) {
        std::cout << "Exception: could not execute sql statement:" << std::endl;
        std::cout << e.what() << std::endl;
        return false;
    }
}

// -------------------------------------- Parent Functions ----------------------------------------

// Insert a new account record.
int SQLConnector::InsertAccount(std::string accountName, std::string email, std::string key, std::string salt) {
    // Sanitize key, salt, and account name
    if (!utils::SanitizeAccountName(accountName)) {
        std::cout << "Account Name is invalid! Cannot insert account record" << std::endl;
        return false;
    }
    // TODO: Sanitize email!
    // if (!utils::SanitizeEmail(email)) {
    //     std::cout << "Email is invalid! Cannot insert account record" << std::endl;
    //     return false;
    // }
    if (!utils::SanitizeHexString(key)) {
        std::cout << "Key is invalid! Cannot insert account record" << std::endl;
        return false;
    }
    if (!utils::SanitizeHexString(salt)) {
        std::cout << "Salt is invalid! Cannot insert account record" << std::endl;
        return false;
    }
    std::string cmd("insert into accounts (accountname, valid, email, playing, key, salt) values (?,?,?,?,?,?)");
    SQLite::Statement query(db, cmd);
    query.bind(1, accountName);
    query.bind(2, true);
    query.bind(3, email);
    query.bind(4, true);
    query.bind(5, key);
    query.bind(6, salt);
    while (query.executeStep()) {
        std::cout << "Executing step for insert account..." << std::endl;
    }

    return true;
}

// Inserts a new player into the database.
int SQLConnector::InsertPlayer(Player p, int account_id) {
    std::cout << "Creating player " << p.GetFirstname() << " " << p.GetLastname() << std::endl;

    // First, insert the base character that the player is based off of
    Character c(
        p.GetFirstname(),
        p.GetLastname(),
        p.GetGuild(),
        p.GetRace(),
        p.GetGender(),
        p.GetFace(),
        p.GetSkin(),
        p.GetZone(),
        p.GetProfession(),
        p.GetEquipment(),
        p.GetStats(),
        p.GetSkills(),
        p.GetLocation()
    );

    int character_id = InsertCharacter(c);

    if(character_id == 0){
        std::cout << "Could not insert player - base character insertion failed" << std::endl;
        return 0;
    }

    // TODO: Deal with these fields
    // sockaddr_in client,
    // std::string account,
    // int id,
    // int session,

    // TODO: Update active player list


    // Now insert the player using the newly-created character
    std::string query_string(R"(
        INSERT INTO players (
            character_id,
            account_id
        ) VALUES (
            :character_id,
            :account_id
        )
    )");


    try {
        SQLite::Statement query(db, query_string);
        query.bind(":character_id", character_id);
        query.bind(":account_id", account_id);
        int rows_modified = query.exec();

        if(rows_modified < 1){
            std::cout << "Could not insert character record! " << rows_modified << "Rows were modified" << std::endl;
            return 0;
        }
        else {
            // Get the id of the newly inserted record
            return db.getLastInsertRowid();
        }
    }
    catch (std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
        std::cout << query_string << std::endl;
        return 0;
    }
}

int SQLConnector::InsertCharacter(Character c) {
    std::stringstream query;

    location_t location = c.GetLocation();
    equipment_t equip = c.GetEquipment();
    stats_t stats = c.GetStats();
    skills_t skills = c.GetSkills();

    std::cout << "Creating character " << c.GetFirstname() << " " << c.GetLastname() << std::endl;

    // Multiline string literal!
    std::string query_string(R"(
    INSERT INTO characters (
        firstname,
        lastname,
        guild,
        race,
        gender,
        face,
        skin,
        profession,
        level,
        hp,
        maxhp,
        bp,
        maxbp,
        mp,
        maxmp,
        ep,
        maxep,
        strength,
        constitution,
        intelligence,
        dexterity,
        axe,
        dagger,
        unarmed,
        hammer,
        polearm,
        spear,
        staff,
        sword,
        archery,
        crossbow,
        sling,
        thrown,
        armor,
        dualweapon,
        shield,
        bardic,
        conjuring,
        druidic,
        illusion,
        necromancy,
        sorcery,
        shamanic,
        summoning,
        spellcraft,
        focus,
        armorsmithing,
        tailoring,
        fletching,
        weaponsmithing,
        alchemy,
        lapidary,
        calligraphy,
        enchanting,
        herbalism,
        hunting,
        mining,
        bargaining,
        camping,
        firstaid,
        lore,
        picklocks,
        scouting,
        search,
        stealth,
        traps,
        aeolandis,
        hieroform,
        highgundis,
        oldpraxic,
        praxic,
        runic,
        head,
        chest,
        arms,
        hands,
        legs,
        feet,
        cloak,
        necklace,
        ringone,
        ringtwo,
        righthand,
        lefthand,
        zone,
        x,
        y,
        z,
        pitch,
        yaw
    ) VALUES (
        :firstname,
        :lastname,
        :guild,
        :race,
        :gender,
        :face,
        :skin,
        :profession,
        :level,
        :hp,
        :maxhp,
        :bp,
        :maxbp,
        :mp,
        :maxmp,
        :ep,
        :maxep,
        :strength,
        :constitution,
        :intelligence,
        :dexterity,
        :axe,
        :dagger,
        :unarmed,
        :hammer,
        :polearm,
        :spear,
        :staff,
        :sword,
        :archery,
        :crossbow,
        :sling,
        :thrown,
        :armor,
        :dualweapon,
        :shield,
        :bardic,
        :conjuring,
        :druidic,
        :illusion,
        :necromancy,
        :sorcery,
        :shamanic,
        :summoning,
        :spellcraft,
        :focus,
        :armorsmithing,
        :tailoring,
        :fletching,
        :weaponsmithing,
        :alchemy,
        :lapidary,
        :calligraphy,
        :enchanting,
        :herbalism,
        :hunting,
        :mining,
        :bargaining,
        :camping,
        :firstaid,
        :lore,
        :picklocks,
        :scouting,
        :search,
        :stealth,
        :traps,
        :aeolandis,
        :hieroform,
        :highgundis,
        :oldpraxic,
        :praxic,
        :runic,
        :head,
        :chest,
        :arms,
        :hands,
        :legs,
        :feet,
        :cloak,
        :necklace,
        :ringone,
        :ringtwo,
        :righthand,
        :lefthand,
        :zone,
        :x,
        :y,
        :z,
        :pitch,
        :yaw
    )
    )"); // End query

    try {
        SQLite::Statement query(db, query_string);
        query.bind(":firstname", c.GetFirstname());
        query.bind(":lastname", c.GetLastname());
        query.bind(":guild", c.GetGuild());
        query.bind(":race", c.GetRace());
        query.bind(":gender", c.GetGender());
        query.bind(":face", c.GetFace());
        query.bind(":skin", c.GetSkin());
        query.bind(":profession", c.GetProfession());
        query.bind(":level", stats.level);
        query.bind(":hp", stats.hp);
        query.bind(":maxhp", stats.maxhp);
        query.bind(":bp", stats.bp);
        query.bind(":maxbp", stats.maxbp);
        query.bind(":mp", stats.mp);
        query.bind(":maxmp", stats.maxmp);
        query.bind(":ep", stats.ep);
        query.bind(":maxep", stats.maxep);
        query.bind(":strength", stats.strength);
        query.bind(":constitution", stats.constitution);
        query.bind(":intelligence", stats.intelligence);
        query.bind(":dexterity", stats.dexterity);
        query.bind(":axe", skills.axe);
        query.bind(":dagger", skills.dagger);
        query.bind(":unarmed", skills.unarmed);
        query.bind(":hammer", skills.hammer);
        query.bind(":polearm", skills.polearm);
        query.bind(":spear", skills.spear);
        query.bind(":staff", skills.staff);
        query.bind(":sword", skills.sword);
        query.bind(":archery", skills.archery);
        query.bind(":crossbow", skills.crossbow);
        query.bind(":sling", skills.sling);
        query.bind(":thrown", skills.thrown);
        query.bind(":armor", skills.armor);
        query.bind(":dualweapon", skills.dualweapon);
        query.bind(":shield", skills.shield);
        query.bind(":bardic", skills.bardic);
        query.bind(":conjuring", skills.conjuring);
        query.bind(":druidic", skills.druidic);
        query.bind(":illusion", skills.illusion);
        query.bind(":necromancy", skills.necromancy);
        query.bind(":sorcery", skills.sorcery);
        query.bind(":shamanic", skills.shamanic);
        query.bind(":summoning", skills.summoning);
        query.bind(":spellcraft", skills.spellcraft);
        query.bind(":focus", skills.focus);
        query.bind(":armorsmithing", skills.armorsmithing);
        query.bind(":tailoring", skills.tailoring);
        query.bind(":fletching", skills.fletching);
        query.bind(":weaponsmithing", skills.weaponsmithing);
        query.bind(":alchemy", skills.alchemy);
        query.bind(":lapidary", skills.lapidary);
        query.bind(":calligraphy", skills.calligraphy);
        query.bind(":enchanting", skills.enchanting);
        query.bind(":herbalism", skills.herbalism);
        query.bind(":hunting", skills.hunting);
        query.bind(":mining", skills.mining);
        query.bind(":bargaining", skills.bargaining);
        query.bind(":camping", skills.camping);
        query.bind(":firstaid", skills.firstaid);
        query.bind(":lore", skills.lore);
        query.bind(":picklocks", skills.picklocks);
        query.bind(":scouting", skills.scouting);
        query.bind(":search", skills.search);
        query.bind(":stealth", skills.stealth);
        query.bind(":traps", skills.traps);
        query.bind(":aeolandis", skills.aeolandis);
        query.bind(":hieroform", skills.hieroform);
        query.bind(":highgundis", skills.highgundis);
        query.bind(":oldpraxic", skills.oldpraxic);
        query.bind(":praxic", skills.praxic);
        query.bind(":runic", skills.runic);
        query.bind(":head", equip.head);
        query.bind(":chest", equip.chest);
        query.bind(":arms", equip.arms);
        query.bind(":hands", equip.hands);
        query.bind(":legs", equip.legs);
        query.bind(":feet", equip.feet);
        query.bind(":cloak", equip.cloak);
        query.bind(":necklace", equip.necklace);
        query.bind(":ringone", equip.ringone);
        query.bind(":ringtwo", equip.ringtwo);
        query.bind(":righthand", equip.righthand);
        query.bind(":lefthand", equip.lefthand);
        query.bind(":zone", c.GetZone());
        query.bind(":x", location.x);
        query.bind(":y", location.y);
        query.bind(":z", location.z);
        query.bind(":pitch", location.pitch);
        query.bind(":yaw", location.yaw);
        int rows_modified = query.exec();
        if(rows_modified < 1){
            std::cout << "Could not insert character record! " << rows_modified << "Rows were modified" << std::endl;
            return 0;
        }
        else {
            // Get the id of the newly inserted record
            return db.getLastInsertRowid();
        }
    }
    catch (std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
        std::cout << query_string << std::endl;
        return 0;
    }
}

// Lists all the accounts
void SQLConnector::ListAccounts() {
    std::string cmd("select * from accounts ORDER BY accountname");
    SQLite::Statement query(db, cmd);
    while (query.executeStep()) {
        std::cout << "Executing step for insert account..." << std::endl;
    }
}

// TODO: Create a view instead of creating a join table right here
std::vector<std::string> SQLConnector::GetPlayerList(std::string accountname) {
    std::vector<std::string> players;

    std::string cmd("select firstname, lastname FROM players JOIN accounts ON players.account_id = accounts.id ORDER BY accountname WHERE accountname = ?");
    SQLite::Statement query(db, cmd);
    query.bind(1, accountname);

    while (query.executeStep()) {
        std::string name = query.getColumn(0);
        std::string lastname = query.getColumn(1);
        name.append(" ").append(lastname);
        players.push_back(name);
    }
    return players;
}

std::set<Npc> SQLConnector::PopulateNpcList() {
    std::set<Npc> npcs;
    std::stringstream query;
    // char *errorMessage = NULL;
    // query << "SELECT * FROM npcs";
    // sqls = sqlite3_exec(database, query.str().c_str(), ParseNpcs, (void*)&npcs, &errorMessage);
    // if (sqls != SQLITE_OK) {
    //     std::cout << "Could not Execute SQL query! Return Code:" << sqls << std::endl;
    // }
    // if (errorMessage) {
    //     std::cout << "SQL ERROR MESSAGE: " << errorMessage << std::endl;
    //     sqlite3_free(errorMessage);
    // }
    return npcs;
}

// Return the salt for the passed account
bool SQLConnector::GetAccountSalt(char *accountName, char *saltStringHex) {
    // Sanitize the account name before preceeding
    if (!utils::SanitizeAccountName(accountName)) {
        return false;
    }
    char *errorMessage = NULL;
    std::stringstream query;
    query << "select salt from accounts where accountname = \"" << accountName << "\"";
    // The fourth param is passed to the callback function as a void pointer to the first param
    // sqls = sqlite3_exec(database, query.str().c_str(), &ReturnStringCallback, saltStringHex, &errorMessage);
    // if (sqls != SQLITE_OK) {
    //     std::cout << "Could not Execute SQL query! Return Code:" << sqls << std::endl;
    // }
    // if (errorMessage) {
    //     // Print out the error message if any
    //     std::cout << "SQL ERROR MESSAGE: " << errorMessage << std::endl;
    //     // Free the error message, since it was alloced in exec()
    //     sqlite3_free(errorMessage);
    //     return false;
    // }
    // Check to see if the salt was retrieved
    if (utils::SanitizeHexString(saltStringHex)) {
        return true;
    }
    else {
        return false;
    }

}

// Returns the key to the passed account
int SQLConnector::GetAccountKey(char *accountName, char *keyStringHex) {
    // // Sanitize the account name before preceeding
    // if (!utils::SanitizeAccountName(accountName)) {
    //     return 0;
    // }
    // char *errorMessage = NULL;
    // std::stringstream query;
    // query << "select key from accounts where accountname = \"" << accountName << "\"";
    // //std::cout << query.str() << std::endl;
    // // The fourth param is passed to the callback function as a void pointer to the first param
    // sqls = sqlite3_exec(database, query.str().c_str(), &ReturnStringCallback, keyStringHex, &errorMessage);
    // if (sqls != SQLITE_OK) {
    //     std::cout << "Could not Execute SQL query! Return Code:" << sqls << std::endl;
    // }
    // if (errorMessage) {
    //     // Print out the error message if any
    //     std::cout << "SQL ERROR MESSAGE: " << errorMessage << std::endl;
    //     // Free the error message, since it was alloced in exec()
    //     sqlite3_free(errorMessage);
    // }
    return 1;
}

// -------------------------------------- Callback Functions ---------------------------------------

// A generic callback function to sqlite3_exec() that copies a c string in the first column
// // of the returned row into stringToReturn. This function assumes that only one record will be
// // in the result set, or else the return value will be the value in the last row processed.
// static int ReturnStringCallback(void *stringToReturn, int argc, char **argv, char **azColName) {
//     strcpy((char *)stringToReturn, argv[0]);
//     return 0;
// }

// static int ParseNpcs(void * npcs, int argc, char ** argv, char ** azColName) {
//     std::set<Npc> * npcset = (std::set<Npc> *)npcs;
//     //npcset->insert(Npc temp(argv));
//     return 0;
// }

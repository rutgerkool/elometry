#include "utils/database/repositories/ClubRepository.h"
#include "utils/Season.h"
#include <iostream>

ClubRepository::ClubRepository(Database& database) {
    db = database.getConnection();
}

std::vector<Club> ClubRepository::fetchClubs(int clubId) {
    std::vector<Club> clubs;
    sqlite3_stmt *stmt;
    
    int currentSeasonYear = getCurrentSeasonYear(); 

    std::string query = "SELECT club_id, name FROM clubs WHERE last_season = ?";

    if (clubId != -1) { 
        query += " AND club_id = ?";
    }

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, currentSeasonYear);

        if (clubId != -1) {
            sqlite3_bind_int(stmt, 2, clubId);
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Club club;
            
            club.clubId = sqlite3_column_int(stmt, 0);
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            club.clubName = name ? std::string(reinterpret_cast<const char*>(name)) : "";
            
            clubs.push_back(club);
        }
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    
    return clubs;
}

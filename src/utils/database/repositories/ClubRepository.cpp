#include "utils/database/repositories/ClubRepository.h"
#include "utils/Season.h"
#include <iostream>
#include <tuple>
#include <algorithm>

ClubRepository::ClubRepository(Database& database) 
    : m_db(database.getConnection()) {
}

std::vector<Club> ClubRepository::fetchClubs(std::optional<int> clubId) const {
    const int currentSeasonYear = getCurrentSeasonYear();
    
    if (clubId.has_value()) {
        return executeQuery("SELECT club_id, name FROM clubs WHERE last_season = ? AND club_id = ?", 
                           currentSeasonYear, *clubId);
    } else {
        return executeQuery("SELECT club_id, name FROM clubs WHERE last_season = ?", 
                           currentSeasonYear);
    }
}

std::optional<Club> ClubRepository::fetchClubById(int clubId) const {
    auto clubs = fetchClubs(clubId);
    return clubs.empty() ? std::nullopt : std::optional{clubs.front()};
}

template<std::integral... Params>
std::vector<Club> ClubRepository::executeQuery(std::string_view query, Params... params) const {
    std::vector<Club> clubs;
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(m_db, query.data(), static_cast<int>(query.size()), &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(m_db) << std::endl;
        return clubs;
    }
    
    if constexpr (sizeof...(params) > 0) {
        int paramIndex = 1;
        (sqlite3_bind_int(stmt, paramIndex++, params), ...);
    }
    
    try {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            clubs.push_back(extractClubFromStatement(stmt));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error extracting club data: " << e.what() << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return clubs;
}

Club ClubRepository::extractClubFromStatement(sqlite3_stmt* stmt) {
    if (!stmt) {
        throw std::invalid_argument("Invalid statement");
    }
    
    Club club;
    club.clubId = sqlite3_column_int(stmt, 0);
    
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    club.clubName = name ? std::string(reinterpret_cast<const char*>(name)) : "";
    
    return club;
}

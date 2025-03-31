#ifndef CLUBREPOSITORY_H
#define CLUBREPOSITORY_H

#include "utils/database/Database.h"
#include <vector>
#include <string_view>
#include <optional>
#include <concepts>

struct Club {
    int clubId;
    std::string clubName;
    
    auto operator<=>(const Club&) const = default;
};

class ClubRepository {
public:
    explicit ClubRepository(Database& database);
    ~ClubRepository() = default;
    
    ClubRepository(const ClubRepository&) = delete;
    ClubRepository& operator=(const ClubRepository&) = delete;
    ClubRepository(ClubRepository&&) noexcept = default;
    ClubRepository& operator=(ClubRepository&&) noexcept = default;
    
    [[nodiscard]] std::vector<Club> fetchClubs(std::optional<int> clubId = std::nullopt) const;
    [[nodiscard]] std::optional<Club> fetchClubById(int clubId) const;
    
private:
    template<std::integral... Params>
    [[nodiscard]] std::vector<Club> executeQuery(std::string_view query, Params... params) const;
    
    [[nodiscard]] static Club extractClubFromStatement(sqlite3_stmt* stmt);
    
    sqlite3* m_db;
};

#endif

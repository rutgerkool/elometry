#ifndef LINEUPTYPES_H
#define LINEUPTYPES_H

#include <string>
#include <vector>

enum class PositionType {
    STARTING,
    BENCH,
    RESERVE
};

struct Formation {
    int id;
    std::string name;
    std::string description;
};

struct PlayerPosition {
    int playerId;
    PositionType positionType;
    std::string fieldPosition;
    int order;
};

struct Lineup {
    int lineupId;
    int teamId;
    int formationId;
    std::string formationName;
    std::string name;
    bool isActive;
    std::vector<PlayerPosition> playerPositions;
};

#endif 

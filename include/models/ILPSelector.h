#ifndef ILP_SELECTOR_H
#define ILP_SELECTOR_H

#include "models/PlayerRating.h"
#include <vector>
#include <string>
#include <glpk.h>
#include <cstdint>

class ILPSelector {
public:
    ILPSelector(std::vector<std::pair<int, Player>>& p, 
                const std::vector<std::string>& pos, 
                int64_t b);

    std::vector<std::pair<int, Player>> selectTeam();

private:
    struct Variable {
        size_t playerIdx;
        size_t positionIdx;
        int varIdx;
        double rating;
        int64_t cost;
    };

    std::vector<std::pair<int, Player>>& players;
    const std::vector<std::string>& requiredPositions;
    int64_t budget;

    std::vector<Variable> createVariables();
    void addBudgetConstraint(glp_prob* lp, const std::vector<Variable>& vars);
    void addPositionConstraints(glp_prob* lp, const std::vector<Variable>& vars);
    void setupObjectiveFunction(glp_prob* lp, const std::vector<Variable>& vars);
};

#endif

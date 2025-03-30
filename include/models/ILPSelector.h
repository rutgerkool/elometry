#ifndef ILP_SELECTOR_H
#define ILP_SELECTOR_H

#include "models/PlayerRating.h"
#include <vector>
#include <string>
#include <string_view>
#include <span>
#include <glpk.h>
#include <cstdint>

class ILPSelector {
    public:
        ILPSelector(std::span<const std::pair<int, Player>> players, 
                    std::span<const std::string> requiredPositions,
                    int64_t budget);

        [[nodiscard]] std::vector<std::pair<int, Player>> selectTeam() const;

    private:
        struct Variable {
            size_t playerIdx;
            size_t positionIdx;
            int varIdx;
            double rating;
            int64_t cost;
        };

        std::span<const std::pair<int, Player>> m_players;
        std::span<const std::string> m_requiredPositions;
        int64_t m_budget;

        [[nodiscard]] std::vector<Variable> createVariables() const;
        void addBudgetConstraint(glp_prob* lp, std::span<const Variable> vars) const;
        void addPositionConstraints(glp_prob* lp, std::span<const Variable> vars) const;
        void setupObjectiveFunction(glp_prob* lp, std::span<const Variable> vars) const;
        [[nodiscard]] glp_prob* createProblem() const;
        void configureVariables(glp_prob* lp, std::span<const Variable> vars) const;
        [[nodiscard]] std::vector<std::pair<int, Player>> extractSolution(
            glp_prob* lp, 
            std::span<const Variable> vars) const;
};

#endif

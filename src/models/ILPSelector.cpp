#include "models/ILPSelector.h"
#include <algorithm>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <numeric>

ILPSelector::ILPSelector(std::span<const std::pair<int, Player>> players, 
                         std::span<const std::string> requiredPositions,
                         int64_t budget)
    : m_players(players)
    , m_requiredPositions(requiredPositions)
    , m_budget(budget) {
    if (budget < 0 || budget > std::numeric_limits<int64_t>::max() / 2) {
        throw std::invalid_argument("Budget value out of valid range");
    }
}

std::vector<ILPSelector::Variable> ILPSelector::createVariables() const {
    std::vector<Variable> vars;
    int varIdx = 1;
    
    for (size_t i = 0; i < m_players.size(); i++) {
        const auto& [_, player] = m_players[i];
        
        for (size_t j = 0; j < m_requiredPositions.size(); j++) {
            if (player.subPosition == m_requiredPositions[j]) {
                vars.push_back({
                    .playerIdx = i,
                    .positionIdx = j,
                    .varIdx = varIdx++,
                    .rating = player.rating,
                    .cost = static_cast<int64_t>(player.marketValue)
                });
            }
        }
    }
    
    return vars;
}

void ILPSelector::addBudgetConstraint(glp_prob* lp, std::span<const Variable> vars) const {
    if (m_budget > std::numeric_limits<double>::max()) {
        throw std::overflow_error("Budget exceeds maximum double value");
    }

    const int rowIdx = glp_add_rows(lp, 1);
    glp_set_row_name(lp, rowIdx, "budget");
    glp_set_row_bnds(lp, rowIdx, GLP_UP, 0.0, static_cast<double>(m_budget));

    std::vector<int> indices(vars.size() + 1);
    std::vector<double> coeffs(vars.size() + 1);
    
    for (size_t i = 0; i < vars.size(); i++) {
        indices[i + 1] = vars[i].varIdx;
        
        if (vars[i].cost > std::numeric_limits<double>::max()) {
            throw std::overflow_error("Cost value exceeds maximum double value");
        }
        
        coeffs[i + 1] = static_cast<double>(vars[i].cost);
    }
    
    glp_set_mat_row(lp, rowIdx, static_cast<int>(vars.size()), indices.data(), coeffs.data());
}

void ILPSelector::addPositionConstraints(glp_prob* lp, std::span<const Variable> vars) const {
    for (size_t pos = 0; pos < m_requiredPositions.size(); pos++) {
        auto posVars = vars | std::views::filter([pos](const Variable& var) {
            return var.positionIdx == pos;
        });
        
        std::vector<Variable> positionVars(posVars.begin(), posVars.end());
        
        if (positionVars.empty()) {
            continue;
        }
        
        const int rowIdx = glp_add_rows(lp, 1);
        const std::string rowName = "pos_" + std::to_string(pos);
        glp_set_row_name(lp, rowIdx, rowName.c_str());
        glp_set_row_bnds(lp, rowIdx, GLP_FX, 1.0, 1.0);
        
        std::vector<int> indices(positionVars.size() + 1);
        std::vector<double> coeffs(positionVars.size() + 1);
        
        for (size_t i = 0; i < positionVars.size(); i++) {
            indices[i + 1] = positionVars[i].varIdx;
            coeffs[i + 1] = 1.0;
        }
        
        glp_set_mat_row(lp, rowIdx, static_cast<int>(positionVars.size()), 
                       indices.data(), coeffs.data());
    }
}

void ILPSelector::setupObjectiveFunction(glp_prob* lp, std::span<const Variable> vars) const {
    const double maxRating = std::transform_reduce(
        vars.begin(), vars.end(),
        0.0,
        [](double a, double b) { return std::max(a, b); },
        [](const Variable& var) { return var.rating; }
    );
    
    for (const auto& var : vars) {
        double coefficient = var.rating;

        if (var.cost <= 0) {
            coefficient -= maxRating * 2;
        }

        glp_set_obj_coef(lp, var.varIdx, coefficient);
    }
}

glp_prob* ILPSelector::createProblem() const {
    glp_prob* lp = glp_create_prob();
    glp_set_prob_name(lp, "team_selection");
    glp_set_obj_dir(lp, GLP_MAX);
    
    return lp;
}

void ILPSelector::configureVariables(glp_prob* lp, std::span<const Variable> vars) const {
    glp_add_cols(lp, static_cast<int>(vars.size()));
    
    for (const auto& var : vars) {
        const std::string colName = "x_" + std::to_string(var.playerIdx) + "_" + 
                                   std::to_string(var.positionIdx);
        
        glp_set_col_name(lp, var.varIdx, colName.c_str());
        glp_set_col_kind(lp, var.varIdx, GLP_BV);
        glp_set_col_bnds(lp, var.varIdx, GLP_DB, 0.0, 1.0);
    }
}

std::vector<std::pair<int, Player>> ILPSelector::extractSolution(
    glp_prob* lp, 
    std::span<const Variable> vars) const {
    std::vector<std::pair<int, Player>> result;
    
    for (const auto& var : vars) {
        if (glp_mip_col_val(lp, var.varIdx) > 0.5) {
            result.push_back(m_players[var.playerIdx]);
        }
    }
    
    return result;
}

std::vector<std::pair<int, Player>> ILPSelector::selectTeam() const {
    glp_prob* lp = createProblem();
    
    try {
        const auto vars = createVariables();
        configureVariables(lp, vars);
        
        addBudgetConstraint(lp, vars);
        addPositionConstraints(lp, vars);
        setupObjectiveFunction(lp, vars);
        
        glp_iocp parm;
        glp_init_iocp(&parm);
        parm.presolve = GLP_ON;
        parm.msg_lev = GLP_MSG_OFF;
        
        const int err = glp_intopt(lp, &parm);
        
        std::vector<std::pair<int, Player>> result;
        if (err == 0) {
            result = extractSolution(lp, vars);
        }
        
        glp_delete_prob(lp);
        return result;
    }
    catch (const std::exception&) {
        glp_delete_prob(lp);
        throw;
    }
}

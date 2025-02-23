#include "models/ILPSelector.h"
#include <algorithm>
#include <map>
#include <set>
#include <iostream>
#include <limits>

ILPSelector::ILPSelector(std::vector<std::pair<int, Player>>& p, 
                        const std::vector<std::string>& pos, 
                        int64_t b) 
    : players(p)
    , requiredPositions(pos)
    , budget(b) {
    if (budget < 0 || budget > std::numeric_limits<int64_t>::max() / 2) {
        throw std::invalid_argument("Budget value out of valid range");
    }
}

std::vector<ILPSelector::Variable> ILPSelector::createVariables() {
    std::vector<Variable> vars;
    size_t varIdx = 1;
    
    for (size_t i = 0; i < players.size(); i++) {
        const auto& player = players[i];
        for (size_t j = 0; j < requiredPositions.size(); j++) {
            if (player.second.subPosition == requiredPositions[j]) {
                Variable var;
                var.playerIdx = i;
                var.positionIdx = j;
                var.varIdx = varIdx++;
                var.rating = player.second.rating;
                var.cost = static_cast<int64_t>(player.second.marketValue); 
                vars.push_back(var);
            }
        }
    }
    
    return vars;
}

void ILPSelector::addBudgetConstraint(glp_prob* lp, const std::vector<Variable>& vars) {
    int rowIdx = glp_add_rows(lp, 1);
    glp_set_row_name(lp, rowIdx, "budget");
    
    if (budget > std::numeric_limits<double>::max()) {
        throw std::overflow_error("Budget exceeds maximum double value");
    }
    glp_set_row_bnds(lp, rowIdx, GLP_UP, 0.0, static_cast<double>(budget));

    std::vector<int> indices(vars.size() + 1);
    std::vector<double> coeffs(vars.size() + 1);
    
    for (size_t i = 0; i < vars.size(); i++) {
        indices[i + 1] = vars[i].varIdx;
        if (vars[i].cost > std::numeric_limits<double>::max()) {
            throw std::overflow_error("Cost value exceeds maximum double value");
        }
        coeffs[i + 1] = static_cast<double>(vars[i].cost);
    }
    
    glp_set_mat_row(lp, rowIdx, vars.size(), indices.data(), coeffs.data());
}

void ILPSelector::addPositionConstraints(glp_prob* lp, const std::vector<Variable>& vars) {
    for (size_t pos = 0; pos < requiredPositions.size(); pos++) {
        std::vector<Variable> posVars;
        for (const auto& var : vars) {
            if (var.positionIdx == pos) {
                posVars.push_back(var);
            }
        }
        
        if (!posVars.empty()) {
            int rowIdx = glp_add_rows(lp, 1);
            glp_set_row_name(lp, rowIdx, ("pos_" + std::to_string(pos)).c_str());
            glp_set_row_bnds(lp, rowIdx, GLP_FX, 1.0, 1.0);
            
            std::vector<int> indices(posVars.size() + 1);
            std::vector<double> coeffs(posVars.size() + 1);
            
            for (size_t i = 0; i < posVars.size(); i++) {
                indices[i + 1] = posVars[i].varIdx;
                coeffs[i + 1] = 1.0;
            }
            
            glp_set_mat_row(lp, rowIdx, posVars.size(), indices.data(), coeffs.data());
        }
    }
}

void ILPSelector::setupObjectiveFunction(glp_prob* lp, const std::vector<Variable>& vars) {
    double maxRating = 0;
    for (const auto& var : vars) {
        maxRating = std::max(maxRating, var.rating);
    }
    
    for (const auto& var : vars) {
        double coefficient = var.rating;
        if (var.cost <= 0) {
            coefficient -= maxRating * 2;
        }
        glp_set_obj_coef(lp, var.varIdx, coefficient);
    }
}

std::vector<std::pair<int, Player>> ILPSelector::selectTeam() {
    glp_prob* lp = glp_create_prob();
    glp_set_prob_name(lp, "team_selection");
    glp_set_obj_dir(lp, GLP_MAX);
    
    auto vars = createVariables();
    glp_add_cols(lp, vars.size());
    
    for (const auto& var : vars) {
        glp_set_col_name(lp, var.varIdx, 
            ("x_" + std::to_string(var.playerIdx) + "_" + 
             std::to_string(var.positionIdx)).c_str());
        glp_set_col_kind(lp, var.varIdx, GLP_BV);
        glp_set_col_bnds(lp, var.varIdx, GLP_DB, 0.0, 1.0);
    }
    
    try {
        addBudgetConstraint(lp, vars);
        addPositionConstraints(lp, vars);
        setupObjectiveFunction(lp, vars);
        
        glp_iocp parm;
        glp_init_iocp(&parm);

        parm.presolve = GLP_ON;
        parm.msg_lev = GLP_MSG_OFF; 
        
        int err = glp_intopt(lp, &parm);
        
        std::vector<std::pair<int, Player>> result;
        if (err == 0) {
            double obj_val = glp_mip_obj_val(lp);
            
            for (const auto& var : vars) {
                if (glp_mip_col_val(lp, var.varIdx) > 0.5) {
                    result.push_back(players[var.playerIdx]);
                    std::cout << "Selected player: " << players[var.playerIdx].second.name 
                        << " - Club: " << players[var.playerIdx].second.clubName
                        << " - Rating: " << players[var.playerIdx].second.rating
                        << " - Market Value: " << players[var.playerIdx].second.marketValue
                        << " - Position: " << players[var.playerIdx].second.subPosition 
                        << std::endl;
                }
            }
        }
        
        glp_delete_prob(lp);
        return result;
    }
    catch (const std::exception& e) {
        glp_delete_prob(lp);
        throw;
    }
}

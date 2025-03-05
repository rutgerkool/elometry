#include "gui/models/TeamSelectModel.h"
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <algorithm>

TeamSelectModel::TeamSelectModel(const std::vector<Team>& teams, int playerId, QObject *parent)
    : QAbstractListModel(parent)
    , allTeams(teams)
    , filteredTeams(teams)
    , playerId(playerId)
{
    for (const auto& team : allTeams) {
        for (const auto& player : team.players) {
            if (player.playerId == playerId) {
                initialSelectedTeamIds.insert(team.teamId);
                selectedTeamIds.insert(team.teamId);
                break;
            }
        }
    }
}

int TeamSelectModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(filteredTeams.size());
}

QVariant TeamSelectModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= filteredTeams.size()) return QVariant();

    const auto& team = filteredTeams[index.row()];
    bool selected = isTeamSelected(team.teamId);
    bool initiallySelected = initialSelectedTeamIds.find(team.teamId) != initialSelectedTeamIds.end();

    if (role == Qt::DisplayRole) {
        return QString::fromStdString(team.teamName);
    }
    else if (role == Qt::CheckStateRole) {
        return selected ? Qt::Checked : Qt::Unchecked;
    }
    else if (role == Qt::BackgroundRole) {
        if (selected) {
            return QColor(45, 65, 90);
        }
        return (index.row() % 2) ? QColor(45, 45, 45) : QColor(53, 53, 53);
    }
    else if (role == Qt::ForegroundRole) {
        if (selected) {
            return QColor(220, 220, 220);
        }
        return QColor(210, 210, 210);
    }
    else if (role == Qt::UserRole) {
        return team.teamId;
    }
    else if (role == Qt::ToolTipRole) {
        QString tooltip = QString::fromStdString(team.teamName);
        if (initiallySelected) {
            tooltip += " (Player already in this team)";
        }
        return tooltip;
    }
    else if (role == Qt::FontRole) {
        if (initiallySelected) {
            QFont font;
            font.setItalic(true);
            return font;
        }
    }

    return QVariant();
}

bool TeamSelectModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || role != Qt::CheckStateRole) 
        return false;
    
    if (index.row() >= filteredTeams.size())
        return false;
    
    int teamId = filteredTeams[index.row()].teamId;
    if (value == Qt::Checked) {
        selectedTeamIds.insert(teamId);
    } else {
        selectedTeamIds.erase(teamId);
    }
    
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags TeamSelectModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void TeamSelectModel::setFilter(const QString& filter) {
    beginResetModel();
    currentFilter = filter;
    
    filteredTeams = allTeams;
    
    if (!filter.isEmpty()) {
        filteredTeams.erase(
            std::remove_if(filteredTeams.begin(), filteredTeams.end(),
                [&filter](const auto& team) {
                    return !QString::fromStdString(team.teamName)
                        .contains(filter, Qt::CaseInsensitive);
                }
            ),
            filteredTeams.end()
        );
    }
    
    endResetModel();
}

void TeamSelectModel::toggleTeamSelection(const QModelIndex &index) {
    if (!index.isValid() || index.row() >= filteredTeams.size()) return;
    
    int teamId = filteredTeams[index.row()].teamId;
    
    if (isTeamSelected(teamId)) {
        selectedTeamIds.erase(teamId);
    } else {
        selectedTeamIds.insert(teamId);
    }
    
    emit dataChanged(index, index);
}

bool TeamSelectModel::isTeamSelected(int teamId) const {
    return selectedTeamIds.find(teamId) != selectedTeamIds.end();
}

void TeamSelectModel::selectTeam(int teamId) {
    selectedTeamIds.insert(teamId);
    
    for (size_t i = 0; i < filteredTeams.size(); ++i) {
        if (filteredTeams[i].teamId == teamId) {
            emit dataChanged(index(static_cast<int>(i), 0), index(static_cast<int>(i), 0));
            break;
        }
    }
}

void TeamSelectModel::deselectTeam(int teamId) {
    selectedTeamIds.erase(teamId);
    
    for (size_t i = 0; i < filteredTeams.size(); ++i) {
        if (filteredTeams[i].teamId == teamId) {
            emit dataChanged(index(static_cast<int>(i), 0), index(static_cast<int>(i), 0));
            break;
        }
    }
}

std::vector<int> TeamSelectModel::getSelectedTeamIds() const {
    std::vector<int> teamIds;
    for (int id : selectedTeamIds) {
        teamIds.push_back(id);
    }
    return teamIds;
}

int TeamSelectModel::getSelectedCount() const {
    return static_cast<int>(selectedTeamIds.size());
}

int TeamSelectModel::getTeamCount() const {
    return static_cast<int>(allTeams.size());
}

std::set<int> TeamSelectModel::getInitialSelectedTeamIds() const {
    return initialSelectedTeamIds;
}

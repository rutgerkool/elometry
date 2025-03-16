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
    initializeSelectedTeams(teams, playerId);
}

void TeamSelectModel::initializeSelectedTeams(const std::vector<Team>& teams, int playerId) {
    for (const auto& team : teams) {
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
    if (!index.isValid() || index.row() >= filteredTeams.size()) 
        return QVariant();

    const auto& team = filteredTeams[index.row()];
    bool selected = isTeamSelected(team.teamId);

    switch (role) {
        case Qt::DisplayRole:
            return getDisplayData(team);
        case Qt::CheckStateRole:
            return selected ? Qt::Checked : Qt::Unchecked;
        case Qt::BackgroundRole:
        case Qt::ForegroundRole:
            return getDecorationData(team, index.row(), selected, role);
        case Qt::UserRole:
            return team.teamId;
        case Qt::ToolTipRole:
            return getTeamTooltip(team);
        case Qt::FontRole:
            return getTeamFontData(team);
        default:
            return QVariant();
    }
}

QVariant TeamSelectModel::getDisplayData(const Team& team) const {
    return QString::fromStdString(team.teamName);
}

QVariant TeamSelectModel::getDecorationData(const Team& team, int row, bool selected, int role) const {
    if (role == Qt::BackgroundRole) {
        if (selected) {
            return QColor(45, 65, 90);
        }
        return (row % 2) ? QColor(45, 45, 45) : QColor(53, 53, 53);
    } else {
        return selected ? QColor(220, 220, 220) : QColor(210, 210, 210);
    }
}

QVariant TeamSelectModel::getTeamFontData(const Team& team) const {
    bool initiallySelected = initialSelectedTeamIds.find(team.teamId) != initialSelectedTeamIds.end();
    
    if (initiallySelected) {
        QFont font;
        font.setItalic(true);
        return font;
    }
    
    return QVariant();
}

QString TeamSelectModel::getTeamTooltip(const Team& team) const {
    QString tooltip = QString::fromStdString(team.teamName);
    bool initiallySelected = initialSelectedTeamIds.find(team.teamId) != initialSelectedTeamIds.end();
    
    if (initiallySelected) {
        tooltip += " (Player already in this team)";
    }
    
    return tooltip;
}

bool TeamSelectModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || role != Qt::CheckStateRole || index.row() >= filteredTeams.size()) 
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
    applyFilter();
    endResetModel();
}

void TeamSelectModel::applyFilter() {
    filteredTeams = allTeams;
    
    if (currentFilter.isEmpty())
        return;
    
    filteredTeams.erase(
        std::remove_if(filteredTeams.begin(), filteredTeams.end(),
            [this](const auto& team) {
                return !QString::fromStdString(team.teamName)
                    .contains(currentFilter, Qt::CaseInsensitive);
            }
        ),
        filteredTeams.end()
    );
}

void TeamSelectModel::toggleTeamSelection(const QModelIndex &index) {
    if (!index.isValid() || index.row() >= filteredTeams.size()) 
        return;
    
    int teamId = filteredTeams[index.row()].teamId;
    
    if (isTeamSelected(teamId)) {
        deselectTeam(teamId);
    } else {
        selectTeam(teamId);
    }
}

bool TeamSelectModel::isTeamSelected(int teamId) const {
    return selectedTeamIds.find(teamId) != selectedTeamIds.end();
}

void TeamSelectModel::selectTeam(int teamId) {
    selectedTeamIds.insert(teamId);
    notifyTeamDataChanged(teamId);
}

void TeamSelectModel::deselectTeam(int teamId) {
    selectedTeamIds.erase(teamId);
    notifyTeamDataChanged(teamId);
}

void TeamSelectModel::notifyTeamDataChanged(int teamId) {
    for (size_t i = 0; i < filteredTeams.size(); ++i) {
        if (filteredTeams[i].teamId == teamId) {
            emit dataChanged(index(static_cast<int>(i), 0), index(static_cast<int>(i), 0));
            break;
        }
    }
}

std::vector<int> TeamSelectModel::getSelectedTeamIds() const {
    std::vector<int> teamIds;
    teamIds.reserve(selectedTeamIds.size());
    
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

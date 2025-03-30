#include "gui/models/TeamSelectModel.h"
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <algorithm>
#include <ranges>

TeamSelectModel::TeamSelectModel(const std::vector<Team>& teams, int playerId, QObject* parent)
    : QAbstractListModel(parent)
    , allTeams(teams)
    , filteredTeams(teams)
    , playerId(playerId)
{
    initializeSelectedTeams(teams, playerId);
}

void TeamSelectModel::initializeSelectedTeams(const std::vector<Team>& teams, int playerId) {
    for (const auto& team : teams) {
        const auto playerIterator = std::ranges::find_if(team.players, 
            [playerId](const auto& player) { return player.playerId == playerId; });
            
        if (playerIterator != team.players.end()) {
            initialSelectedTeamIds.insert(team.teamId);
            selectedTeamIds.insert(team.teamId);
        }
    }
}

int TeamSelectModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(filteredTeams.size());
}

QVariant TeamSelectModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || static_cast<size_t>(index.row()) >= filteredTeams.size()) {
        return QVariant();
    }

    const auto& team = filteredTeams[index.row()];
    const bool selected = isTeamSelected(team.teamId);

    switch (role) {
        case Qt::DisplayRole:
            return getDisplayData(team);
        case Qt::CheckStateRole:
            return selected ? Qt::Checked : Qt::Unchecked;
        case Qt::BackgroundRole:
        case Qt::ForegroundRole:
            return getDecorationData(index.row(), selected, role);
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

QVariant TeamSelectModel::getDecorationData(int row, bool selected, int role) const {
    if (role == Qt::BackgroundRole) {
        return selected ? QColor(45, 65, 90) : ((row % 2) ? QColor(45, 45, 45) : QColor(53, 53, 53));
    } 
    
    return selected ? QColor(220, 220, 220) : QColor(210, 210, 210);
}

QVariant TeamSelectModel::getTeamFontData(const Team& team) const {
    if (initialSelectedTeamIds.contains(team.teamId)) {
        QFont font;
        font.setItalic(true);
        return font;
    }
    
    return QVariant();
}

QString TeamSelectModel::getTeamTooltip(const Team& team) const {
    QString tooltip = QString::fromStdString(team.teamName);
    
    if (initialSelectedTeamIds.contains(team.teamId)) {
        tooltip += " (Player already in this team)";
    }
    
    return tooltip;
}

bool TeamSelectModel::setData(const QModelIndex& index, const QVariant& value, int) {
    if (!index.isValid() || static_cast<size_t>(index.row()) >= filteredTeams.size()) {
        return false;
    }
    
    const int teamId = filteredTeams[index.row()].teamId;
    
    if (value == Qt::Checked) {
        selectedTeamIds.insert(teamId);
    } else {
        selectedTeamIds.erase(teamId);
    }
    
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags TeamSelectModel::flags(const QModelIndex& index) const {
    return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable) : Qt::NoItemFlags;
}

void TeamSelectModel::setFilter(const QString& filter) {
    beginResetModel();
    currentFilter = filter;
    applyFilter();
    endResetModel();
}

void TeamSelectModel::applyFilter() {
    if (currentFilter.isEmpty()) {
        filteredTeams = allTeams;
        return;
    }
    
    filteredTeams.clear();
    
    std::ranges::copy_if(allTeams, std::back_inserter(filteredTeams), 
        [this](const auto& team) {
            return QString::fromStdString(team.teamName).contains(currentFilter, Qt::CaseInsensitive);
        });
}

void TeamSelectModel::toggleTeamSelection(const QModelIndex& index) {
    if (!index.isValid() || static_cast<size_t>(index.row()) >= filteredTeams.size()) {
        return;
    }
    
    const int teamId = filteredTeams[index.row()].teamId;
    
    isTeamSelected(teamId) ? deselectTeam(teamId) : selectTeam(teamId);
}

bool TeamSelectModel::isTeamSelected(int teamId) const noexcept {
    return selectedTeamIds.contains(teamId);
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
    const auto it = std::ranges::find_if(filteredTeams, 
        [teamId](const auto& team) { return team.teamId == teamId; });
        
    if (it != filteredTeams.end()) {
        const int row = static_cast<int>(std::distance(filteredTeams.begin(), it));
        emit dataChanged(index(row, 0), index(row, 0));
    }
}

std::vector<int> TeamSelectModel::getSelectedTeamIds() const {
    return std::vector<int>(selectedTeamIds.begin(), selectedTeamIds.end());
}

int TeamSelectModel::getSelectedCount() const noexcept {
    return static_cast<int>(selectedTeamIds.size());
}

int TeamSelectModel::getTeamCount() const noexcept {
    return static_cast<int>(allTeams.size());
}

std::set<int> TeamSelectModel::getInitialSelectedTeamIds() const noexcept {
    return initialSelectedTeamIds;
}

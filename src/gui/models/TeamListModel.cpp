#include "gui/models/TeamListModel.h"
#include <algorithm>
#include <ranges>

TeamListModel::TeamListModel(TeamManager& teamManager, QObject* parent)
    : QAbstractListModel(parent)
    , m_teamManager(teamManager)
{
    refresh();
}

int TeamListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    return static_cast<int>(m_teams.size());
}

QVariant TeamListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_teams.size())) {
        return {};
    }

    const auto& team = m_teams[index.row()];
    
    switch (role) {
        case Qt::DisplayRole:
            return QString::fromStdString(team.teamName);
        case Qt::UserRole:
            return team.teamId;
        default:
            return {};
    }
}

Qt::ItemFlags TeamListModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool TeamListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || 
        index.row() >= static_cast<int>(m_teams.size()) || 
        role != Qt::EditRole) {
        return false;
    }
    
    const QString newName = value.toString();
    
    if (newName.isEmpty()) {
        return false;
    }
    
    const int teamId = m_teams[index.row()].teamId;
    
    if (!m_teamManager.updateTeamName(teamId, newName.toStdString())) {
        return false;
    }
    
    m_teams[index.row()].teamName = newName.toStdString();
    emit dataChanged(index, index, {Qt::DisplayRole});
    
    return true;
}

void TeamListModel::refresh() {
    beginResetModel();
    
    auto allTeams = m_teamManager.getAllTeams();
    m_teams = std::move(allTeams);
    
    endResetModel();
}

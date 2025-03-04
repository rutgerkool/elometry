#include "gui/models/TeamListModel.h"
#include <algorithm>

TeamListModel::TeamListModel(TeamManager& tm, QObject *parent)
    : QAbstractListModel(parent)
    , teamManager(tm)
{
    refresh();
}

int TeamListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(teams.size());
}

QVariant TeamListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        return QString::fromStdString(teams[index.row()].teamName);
    }
    else if (role == Qt::UserRole) {
        return teams[index.row()].teamId;
    }

    return QVariant();
}

Qt::ItemFlags TeamListModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool TeamListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= teams.size() || role != Qt::EditRole) {
        return false;
    }
    
    int teamId = teams[index.row()].teamId;
    QString newName = value.toString();
    
    if (newName.isEmpty()) {
        return false;
    }
    
    bool success = teamManager.updateTeamName(teamId, newName.toStdString());
    
    if (success) {
        teams[index.row()].teamName = newName.toStdString();
        emit dataChanged(index, index, {Qt::DisplayRole});
    }
    
    return success;
}

void TeamListModel::refresh() {
    beginResetModel();
    teams = teamManager.getAllTeams();
    endResetModel();
}

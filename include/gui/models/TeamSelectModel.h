#ifndef TEAMSELECTMODEL_H
#define TEAMSELECTMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <vector>
#include "services/TeamManager.h"
#include <set>

class TeamSelectModel : public QAbstractListModel {
    Q_OBJECT

    public:
        explicit TeamSelectModel(const std::vector<Team>& teams, int playerId, QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        
        void setFilter(const QString& filter);
        void toggleTeamSelection(const QModelIndex &index);
        bool isTeamSelected(int teamId) const;
        void selectTeam(int teamId);
        void deselectTeam(int teamId);
        std::vector<int> getSelectedTeamIds() const;
        int getSelectedCount() const;
        int getTeamCount() const;
        std::set<int> getInitialSelectedTeamIds() const;

    private:
        std::vector<Team> allTeams;
        std::vector<Team> filteredTeams;
        std::set<int> selectedTeamIds;
        std::set<int> initialSelectedTeamIds;
        int playerId;
        QString currentFilter;
};

#endif

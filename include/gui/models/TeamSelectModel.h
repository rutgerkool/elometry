#ifndef TEAMSELECTMODEL_H
#define TEAMSELECTMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QString>
#include <vector>
#include <set>
#include "services/TeamManager.h"

class TeamSelectModel final : public QAbstractListModel {
    Q_OBJECT

    public:
        explicit TeamSelectModel(const std::vector<Team>& teams, int playerId, QObject* parent = nullptr);

        [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
        
        void setFilter(const QString& filter);
        void toggleTeamSelection(const QModelIndex& index);
        [[nodiscard]] bool isTeamSelected(int teamId) const noexcept;
        void selectTeam(int teamId);
        void deselectTeam(int teamId);
        [[nodiscard]] std::vector<int> getSelectedTeamIds() const;
        [[nodiscard]] int getSelectedCount() const noexcept;
        [[nodiscard]] int getTeamCount() const noexcept;
        [[nodiscard]] std::set<int> getInitialSelectedTeamIds() const noexcept;

    private:
        void notifyTeamDataChanged(int teamId);
        void initializeSelectedTeams(const std::vector<Team>& teams, int playerId);
        void applyFilter();
        [[nodiscard]] QVariant getDisplayData(const Team& team) const;
        [[nodiscard]] QVariant getDecorationData(int row, bool selected, int role) const;
        [[nodiscard]] QVariant getTeamFontData(const Team& team) const;
        [[nodiscard]] QString getTeamTooltip(const Team& team) const;
        
        std::vector<Team> allTeams;
        std::vector<Team> filteredTeams;
        std::set<int> selectedTeamIds;
        std::set<int> initialSelectedTeamIds;
        int playerId;
        QString currentFilter;
};

#endif

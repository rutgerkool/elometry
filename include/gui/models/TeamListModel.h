#ifndef TEAMLISTMODEL_H
#define TEAMLISTMODEL_H

#include "models/PlayerRating.h"
#include "services/TeamManager.h"
#include <QtCore/QAbstractListModel>
#include <QtCore/QString>
#include <vector>
#include <span>

class TeamListModel final : public QAbstractListModel {
    Q_OBJECT

    public:
        explicit TeamListModel(TeamManager& teamManager, QObject* parent = nullptr);
        ~TeamListModel() override = default;

        TeamListModel(const TeamListModel&) = delete;
        TeamListModel& operator=(const TeamListModel&) = delete;
        TeamListModel(TeamListModel&&) = delete;
        TeamListModel& operator=(TeamListModel&&) = delete;

        [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    public slots:
        void refresh();

    private:
        TeamManager& m_teamManager;
        std::vector<Team> m_teams;
};

#endif

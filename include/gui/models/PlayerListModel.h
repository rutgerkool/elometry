#ifndef PLAYERLISTMODEL_H
#define PLAYERLISTMODEL_H

#include "models/PlayerRating.h"
#include "services/TeamManager.h"
#include <QtCore/QAbstractTableModel>
#include <QtCore/QString>
#include <vector>
#include <string>
#include <span>
#include <optional>

class PlayerListModel final : public QAbstractTableModel {
    Q_OBJECT

    public:
        explicit PlayerListModel(std::span<const std::pair<int, Player>> players, QObject* parent = nullptr);
        ~PlayerListModel() override = default;

        PlayerListModel(const PlayerListModel&) = delete;
        PlayerListModel& operator=(const PlayerListModel&) = delete;
        PlayerListModel(PlayerListModel&&) = delete;
        PlayerListModel& operator=(PlayerListModel&&) = delete;

        [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        
        void setPagination(int start, int max);
        [[nodiscard]] size_t totalPlayers() const noexcept;
        [[nodiscard]] int filteredPlayerCount() const noexcept;

    public slots:
        void setFilter(const QString& filter);
        void setPositionFilter(const QString& position);
        void sort(int column, Qt::SortOrder order) override;

    private:
        void applyFilters();
        [[nodiscard]] QVariant getDisplayData(const QModelIndex& index) const;
        [[nodiscard]] bool matchesFilter(const std::pair<int, Player>& player) const;
        [[nodiscard]] bool matchesPosition(const std::pair<int, Player>& player) const;

        int m_startIndex{0};
        int m_maxPlayers{20};
        std::vector<std::pair<int, Player>> m_allPlayers;
        std::vector<std::pair<int, Player>> m_filteredPlayers;
        QString m_currentFilter;
        QString m_currentPosition;
};

#endif

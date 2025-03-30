#ifndef PLAYERSELECT_MODEL_H
#define PLAYERSELECT_MODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <vector>
#include <unordered_set>
#include <span>
#include <optional>
#include "services/TeamManager.h"

class PlayerSelectModel final : public QAbstractTableModel {
    Q_OBJECT

    public:
        explicit PlayerSelectModel(std::span<const std::pair<int, Player>> players, QObject* parent = nullptr);
        ~PlayerSelectModel() override = default;

        PlayerSelectModel(const PlayerSelectModel&) = delete;
        PlayerSelectModel& operator=(const PlayerSelectModel&) = delete;
        PlayerSelectModel(PlayerSelectModel&&) = delete;
        PlayerSelectModel& operator=(PlayerSelectModel&&) = delete;

        [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
        
        void setFilter(const QString& filter);
        void setPositionFilter(const QString& position);
        void setPagination(int start, int max);
        [[nodiscard]] int filteredPlayerCount() const noexcept;
        
        void togglePlayerSelection(const QModelIndex& index);
        void selectPlayer(int playerId);
        void deselectPlayer(int playerId);
        [[nodiscard]] bool isPlayerSelected(int playerId) const noexcept;
        [[nodiscard]] std::vector<Player> getSelectedPlayers() const;
        [[nodiscard]] int getSelectedCount() const noexcept;
        
        void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    private:
        void applyFilters();
        void updatePlayerVisibility(int playerId);
        
        [[nodiscard]] QVariant getDisplayData(const QModelIndex& index) const;
        [[nodiscard]] QVariant getDecorativeData(const QModelIndex& index) const;
        [[nodiscard]] QVariant getAlignmentData(const QModelIndex& index) const;
        [[nodiscard]] QVariant getTooltipData(const QModelIndex& index) const;
        [[nodiscard]] QVariant getFontData(const QModelIndex& index) const;
        
        [[nodiscard]] QVariant getHeaderDisplayData(int section) const;
        [[nodiscard]] QVariant getHeaderTooltipData(int section) const;
        [[nodiscard]] QVariant getHeaderAlignmentData(int section) const;
        
        std::vector<std::pair<int, Player>> m_allPlayers;
        std::vector<std::pair<int, Player>> m_filteredPlayers;
        std::unordered_set<int> m_selectedPlayerIds;
        
        QString m_currentFilter;
        QString m_currentPosition;
        int m_startIndex = 0;
        int m_maxPlayers = 20;
};

#endif

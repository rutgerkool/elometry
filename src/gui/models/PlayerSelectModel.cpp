#include "gui/models/PlayerSelectModel.h"
#include <algorithm>
#include <ranges>
#include <QFont>
#include <QColor>

PlayerSelectModel::PlayerSelectModel(std::span<const std::pair<int, Player>> players, QObject* parent)
    : QAbstractTableModel(parent)
    , m_allPlayers(players.begin(), players.end())
    , m_filteredPlayers(players.begin(), players.end())
{
}

int PlayerSelectModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    const int totalFiltered = filteredPlayerCount();
    return std::min(m_maxPlayers, totalFiltered - m_startIndex);
}

int PlayerSelectModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 6;
}

QVariant PlayerSelectModel::getDisplayData(const QModelIndex& index) const {
    const int actualRow = index.row() + m_startIndex;
    if (actualRow < 0 || static_cast<size_t>(actualRow) >= m_filteredPlayers.size()) {
        return {};
    }

    const auto& player = m_filteredPlayers[actualRow].second;
    
    switch (index.column()) {
        case 0: return {};
        case 1: return m_filteredPlayers[actualRow].first;
        case 2: return QString::fromStdString(player.name);
        case 3: return player.rating;
        case 4: return QString::fromStdString(player.subPosition);
        case 5: {
            double valueInMillions = player.marketValue / 1000000.0;
            return QString("€%1M").arg(valueInMillions, 0, 'f', 1);
        }
        default: return {};
    }
}

QVariant PlayerSelectModel::getDecorativeData(const QModelIndex& index) const {
    const int actualRow = index.row() + m_startIndex;
    if (actualRow < 0 || static_cast<size_t>(actualRow) >= m_filteredPlayers.size()) {
        return {};
    }

    int playerId = m_filteredPlayers[actualRow].first;
    bool selected = isPlayerSelected(playerId);
    
    if (index.column() == 0 && index.data(Qt::DisplayRole).isNull()) {
        return selected ? Qt::Checked : Qt::Unchecked;
    }
    
    return selected ? QColor(45, 65, 90) : (index.row() % 2 ? QColor(45, 45, 45) : QColor(53, 53, 53));
}

QVariant PlayerSelectModel::getAlignmentData(const QModelIndex& index) const {
    switch (index.column()) {
        case 0: return static_cast<int>(Qt::AlignCenter);
        case 3:
        case 5: return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        default: return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }
}

QVariant PlayerSelectModel::getTooltipData(const QModelIndex& index) const {
    const int actualRow = index.row() + m_startIndex;
    if (actualRow < 0 || static_cast<size_t>(actualRow) >= m_filteredPlayers.size()) {
        return {};
    }

    int playerId = m_filteredPlayers[actualRow].first;
    bool selected = isPlayerSelected(playerId);
    const auto& player = m_filteredPlayers[actualRow].second;
    
    switch (index.column()) {
        case 0: return selected ? tr("Click to unselect") : tr("Click to select");
        case 2: return QString::fromStdString(player.name);
        case 5: return QString("€%L1").arg(player.marketValue);
        default: return {};
    }
}

QVariant PlayerSelectModel::getFontData(const QModelIndex& index) const {
    if (index.column() == 2) {
        QFont font;
        font.setBold(true);
        return font;
    }
    return {};
}

QVariant PlayerSelectModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }
    
    const int actualRow = index.row() + m_startIndex;
    if (actualRow < 0 || static_cast<size_t>(actualRow) >= m_filteredPlayers.size()) {
        return {};
    }

    const int playerId = m_filteredPlayers[actualRow].first;
    const bool selected = isPlayerSelected(playerId);

    switch (role) {
        case Qt::DisplayRole:
            return getDisplayData(index);
        case Qt::CheckStateRole:
            return (index.column() == 0) ? (selected ? Qt::Checked : Qt::Unchecked) : QVariant{};
        case Qt::BackgroundRole:
            return selected ? QColor(45, 65, 90) : ((index.row() % 2) ? QColor(45, 45, 45) : QColor(53, 53, 53));
        case Qt::ForegroundRole:
            return selected ? QColor(220, 220, 220) : QColor(210, 210, 210);
        case Qt::UserRole:
            return playerId;
        case Qt::TextAlignmentRole:
            return getAlignmentData(index);
        case Qt::ToolTipRole:
            return getTooltipData(index);
        case Qt::FontRole:
            return getFontData(index);
        default:
            return {};
    }
}

QVariant PlayerSelectModel::getHeaderDisplayData(int section) const {
    switch (section) {
        case 0: return "";
        case 1: return tr("ID");
        case 2: return tr("Name");
        case 3: return tr("Rating");
        case 4: return tr("Position");
        case 5: return tr("Market Value");
        default: return {};
    }
}

QVariant PlayerSelectModel::getHeaderTooltipData(int section) const {
    switch (section) {
        case 0: return tr("Select/Unselect");
        case 1: return tr("Player ID");
        case 2: return tr("Player Name");
        case 3: return tr("Player Rating");
        case 4: return tr("Player Position");
        case 5: return tr("Player Market Value");
        default: return {};
    }
}

QVariant PlayerSelectModel::getHeaderAlignmentData(int section) const {
    switch (section) {
        case 0: return static_cast<int>(Qt::AlignCenter);
        case 3:
        case 5: return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        default: return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }
}

QVariant PlayerSelectModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal) {
        return {};
    }
    
    switch (role) {
        case Qt::DisplayRole:
            return getHeaderDisplayData(section);
        case Qt::ForegroundRole:
            return QColor(220, 220, 220);
        case Qt::TextAlignmentRole:
            return getHeaderAlignmentData(section);
        case Qt::ToolTipRole:
            return getHeaderTooltipData(section);
        default:
            return {};
    }
}

bool PlayerSelectModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.column() != 0 || role != Qt::CheckStateRole) {
        return false;
    }
    
    const size_t actualRow = index.row() + m_startIndex;
    if (actualRow >= m_filteredPlayers.size()) {
        return false;
    }
    
    const int playerId = m_filteredPlayers[actualRow].first;
    
    if (value == Qt::Checked) {
        m_selectedPlayerIds.insert(playerId);
    } else {
        m_selectedPlayerIds.erase(playerId);
    }
    
    emit dataChanged(this->index(index.row(), 0), this->index(index.row(), columnCount() - 1));
    return true;
}

Qt::ItemFlags PlayerSelectModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    if (index.column() == 0) {
        itemFlags |= Qt::ItemIsUserCheckable;
    }
    
    return itemFlags;
}

void PlayerSelectModel::applyFilters() {
    m_filteredPlayers = m_allPlayers;
    
    auto shouldRemove = [this](const auto& player) {
        const bool matchesNameFilter = m_currentFilter.isEmpty() || 
                                      QString::fromStdString(player.second.name)
                                          .contains(m_currentFilter, Qt::CaseInsensitive);
                                          
        const bool matchesPositionFilter = m_currentPosition.isEmpty() || 
                                          player.second.subPosition == m_currentPosition.toStdString() || 
                                          player.second.position == m_currentPosition.toStdString();
                                          
        return !matchesNameFilter || !matchesPositionFilter;
    };
    
    m_filteredPlayers.erase(
        std::remove_if(m_filteredPlayers.begin(), m_filteredPlayers.end(), shouldRemove),
        m_filteredPlayers.end()
    );
}

void PlayerSelectModel::setFilter(const QString& filter) {
    beginResetModel();
    m_currentFilter = filter;
    applyFilters();
    m_startIndex = 0;
    endResetModel();
}

void PlayerSelectModel::setPositionFilter(const QString& position) {
    beginResetModel();
    m_currentPosition = position;
    applyFilters();
    m_startIndex = 0;
    endResetModel();
}

void PlayerSelectModel::setPagination(int start, int max) {
    beginResetModel();
    m_startIndex = start;
    m_maxPlayers = max;
    endResetModel();
}

int PlayerSelectModel::filteredPlayerCount() const noexcept {
    return static_cast<int>(m_filteredPlayers.size());
}

void PlayerSelectModel::togglePlayerSelection(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    
    const int actualRow = index.row() + m_startIndex;
    if (static_cast<size_t>(actualRow) >= m_filteredPlayers.size()) {
        return;
    }
    
    const int playerId = m_filteredPlayers[actualRow].first;
    
    if (isPlayerSelected(playerId)) {
        m_selectedPlayerIds.erase(playerId);
    } else {
        m_selectedPlayerIds.insert(playerId);
    }
    
    emit dataChanged(this->index(index.row(), 0), this->index(index.row(), columnCount() - 1));
}

void PlayerSelectModel::updatePlayerVisibility(int playerId) {
    for (size_t i = 0; i < m_filteredPlayers.size(); ++i) {
        if (m_filteredPlayers[i].first == playerId) {
            const int visibleRow = static_cast<int>(i) - m_startIndex;
            if (visibleRow >= 0 && visibleRow < m_maxPlayers) {
                emit dataChanged(this->index(visibleRow, 0), 
                               this->index(visibleRow, columnCount() - 1));
            }
            break;
        }
    }
}

void PlayerSelectModel::selectPlayer(int playerId) {
    m_selectedPlayerIds.insert(playerId);
    updatePlayerVisibility(playerId);
}

void PlayerSelectModel::deselectPlayer(int playerId) {
    m_selectedPlayerIds.erase(playerId);
    updatePlayerVisibility(playerId);
}

bool PlayerSelectModel::isPlayerSelected(int playerId) const noexcept {
    return m_selectedPlayerIds.contains(playerId);
}

std::vector<Player> PlayerSelectModel::getSelectedPlayers() const {
    std::vector<Player> selectedPlayers;
    selectedPlayers.reserve(m_selectedPlayerIds.size());
    
    for (const auto& [id, player] : m_allPlayers) {
        if (isPlayerSelected(id)) {
            selectedPlayers.push_back(player);
        }
    }
    
    return selectedPlayers;
}

int PlayerSelectModel::getSelectedCount() const noexcept {
    return static_cast<int>(m_selectedPlayerIds.size());
}

void PlayerSelectModel::sort(int column, Qt::SortOrder order) {
    beginResetModel();

    auto comparator = [column, order, this](const auto& a, const auto& b) {
        if (column == 0) {
            const bool aSelected = isPlayerSelected(a.first);
            const bool bSelected = isPlayerSelected(b.first);
            return order == Qt::AscendingOrder ? aSelected < bSelected : aSelected > bSelected;
        }
        
        switch (column) {
            case 1: return order == Qt::AscendingOrder ? a.first < b.first : a.first > b.first;
            case 2: return order == Qt::AscendingOrder ? a.second.name < b.second.name : a.second.name > b.second.name;
            case 3: return order == Qt::AscendingOrder ? a.second.rating < b.second.rating : a.second.rating > b.second.rating;
            case 4: return order == Qt::AscendingOrder ? a.second.subPosition < b.second.subPosition : a.second.subPosition > b.second.subPosition;
            case 5: return order == Qt::AscendingOrder ? a.second.marketValue < b.second.marketValue : a.second.marketValue > b.second.marketValue;
            default: return false;
        }
    };
    
    std::sort(m_filteredPlayers.begin(), m_filteredPlayers.end(), comparator);
    m_startIndex = 0;
    endResetModel();
}

#ifndef PLAYERSEARCHMODEL_H
#define PLAYERSEARCHMODEL_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QTableView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QScrollBar>
#include <QTimer>
#include "services/TeamManager.h"
#include <set>

class PlayerSelectModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit PlayerSelectModel(const std::vector<std::pair<int, Player>>& players, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    void setFilter(const QString& filter);
    void setPositionFilter(const QString& position);
    void setPagination(int start, int max);
    int filteredPlayerCount() const;
    
    void togglePlayerSelection(const QModelIndex &index);
    void selectPlayer(int playerId);
    void deselectPlayer(int playerId);
    bool isPlayerSelected(int playerId) const;
    std::vector<Player> getSelectedPlayers() const;
    int getSelectedCount() const;
    
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    std::vector<std::pair<int, Player>> allPlayers;
    std::vector<std::pair<int, Player>> filteredPlayers;
    std::set<int> selectedPlayerIds;
    
    QString currentFilter;
    QString currentPosition;
    int startIndex = 0;
    int maxPlayers = 20;
};

#endif

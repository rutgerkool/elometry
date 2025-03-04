#ifndef PLAYERSELECTDIALOG_H
#define PLAYERSELECTDIALOG_H

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
#include "gui/models/PlayerListModel.h"
#include "gui/models/PlayerSelectModel.h"
#include <set>

class PlayerSelectDialog : public QDialog {
    Q_OBJECT

public:
    explicit PlayerSelectDialog(TeamManager& teamManager, Team* currentTeam = nullptr, QWidget *parent = nullptr);
    std::vector<Player> getSelectedPlayers() const;

private slots:
    void searchPlayers();
    void positionFilterChanged();
    void clearFilters();
    void toggleSelection();
    void updateSelectionInfo();
    void sortPlayerList(int column, Qt::SortOrder order);
    void checkScrollPosition();
    void loadMorePlayersIfNeeded();

private:
    TeamManager& teamManager;
    PlayerSelectModel* playersModel;
    Team* currentTeam;

    QTableView* playersTable;
    QLineEdit* searchInput;
    QComboBox* positionFilter;
    QPushButton* clearButton;
    QDialogButtonBox* buttonBox;
    QLabel* selectionInfoLabel;
    QLabel* titleLabel;
    QLabel* instructionLabel;
    QLabel* loadingIndicator;

    int currentOffset = 0;
    const int pageSize = 20;
    bool isLoading = false;

    void setupUi();
    void setupConnections();
    void initializePositionFilter();
    void updatePlayerList();
};

#endif

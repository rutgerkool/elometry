#ifndef LINEUPVIEW_H
#define LINEUPVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtGui/QStandardItemModel>
#include <QtCore/QMap>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QApplication>
#include <QEnterEvent>
#include <set>
#include "services/TeamManager.h"
#include "gui/components/LineupPitchView.h"
#include "gui/components/LineupCreationDialog.h"
#include "gui/components/DraggableListWidget.h"

class LineupView : public QWidget {
    Q_OBJECT

    public:
        explicit LineupView(TeamManager& teamManager, Team* currentTeam, QWidget *parent = nullptr);
        void setTeam(Team* team);

    signals:
        void lineupSaved();
        void playerClicked(int playerId);

    private slots:
        void createNewLineup();
        void loadSelectedLineup(int index);
        void saveCurrentLineup(bool forceSave = false);
        void handleDragDropPlayer(int playerId, const QString& fromPosition, const QString& toPosition);
        void updatePlayerLists();

    private:
        void setupUi();
        void setupControlsLayout(QVBoxLayout* mainLayout);
        void setupLineupContent();
        void setupBenchAndReservesGroups(QVBoxLayout* playersListLayout);
        void setupPitchLayout(QHBoxLayout* contentLayout, QVBoxLayout* playersListLayout);
        void setupConnections();
        void setupAdditionalConnections();
        void connectDraggableListWidgets();
        void loadLineups();
        void populateLineupComboBox(const std::vector<Lineup>& lineups);
        void populatePlayerLists();
        void collectStartingPlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds);
        void collectBenchPlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds);
        void collectReservePlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds);
        void addRemainingPlayersToReserveList(const QSet<int>& usedPlayerIds);
        void updateLineupVisibility(bool visible);
        QListWidgetItem* createPlayerItem(const Player& player);
        Player* findPlayerById(int playerId);
        bool eventFilter(QObject* watched, QEvent* event);
        QMap<int, QPixmap> playerImageCache;
        void loadPlayerImage(int playerId, const QString& imageUrl, const QString& position);
        void handleImageLoaded(QNetworkReply* reply, int playerId, const QString& position);
        void loadTeamPlayerImages();
        void loadActiveLineup();
        void populateFieldPositions(const std::set<int>& teamPlayerIds);
        void selectNewLineupInComboBox(int lineupId);
        void handleFieldToListDrop(int playerId, const QString& fromPosition, const QString& toPosition);
        void handleListToListDrop(int playerId, const QString& fromPosition, const QString& toPosition);
        void handleListToFieldDrop(int playerId, const QString& fromPosition, const QString& toPosition, int existingPlayerId);
        void handleExistingPlayerMove(int existingPlayerId, const QString& fromPosition);
        void movePlayerBetweenLists(int playerId, const QString& fromPosition, const QString& toPosition);
        void processDropEvent(QDropEvent* dropEvent, QObject* watched);
        void handleFieldToListDropInEvent(int playerId, const QString& fromPosition, const QString& toPosition);
        void handleListToListDropInEvent(int playerId, const QString& fromPosition, const QString& toPosition);
        void clearAndReloadLineup(const Lineup& lineup);
        void setPlayerInPitchView(int playerId, const QString& position);
        void collectPlayerPositionsFromPitch();
        void collectPlayerPositionsFromBench();
        void collectPlayerPositionsFromReserves();
        void addRemainingPlayersToReserves();
        bool isPlayerInListAlready(int playerId, QListWidget* targetList);
        QPixmap getPlayerImage(int playerId, const QString& position);
        void removePlayerFromList(int playerId, QListWidget* sourceList);

        TeamManager& teamManager;
        Team* currentTeam;
        Lineup currentLineup;
        QMap<int, Formation> formations;

        QComboBox* existingLineupsComboBox;
        QPushButton* newLineupButton;
        QPushButton* deleteLineupButton;
        
        QLabel* lineupNotSelectedLabel;
        QWidget* lineupContentWidget;
        
        LineupPitchView* pitchView;
        QListWidget* reservesList;
        QListWidget* benchList;
        
        QScrollArea* lineupScrollArea;
        QLabel* instructionsLabel;
};

#endif

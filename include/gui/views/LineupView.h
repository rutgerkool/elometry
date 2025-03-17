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
#include <QtWidgets/QGroupBox>
#include <QtGui/QStandardItemModel>
#include <QtCore/QMap>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QApplication>
#include <QEnterEvent>
#include <QtWidgets/QStackedWidget>
#include <set>
#include <map>
#include "services/TeamManager.h"
#include "gui/views/LineupPitchView.h"
#include "gui/components/dialogs/LineupCreationDialog.h"
#include "gui/components/widgets/DraggableListWidget.h"

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

    protected:
        virtual void resizeEvent(QResizeEvent* event) override;

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
        void populatePlayerLists();
        void collectStartingPlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds);
        void collectBenchPlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds);
        void collectReservePlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds);
        void addRemainingPlayersToReserveList(const QSet<int>& usedPlayerIds);
        void updateLineupVisibility(bool visible);
        bool eventFilter(QObject* watched, QEvent* event);
        void loadPlayerImage(int playerId, const QString& imageUrl, const QString& position);
        void handleImageLoaded(QNetworkReply* reply, int playerId, const QString& position);
        void loadTeamPlayerImages();
        void loadActiveLineup();
        void populateFieldPositions(const std::set<int>& teamPlayerIds);
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
        void removePlayerFromList(int playerId, QListWidget* sourceList);
        void addRemainingPlayersToReserves();
        bool isPlayerInListAlready(int playerId, QListWidget* targetList);
        QListWidgetItem* createPlayerItem(const Player& player);
        Player* findPlayerById(int playerId);
        QPixmap getPlayerImage(int playerId, const QString& position);
        QWidget* createPlaceholderWidget(QWidget* parent);
        
        void setupLineupRatingDisplay();
        void updateLineupRatingDisplay();
        double calculateInitialLineupRating();
        void handleDeleteLineupAction();
        void setupListWidget(QListWidget* listWidget);
        void connectListWidgetItems(QListWidget* listWidget);
        QString getPlayerImageUrl(const Player* player);
        bool isPlayerInTeam(int playerId, const std::set<int>& teamPlayerIds);
        void updateRatingLabel(double averageRating, double ratingDiff);
        void createAndSetupNewLineup(int formationId, const QString& lineupName);
        void setupPlaceholderLabels(QVBoxLayout* layout, QWidget* parent);
        void setupLineupControls(QGridLayout* controlsLayout);
        void setupStackedWidget();
        QGroupBox* createBenchGroup();
        QGroupBox* createReservesGroup();
        void setupInstructionsLabel();
        void processDropEventData(QDropEvent* dropEvent, QObject* watched, const QString& mimeText);
        void handleDropPositions(QDropEvent* dropEvent, int playerId, const QString& fromPosition, const QString& toPosition);
        bool confirmLineupDeletion();
        void setupPitchViewConnections();
        void setupListWidgetConnections();
        void addLineupToComboBox(const Lineup& lineup);
        QString createLineupDisplayName(int lineupId, const QString& lineupName, const QString& formationName);
        void highlightActiveLineup();
        void loadActiveTeamLineup();
        void setActiveLineup(const Lineup& activeLineup);
        std::set<int> getTeamPlayerIds();
        void loadPlayerImageFromUrl(const Player& player);
        QListWidget* getListWidgetByPosition(const QString& position);
        void removePlayerFromSourceList(int playerId, QListWidget* sourceList);
        void collectAllPlayerPositions();
        void addPlayerToPositions(int playerId, PositionType posType, const QString& fieldPos, int order);
        void collectPlayersFromList(QListWidget* listWidget, PositionType posType);
        void processPlayerDragDrop(int playerId, const QString& fromPosition, const QString& toPosition);
        int findExistingPlayerAtPosition(const QString& position);
        void addPlayerToList(int playerId, QListWidget* targetList);
        bool isFromField(const QString& position);
        bool isListPosition(const QString& position);
        QPixmap loadPlayerImageFromSource(int playerId, const QString& position);
        void moveExistingPlayerToField(int existingPlayerId, Player* existingPlayer, const QString& position);
        void moveExistingPlayerToList(Player* player, const QString& targetPosition);
        void addPlayerToListAndTrack(int playerId, QListWidget* list, QSet<int>& usedPlayerIds);
        void processLoadedImage(QNetworkReply* reply, int playerId, const QString& position);
        void setupRatingLayout(QVBoxLayout* ratingLayout);
        void resetRatingDisplay();
        double calculateCurrentLineupRating();

        TeamManager& teamManager;
        Team* currentTeam;
        Lineup currentLineup;
        QMap<int, Formation> formations;
        QMap<int, QPixmap> playerImageCache;

        QComboBox* existingLineupsComboBox;
        QPushButton* newLineupButton;
        QPushButton* deleteLineupButton;
        
        QWidget* lineupContentWidget;
        
        LineupPitchView* pitchView;
        QListWidget* reservesList;
        QListWidget* benchList;
        
        QScrollArea* lineupScrollArea;
        QLabel* instructionsLabel;
        
        QLabel* lineupRatingLabel;
        QWidget* lineupRatingWidget;
        double initialLineupRating;
        bool hasInitialLineupRating;

        std::map<std::pair<int, int>, double> initialLineupRatings;
        
        QStackedWidget* lineupStackedWidget;
};

#endif

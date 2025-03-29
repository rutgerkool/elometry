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
#include <span>
#include <optional>
#include "services/TeamManager.h"
#include "gui/views/LineupPitchView.h"
#include "gui/components/dialogs/LineupCreationDialog.h"
#include "gui/components/widgets/DraggableListWidget.h"

class LineupView final : public QWidget {
    Q_OBJECT

    public:
        explicit LineupView(TeamManager& teamManager, Team* currentTeam = nullptr, QWidget* parent = nullptr);
        ~LineupView() override;
        
        LineupView(const LineupView&) = delete;
        LineupView& operator=(const LineupView&) = delete;
        LineupView(LineupView&&) = delete;
        LineupView& operator=(LineupView&&) = delete;
        
        void setTeam(Team* team);

    signals:
        void lineupSaved();
        void playerClicked(int playerId);

    private slots:
        void createNewLineup();
        void loadSelectedLineup(int index);
        void saveCurrentLineup();
        void handleDragDropPlayer(int playerId, const QString& fromPosition, const QString& toPosition);
        void updatePlayerLists();

    protected:
        void resizeEvent(QResizeEvent* event) override;

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
        void collectStartingPlayers(std::span<const Player> teamPlayers, QSet<int>& usedPlayerIds);
        void collectBenchPlayers(std::span<const Player> teamPlayers, QSet<int>& usedPlayerIds);
        void collectReservePlayers(std::span<const Player> teamPlayers, QSet<int>& usedPlayerIds);
        void addRemainingPlayersToReserveList(const QSet<int>& usedPlayerIds);
        void updateLineupVisibility(bool visible);
        [[nodiscard]] bool eventFilter(QObject* watched, QEvent* event) override;
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
        [[nodiscard]] bool isPlayerInListAlready(int playerId, QListWidget* targetList) const;
        [[nodiscard]] QListWidgetItem* createPlayerItem(const Player& player) const;
        [[nodiscard]] Player* findPlayerById(int playerId) const;
        [[nodiscard]] QPixmap getPlayerImage(int playerId, const QString& position);
        [[nodiscard]] QWidget* createPlaceholderWidget(QWidget* parent);
        
        void setupLineupRatingDisplay();
        void updateLineupRatingDisplay();
        [[nodiscard]] double calculateInitialLineupRating() const;
        void handleDeleteLineupAction();
        void setupListWidget(QListWidget* listWidget);
        void connectListWidgetItems(QListWidget* listWidget);
        [[nodiscard]] QString getPlayerImageUrl(const Player* player) const;
        [[nodiscard]] bool isPlayerInTeam(int playerId, const std::set<int>& teamPlayerIds) const;
        void updateRatingLabel(double averageRating, double ratingDiff);
        void createAndSetupNewLineup(int formationId, const QString& lineupName);
        void setupPlaceholderLabels(QVBoxLayout* layout, QWidget* parent);
        void setupLineupControls(QGridLayout* controlsLayout);
        void setupStackedWidget();
        [[nodiscard]] QGroupBox* createBenchGroup();
        [[nodiscard]] QGroupBox* createReservesGroup();
        void setupInstructionsLabel();
        void processDropEventData(QDropEvent* dropEvent, QObject* watched, const QString& mimeText);
        void handleDropPositions(QDropEvent* dropEvent, int playerId, const QString& fromPosition, const QString& toPosition);
        [[nodiscard]] bool confirmLineupDeletion();
        void setupPitchViewConnections();
        void setupListWidgetConnections();
        void addLineupToComboBox(const Lineup& lineup);
        [[nodiscard]] QString createLineupDisplayName(int lineupId, const QString& lineupName, const QString& formationName) const;
        void highlightActiveLineup();
        void loadActiveTeamLineup();
        void setActiveLineup(const Lineup& activeLineup);
        [[nodiscard]] std::set<int> getTeamPlayerIds() const;
        void loadPlayerImageFromUrl(const Player& player);
        [[nodiscard]] QListWidget* getListWidgetByPosition(const QString& position) const;
        void removePlayerFromSourceList(int playerId, QListWidget* sourceList);
        void collectAllPlayerPositions();
        void addPlayerToPositions(int playerId, PositionType posType, const QString& fieldPos, int order);
        void collectPlayersFromList(QListWidget* listWidget, PositionType posType);
        void processPlayerDragDrop(int playerId, const QString& fromPosition, const QString& toPosition);
        [[nodiscard]] int findExistingPlayerAtPosition(const QString& position) const;
        void addPlayerToList(int playerId, QListWidget* targetList);
        [[nodiscard]] bool isFromField(const QString& position) const;
        [[nodiscard]] bool isListPosition(const QString& position) const;
        [[nodiscard]] QPixmap loadPlayerImageFromSource(int playerId, const QString& position);
        void moveExistingPlayerToField(int existingPlayerId, Player* existingPlayer, const QString& position);
        void moveExistingPlayerToList(Player* player, const QString& targetPosition);
        void addPlayerToListAndTrack(int playerId, QListWidget* list, QSet<int>& usedPlayerIds);
        void processLoadedImage(QNetworkReply* reply, int playerId, const QString& position);
        void setupRatingLayout(QVBoxLayout* ratingLayout);
        void resetRatingDisplay();
        [[nodiscard]] double calculateCurrentLineupRating() const;

        TeamManager& m_teamManager;
        Team* m_currentTeam{nullptr};
        Lineup m_currentLineup;
        QMap<int, Formation> m_formations;
        QMap<int, QPixmap> m_playerImageCache;

        QComboBox* m_existingLineupsComboBox{nullptr};
        QPushButton* m_newLineupButton{nullptr};
        QPushButton* m_deleteLineupButton{nullptr};
        
        QWidget* m_lineupContentWidget{nullptr};
        
        LineupPitchView* m_pitchView{nullptr};
        QListWidget* m_reservesList{nullptr};
        QListWidget* m_benchList{nullptr};
        
        QScrollArea* m_lineupScrollArea{nullptr};
        QLabel* m_instructionsLabel{nullptr};
        
        QLabel* m_lineupRatingLabel{nullptr};
        QWidget* m_lineupRatingWidget{nullptr};
        double m_initialLineupRating{0.0};
        bool m_hasInitialLineupRating{false};

        std::map<std::pair<int, int>, double> m_initialLineupRatings;
        
        QStackedWidget* m_lineupStackedWidget{nullptr};
        
        std::unique_ptr<QNetworkAccessManager> m_networkManager;
};

#endif

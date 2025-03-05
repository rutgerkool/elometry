#ifndef PLAYERLISTVIEW_H
#define PLAYERLISTVIEW_H

#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QtNetwork/QNetworkAccessManager>
#include <QScrollArea>
#include <QMap>
#include <set>

class PlayerListModel;

class PlayerListView : public QWidget {
    Q_OBJECT

    public:
        explicit PlayerListView(RatingManager& ratingManager, TeamManager& teamManager, QWidget *parent = nullptr);

    signals:
        void backToMain(); 

    private slots:
        void filterPlayers();
        void searchPlayers(const QString& text);
        void filterByPosition(const QString& position);
        void animateTable();
        void updatePlayerDetails();
        void showPlayerHistory();
        void fetchPlayerDetailImage(const QString& imageUrl);
        void loadLocalPlayerDetailImage(const QString& imageUrl);
        void animatePlayerDetails();
        void selectPlayerForComparison();
        void compareWithSelectedPlayer();
        void clearComparisonSelection();
        void showPlayerComparison();
        void updateComparisonButtons();
        void addPlayerToTeams();

    private:
        void setupUi();
        QFrame* setupTableSection();
        void configureTableView();
        QScrollArea* setupPlayerDetailsSection();
        QWidget* setupPaginationSection();
        void setupInputSection();
        QHBoxLayout* createInputLayout();
        void createPlayerInfoWidgets();
        void createPlayerActionButtons();
        void setupConnections();
        void setupPaginationConnections();
        void setupTableConnections();
        void setupButtonConnections();
        void setupAnimations();
        void setupTableAnimations();
        void setupPlayerDetailsAnimations();
        void updatePagination();
        bool findPlayerById(int playerId, Player& player);
        std::set<int> getTeamsContainingPlayer(int playerId);
        std::vector<int> getTeamsToAdd(const std::set<int>& initialTeamIds, const std::set<int>& finalTeamIds);
        std::vector<int> getTeamsToRemove(const std::set<int>& initialTeamIds, const std::set<int>& finalTeamIds);
        int processAddPlayerToTeams(const std::vector<int>& teamIds, const Player& player);
        int removePlayerFromTeams(const std::vector<int>& teamIds);
        void showResultMessage(int addedCount, int removedCount);

        int currentPage = 0;
        int totalPages = 1;
        const int playersPerPage = 20;
        int currentPlayerId = -1;
        int comparisonPlayerId = -1;

        QPushButton* prevPageButton;
        QPushButton* nextPageButton;
        QPushButton* backButton;
        QLabel* pageInfoLabel;
        QTableView* tableView;
        QLineEdit* searchBox;
        QComboBox* positionFilter;
        QHBoxLayout* paginationLayout;
        QLabel* totalPagesLabel;

        QWidget* playerDetailsWidget;
        QLabel* playerImage;
        QLabel* playerName;
        QLabel* playerClub;
        QLabel* playerPosition;
        QLabel* playerMarketValue;
        QLabel* playerRating;
        QPushButton* viewHistoryButton;
        QPushButton* selectForCompareButton;
        QPushButton* compareWithSelectedButton;
        QPushButton* clearComparisonButton;
        QPushButton* addToTeamButton;

        RatingManager& ratingManager;
        TeamManager& teamManager;
        PlayerListModel* model;
        QNetworkAccessManager* networkManager;
        QMap<int, QPixmap> playerImageCache;

        QGraphicsOpacityEffect* tableOpacityEffect;
        QPropertyAnimation* tableOpacityAnimation;
        QPropertyAnimation* tableSlideAnimation;
        QParallelAnimationGroup* tableAnimGroup;

        QGraphicsOpacityEffect* playerDetailsOpacityEffect;
        QPropertyAnimation* playerDetailsOpacityAnimation;
        QPropertyAnimation* playerDetailsSlideAnimation;
        QParallelAnimationGroup* playerDetailsAnimGroup;
};

#endif

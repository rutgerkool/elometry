#ifndef PLAYERLISTVIEW_H
#define PLAYERLISTVIEW_H

#include "services/RatingManager.h"
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
#include <QMap>

class PlayerListModel;

class PlayerListView : public QWidget {
    Q_OBJECT

    public:
        explicit PlayerListView(RatingManager& ratingManager, QWidget *parent = nullptr);

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

    private:
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

        RatingManager& ratingManager;
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

        void setupUi();
        void setupConnections();
        void setupAnimations();
        void updatePagination();
        void loadPlayerImage(int playerId, const QString& imageUrl);
        void handleImageResponse(QNetworkReply* reply, int playerId);
        void loadLocalImage(int playerId, const QString& imageUrl);
        void updatePlayerImageInModel(int playerId);
};

#endif

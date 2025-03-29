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
#include <QGraphicsOpacityEffect>
#include <QtNetwork/QNetworkAccessManager>
#include <QScrollArea>
#include <QMap>
#include <set>
#include <memory>
#include <optional>

class PlayerListModel;

class PlayerListView final : public QWidget {
    Q_OBJECT

public:
    explicit PlayerListView(RatingManager& ratingManager, TeamManager& teamManager, QWidget* parent = nullptr);
    ~PlayerListView() override;

    PlayerListView(const PlayerListView&) = delete;
    PlayerListView& operator=(const PlayerListView&) = delete;
    PlayerListView(PlayerListView&&) = delete;
    PlayerListView& operator=(PlayerListView&&) = delete;

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
        void navigateToMainView();

    private:
        void setupUi();
        [[nodiscard]] QFrame* setupTableSection();
        void configureTableView();
        [[nodiscard]] QScrollArea* setupPlayerDetailsSection();
        [[nodiscard]] QWidget* setupPaginationSection();
        [[nodiscard]] QWidget* createPageInfoContainer();
        void setupInputSection();
        [[nodiscard]] QHBoxLayout* createInputLayout();
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
        [[nodiscard]] bool findPlayerById(int playerId, Player& player) const;
        [[nodiscard]] std::set<int> getTeamsContainingPlayer(int playerId) const;
        [[nodiscard]] std::vector<int> getTeamsToAdd(const std::set<int>& initialTeamIds, const std::set<int>& finalTeamIds) const;
        [[nodiscard]] std::vector<int> getTeamsToRemove(const std::set<int>& initialTeamIds, const std::set<int>& finalTeamIds) const;
        [[nodiscard]] int processAddPlayerToTeams(const std::vector<int>& teamIds, const Player& player);
        [[nodiscard]] int removePlayerFromTeams(const std::vector<int>& teamIds);
        void showResultMessage(int addedCount, int removedCount);
        void handleTableSelectionChanged();
        void clearPlayerDetails();
        void findAndUpdatePlayerDetails(int playerId);
        void handleComparisonState();
        void handlePlayerImageLoading(const Player& player);
        void setupPlayerDetailsContent();
        void createPlayerName(QVBoxLayout* layout);
        void addPlayerInfoToLayout(QVBoxLayout* layout);
        void addPlayerButtonsToLayout(QVBoxLayout* layout);
        void updatePlayerDetails(const Player& player);

        int m_currentPage{0};
        int m_totalPages{1};
        static constexpr int m_playersPerPage{20};
        int m_currentPlayerId{-1};
        int m_comparisonPlayerId{-1};

        RatingManager& m_ratingManager;
        TeamManager& m_teamManager;
        std::unique_ptr<PlayerListModel> m_model;
        std::unique_ptr<QNetworkAccessManager> m_networkManager;
        std::map<int, QPixmap> m_playerImageCache;

        QPushButton* m_prevPageButton{nullptr};
        QPushButton* m_nextPageButton{nullptr};
        QPushButton* m_backButton{nullptr};
        QLabel* m_pageInfoLabel{nullptr};
        QTableView* m_tableView{nullptr};
        QLineEdit* m_searchBox{nullptr};
        QComboBox* m_positionFilter{nullptr};
        QHBoxLayout* m_paginationLayout{nullptr};
        QLabel* m_totalPagesLabel{nullptr};

        QWidget* m_playerDetailsWidget{nullptr};
        QLabel* m_playerImage{nullptr};
        QLabel* m_playerName{nullptr};
        QLabel* m_playerClub{nullptr};
        QLabel* m_playerPosition{nullptr};
        QLabel* m_playerMarketValue{nullptr};
        QPushButton* m_viewHistoryButton{nullptr};
        QPushButton* m_selectForCompareButton{nullptr};
        QPushButton* m_compareWithSelectedButton{nullptr};
        QPushButton* m_clearComparisonButton{nullptr};
        QPushButton* m_addToTeamButton{nullptr};

        QGraphicsOpacityEffect* m_tableOpacityEffect{nullptr};
        QPropertyAnimation* m_tableOpacityAnimation{nullptr};
        QPropertyAnimation* m_tableSlideAnimation{nullptr};
        QParallelAnimationGroup* m_tableAnimGroup{nullptr};

        QGraphicsOpacityEffect* m_playerDetailsOpacityEffect{nullptr};
        QPropertyAnimation* m_playerDetailsOpacityAnimation{nullptr};
        QPropertyAnimation* m_playerDetailsSlideAnimation{nullptr};
        QParallelAnimationGroup* m_playerDetailsAnimGroup{nullptr};
};

#endif

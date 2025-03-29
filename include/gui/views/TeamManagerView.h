#ifndef TEAMMANAGERVIEW_H
#define TEAMMANAGERVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtGui/QStandardItemModel>
#include <QtNetwork/QNetworkAccessManager>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>
#include "services/TeamManager.h"
#include "gui/models/TeamListModel.h"
#include "gui/views/LineupView.h"

class TeamManagerView final : public QWidget {
    Q_OBJECT

    public:
        explicit TeamManagerView(TeamManager& teamManager, QWidget* parent = nullptr);
        ~TeamManagerView() override;

        TeamManagerView(const TeamManagerView&) = delete;
        TeamManagerView& operator=(const TeamManagerView&) = delete;
        TeamManagerView(TeamManagerView&&) = delete;
        TeamManagerView& operator=(TeamManagerView&&) = delete;

    signals:
        void backToMain();

    private slots:
        void createNewTeam();
        void loadSelectedTeam();
        void autoFillTeam();
        void updateTeamInfo();
        void showClubSelectionDialog();
        void updateBudget(int newBudget);
        void removeSelectedPlayer();
        void navigateBack();
        void updatePlayerDetails();
        void deleteSelectedTeam();
        void editTeamName();
        void animateTeamView();
        void animatePlayerDetails();
        void showPlayerHistory();
        void searchAndAddPlayers();
        void selectPlayerForComparison();
        void compareWithSelectedPlayer();
        void clearComparisonSelection();
        void showPlayerComparison();
        void updateComparisonButtons();
        void hidePlayerDetails();
        void switchToPlayersView();
        void switchToLineupView();
        void showLineupPlayerHistory(int playerId);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        void setupUi();
        void setupTeamRatingDisplay();
        void setupTopButtonLayout(QHBoxLayout* topBarLayout);
        void setupViewToggleButtons(QHBoxLayout* viewToggleLayout);
        void setupLeftContainerWidget();
        void setupCenterContainerWidget();
        void setupRightContainerWidget();
        
        [[nodiscard]] QWidget* setupLeftPanel();
        void setupLeftPanelHeader(QVBoxLayout* layout);
        void setupLeftPanelTeamList(QVBoxLayout* layout);
        void setupLeftPanelButtons(QVBoxLayout* layout);
        
        [[nodiscard]] QWidget* setupCenterPanel();
        void setupCenterPanelHeader(QVBoxLayout* layout);
        void setupCenterPanelTeamTable(QVBoxLayout* layout);
        void setupCenterPanelBudget(QVBoxLayout* layout);
        void setupCenterPanelButtons(QVBoxLayout* layout);
        
        [[nodiscard]] QWidget* setupLineupPanel();
        
        [[nodiscard]] QScrollArea* setupRightPanel();
        void initializePlayerDetailsScrollArea();
        void setupPlayerDetailsWidget();
        void setupPlayerDetailsHeader(QVBoxLayout* layout);
        void setupPlayerDetailsContent(QVBoxLayout* layout);
        
        void createPlayerImageLabel();
        void createPlayerTextLabels();
        [[nodiscard]] QLabel* createInfoLabel(const QString& objectName);
        void createPlayerActionButtons();
        [[nodiscard]] QHBoxLayout* setupImageLayout();
        void updatePanelSizes();
        
        void setupConnections();
        void setupActionConnections();
        void setupSelectionConnections();
        void setupComparisonConnections();
        
        void setupAnimations();
        void setupTeamListAnimations();
        void setupTeamPlayersAnimations();
        void setupPlayerDetailsAnimations();
        
        void enableTeamControls();
        void disableTeamControls();
        
        [[nodiscard]] QStandardItemModel* createPlayerModel();
        [[nodiscard]] QStandardItemModel* initializePlayerModel();
        void populatePlayerModel(QStandardItemModel* playerModel);
        void configureCurrentTeamPlayers(QStandardItemModel* playerModel);
        
        [[nodiscard]] Player* findPlayerById(int playerId);
        [[nodiscard]] int getSelectedPlayerId();
        void updatePlayerInfoLabels(Player* player);
        void updatePlayerImage(Player* player);
        
        void createAndSaveNewTeam(const QString& name);
        void loadTeamFromClub(int clubId);
        void loadAvailableClubs();
        void loadTeamById(int teamId);
        [[nodiscard]] bool validateTeamSelection();
        void processTeamNameUpdate(const QString& newName);
        void refreshTeamListAndSelection(int teamId);
        void deleteTeamAndUpdateUI(int teamId);
        void updateTeamWithSelectedPlayers(std::span<const Player> selectedPlayers);
        
        void processPlayerImage(int playerId, QStandardItem* imageItem, const QString& imageUrl);
        void loadPlayerImage(int playerId, const QString& imageUrl);
        void loadNetworkImage(int playerId, const QString& imageUrl);
        void handleImageResponse(QNetworkReply* reply, int playerId);
        void loadLocalImage(int playerId, const QString& imageUrl);
        void updatePlayerImageInModel(int playerId);
        void fetchPlayerDetailImage(const QString& imageUrl);
        void handlePlayerDetailImageResponse(QNetworkReply* reply);
        void loadLocalPlayerDetailImage(const QString& imageUrl);
        
        [[nodiscard]] bool isValidPlayerSelection(const QModelIndex& index);
        void disableComparisonButtons();
        void updateComparisonButtonsState(int playerId);
        void showNormalSelectionState(int playerId);
        void showSamePlayerSelectionState();
        void showComparisonReadyState();
        void clearPlayerDetails();
        void disablePlayerDetailButtons();
        void resetComparisonState();
        void fadeOutPlayerDetails();
        
        void updateTeamRatingDisplay();
        void updateTeamRatingLabel(double averageRating, double ratingDiff);

        TeamManager& m_teamManager;
        TeamListModel* m_model{nullptr};
        Team* m_currentTeam{nullptr};
        std::unique_ptr<QNetworkAccessManager> m_networkManager;
        std::unordered_map<int, QPixmap> m_playerImageCache;
        int m_comparisonPlayerId{-1};

        QListView* m_teamList{nullptr};
        QTableView* m_currentTeamPlayers{nullptr};
        QScrollArea* m_playerDetailsScrollArea{nullptr};
        QWidget* m_playerDetailsWidget{nullptr};
        QWidget* m_leftWidget{nullptr};
        QWidget* m_lineupContainer{nullptr};
        QStackedWidget* m_mainViewStack{nullptr};
        QStackedWidget* m_centerStackedWidget{nullptr};
        QWidget* m_lineupWidget{nullptr};
        LineupView* m_lineupView{nullptr};

        QPushButton* m_newTeamButton{nullptr};
        QPushButton* m_loadTeamByIdButton{nullptr};
        QPushButton* m_autoFillButton{nullptr};
        QPushButton* m_removePlayerButton{nullptr};
        QPushButton* m_backButton{nullptr};
        QPushButton* m_deleteTeamButton{nullptr};
        QPushButton* m_editTeamNameButton{nullptr};
        QPushButton* m_addPlayersButton{nullptr};
        QPushButton* m_viewHistoryButton{nullptr};
        QPushButton* m_selectForCompareButton{nullptr};
        QPushButton* m_compareWithSelectedButton{nullptr};
        QPushButton* m_clearComparisonButton{nullptr};
        QPushButton* m_viewPlayersButton{nullptr};
        QPushButton* m_viewLineupButton{nullptr};

        QLineEdit* m_teamNameInput{nullptr};
        QSpinBox* m_budgetInput{nullptr};

        QLabel* m_playerImage{nullptr};
        QLabel* m_playerName{nullptr};
        QLabel* m_playerClub{nullptr};
        QLabel* m_playerPosition{nullptr};
        QLabel* m_playerMarketValue{nullptr};
        QLabel* m_playerRating{nullptr};
        QLabel* m_teamRatingLabel{nullptr};
        QWidget* m_teamRatingWidget{nullptr};

        std::vector<std::pair<int, std::string>> m_availableClubs;
        std::unordered_map<int, double> m_teamInitialRatings;
        double m_initialTeamRating{0.0};
        bool m_hasInitialRating{false};

        QGraphicsOpacityEffect* m_teamListOpacityEffect{nullptr};
        QPropertyAnimation* m_teamListOpacityAnimation{nullptr};
        QGraphicsOpacityEffect* m_teamPlayersOpacityEffect{nullptr};
        QPropertyAnimation* m_teamPlayersOpacityAnimation{nullptr};
        QGraphicsOpacityEffect* m_playerDetailsOpacityEffect{nullptr};
        QPropertyAnimation* m_playerDetailsOpacityAnimation{nullptr};
        QPropertyAnimation* m_playerDetailsSlideAnimation{nullptr};
        QParallelAnimationGroup* m_playerDetailsAnimGroup{nullptr};
};

#endif

#ifndef TEAMMANAGERVIEW_H
#define TEAMMANAGERVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollArea>
#include <QtGui/QPixmap>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWidgets/QListView>
#include <QMap>
#include <unordered_map>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include "utils/database/repositories/ClubRepository.h"
#include "services/TeamManager.h"
#include "gui/models/TeamListModel.h"
#include "gui/views/LineupView.h"
#include "gui/components/dialogs/LineupCreationDialog.h"

class TeamManagerView : public QWidget {
    Q_OBJECT

public:
    explicit TeamManagerView(TeamManager& teamManager, QWidget *parent = nullptr);
    ~TeamManagerView() override;

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
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    void setupUi();
    void setupTeamRatingDisplay();
    void setupMainLayout(QVBoxLayout* mainLayout, QWidget* threeColumnContainer);
    void setupTopButtonLayout(QHBoxLayout* topButtonLayout);
    void setupViewToggleButtons(QHBoxLayout* viewToggleLayout);
    void setupThreeColumnContainer(QWidget* threeColumnContainer);
    void setupLeftContainerWidget(QHBoxLayout* layout);
    void setupCenterContainerWidget(QHBoxLayout* layout);
    void setupRightContainerWidget(QHBoxLayout* layout);
    void cleanupLayoutItems();
    
    QWidget* setupLeftPanel();
    void setupLeftPanelHeader(QVBoxLayout* layout);
    void setupLeftPanelTeamList(QVBoxLayout* layout);
    void setupLeftPanelButtons(QVBoxLayout* layout);
    
    QWidget* setupCenterPanel();
    void setupCenterPanelHeader(QVBoxLayout* layout);
    void setupCenterPanelTeamTable(QVBoxLayout* layout);
    void setupCenterPanelBudget(QVBoxLayout* layout);
    void setupCenterPanelButtons(QVBoxLayout* layout);
    
    QWidget* setupLineupPanel();
    
    QScrollArea* setupRightPanel();
    void initializePlayerDetailsScrollArea();
    void setupPlayerDetailsWidget();
    void setupPlayerDetailsHeader(QVBoxLayout* layout);
    void setupPlayerDetailsContent(QVBoxLayout* layout);
    
    void createPlayerImageLabel();
    void createPlayerTextLabels();
    QLabel* createInfoLabel(const QString& objectName);
    
    void createPlayerActionButtons();
    QHBoxLayout* setupImageLayout();
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
    
    QStandardItemModel* createPlayerModel();
    QStandardItemModel* initializePlayerModel();
    void populatePlayerModel(QStandardItemModel* playerModel);
    void configureCurrentTeamPlayers(QStandardItemModel* playerModel);
    
    Player* findPlayerById(int playerId);
    int getSelectedPlayerId();
    void updatePlayerInfoLabels(Player* player);
    void updatePlayerImage(Player* player);
    
    void createAndSaveNewTeam(const QString& name);
    void loadTeamFromClub(int clubId);
    void loadAvailableClubs();
    void loadTeamById(int teamId);
    bool validateTeamSelection();
    void processTeamNameUpdate(const QString& newName);
    void refreshTeamListAndSelection(int teamId);
    void deleteTeamAndUpdateUI(int teamId);
    void updateTeamWithSelectedPlayers(const std::vector<Player>& selectedPlayers);
    
    void processPlayerImage(int playerId, QStandardItem* imageItem, const QString& imageUrl);
    void loadPlayerImage(int playerId, const QString& imageUrl);
    void loadNetworkImage(int playerId, const QString& imageUrl);
    void handleImageResponse(QNetworkReply* reply, int playerId);
    void loadLocalImage(int playerId, const QString& imageUrl);
    void updatePlayerImageInModel(int playerId);
    void fetchPlayerDetailImage(const QString& imageUrl);
    void handlePlayerDetailImageResponse(QNetworkReply* reply);
    void loadLocalPlayerDetailImage(const QString& imageUrl);
    
    bool isValidPlayerSelection(const QModelIndex& index);
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

    TeamManager& teamManager;
    TeamListModel* model;
    Team* currentTeam;
    QNetworkAccessManager* networkManager;
    QMap<int, QPixmap> playerImageCache;
    int comparisonPlayerId = -1;

    QListView* teamList;
    QTableView* currentTeamPlayers;
    QScrollArea* playerDetailsScrollArea;
    QWidget* playerDetailsWidget;
    QWidget* leftWidget;
    QWidget* lineupContainer;
    QStackedWidget* mainViewStack;
    QStackedWidget* centerStackedWidget;
    QWidget* lineupWidget;
    LineupView* lineupView;

    QPushButton* newTeamButton;
    QPushButton* loadTeamByIdButton;
    QPushButton* autoFillButton;
    QPushButton* removePlayerButton;
    QPushButton* backButton;
    QPushButton* deleteTeamButton;
    QPushButton* editTeamNameButton;
    QPushButton* addPlayersButton;
    QPushButton* viewHistoryButton;
    QPushButton* selectForCompareButton;
    QPushButton* compareWithSelectedButton;
    QPushButton* clearComparisonButton;
    QPushButton* viewPlayersButton;
    QPushButton* viewLineupButton;

    QLineEdit* teamNameInput;
    QSpinBox* budgetInput;

    QLabel* playerImage;
    QLabel* playerName;
    QLabel* playerClub;
    QLabel* playerPosition;
    QLabel* playerMarketValue;
    QLabel* playerRating;
    QLabel* teamRatingLabel;
    QWidget* teamRatingWidget;

    std::vector<std::pair<int, std::string>> availableClubs;
    std::unordered_map<int, double> teamInitialRatings;
    double initialTeamRating;
    bool hasInitialRating;

    QGraphicsOpacityEffect* teamListOpacityEffect = nullptr;
    QPropertyAnimation* teamListOpacityAnimation = nullptr;
    QGraphicsOpacityEffect* teamPlayersOpacityEffect = nullptr;
    QPropertyAnimation* teamPlayersOpacityAnimation = nullptr;
    QGraphicsOpacityEffect* playerDetailsOpacityEffect = nullptr;
    QPropertyAnimation* playerDetailsOpacityAnimation = nullptr;
    QPropertyAnimation* playerDetailsSlideAnimation = nullptr;
    QParallelAnimationGroup* playerDetailsAnimGroup = nullptr;
};

#endif

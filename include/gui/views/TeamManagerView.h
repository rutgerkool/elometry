#ifndef TEAMMANAGERVIEW_H
#define TEAMMANAGERVIEW_H

#include <QtWidgets/QWidget>
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
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include "utils/database/repositories/ClubRepository.h"
#include "services/TeamManager.h"
#include "gui/models/TeamListModel.h"

class TeamManagerView : public QWidget {
    Q_OBJECT

public:
    explicit TeamManagerView(TeamManager& teamManager, QWidget *parent = nullptr);

signals:
    void backToMain();

private slots:
    void createNewTeam();
    void loadSelectedTeam();
    void autoFillTeam();
    void updateTeamInfo();
    void loadTeamById();
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

private:
    void setupUi();
    QWidget* setupLeftPanel();
    QWidget* setupCenterPanel();
    QScrollArea* setupRightPanel();
    void createPlayerInfoLabels();
    void createPlayerActionButtons();
    QHBoxLayout* setupImageLayout();
    
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
    Player* findPlayerById(int playerId);
    int getSelectedPlayerId();
    
    void loadPlayerImage(int playerId, const QString& imageUrl);
    void handleImageResponse(QNetworkReply* reply, int playerId);
    void loadLocalImage(int playerId, const QString& imageUrl);
    void updatePlayerImageInModel(int playerId);
    void fetchPlayerDetailImage(const QString& imageUrl);
    void loadLocalPlayerDetailImage(const QString& imageUrl);

    TeamManager& teamManager;
    Team* currentTeam;
    TeamListModel* model;
    QNetworkAccessManager* networkManager;
    QMap<int, QPixmap> playerImageCache;
    int comparisonPlayerId = -1;

    QListView* teamList;
    QTableView* currentTeamPlayers;
    QPushButton* newTeamButton;
    QPushButton* loadTeamByIdButton;
    QPushButton* autoFillButton;
    QPushButton* removePlayerButton;
    QPushButton* backButton;
    QPushButton* deleteTeamButton;
    QPushButton* editTeamNameButton;
    QLineEdit* teamNameInput;
    QSpinBox* budgetInput;
    QPushButton* addPlayersButton;

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

    QWidget* playerDetailsWidget;

    std::vector<std::pair<int, std::string>> availableClubs;

    QGraphicsOpacityEffect* teamListOpacityEffect;
    QPropertyAnimation* teamListOpacityAnimation;
    
    QGraphicsOpacityEffect* teamPlayersOpacityEffect;
    QPropertyAnimation* teamPlayersOpacityAnimation;
    
    QGraphicsOpacityEffect* playerDetailsOpacityEffect;
    QPropertyAnimation* playerDetailsOpacityAnimation;
    QPropertyAnimation* playerDetailsSlideAnimation;
    QParallelAnimationGroup* playerDetailsAnimGroup;
};

#endif

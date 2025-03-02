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
#include "gui/models/Models.h"

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

private:
    TeamManager& teamManager;
    Team* currentTeam;
    TeamListModel* model;
    QNetworkAccessManager* networkManager;
    QMap<int, QPixmap> playerImageCache;

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

    QLabel* playerImage;
    QLabel* playerName;
    QLabel* playerClub;
    QLabel* playerPosition;
    QLabel* playerMarketValue;
    QLabel* playerRating;
    QPushButton* viewHistoryButton;

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

    void setupUi();
    void setupConnections();
    void setupAnimations();
    void loadPlayerImage(int playerId, const QString& imageUrl);
    void handleImageResponse(QNetworkReply* reply, int playerId);
    void loadLocalImage(int playerId, const QString& imageUrl);
    void updatePlayerImageInModel(int playerId);
    void fetchPlayerDetailImage(const QString& imageUrl);
    void loadLocalPlayerDetailImage(const QString& imageUrl);
};

#endif

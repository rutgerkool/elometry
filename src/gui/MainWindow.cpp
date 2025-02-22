#include "gui/MainWindow.h"
#include "gui/PlayerListView.h"
#include "gui/TeamManagerView.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel> 
#include <QtGui/QIcon>
#include <QtCore/QSize> 

MainWindow::MainWindow(RatingManager& rm, TeamManager& tm, QWidget *parent)
    : QMainWindow(parent)
    , ratingManager(rm)
    , teamManager(tm)
    , playerListView(nullptr)
    , teamManagerView(nullptr)
{
    setupUi();
}

MainWindow::~MainWindow() {
    delete playerListView;
    delete teamManagerView;
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(16);
    setCentralWidget(centralWidget);

    QLabel* headerLabel = new QLabel("Elometry", this);
    headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #0078d4; margin-bottom: 16px;");
    layout->addWidget(headerLabel);

    QPushButton* playerListButton = new QPushButton("Player Ratings List", this);
    playerListButton->setIcon(QIcon(":/icons/players.png"));  
    playerListButton->setIconSize(QSize(24, 24));

    QPushButton* teamManagerButton = new QPushButton("Team Manager", this);
    teamManagerButton->setIcon(QIcon(":/icons/team.png"));
    teamManagerButton->setIconSize(QSize(24, 24));

    layout->addWidget(playerListButton);
    layout->addSpacing(8);
    layout->addWidget(teamManagerButton);
    layout->addStretch();  

    connect(playerListButton, &QPushButton::clicked, this, &MainWindow::showPlayerList);
    connect(teamManagerButton, &QPushButton::clicked, this, &MainWindow::showTeamManager);

    setWindowTitle("Elometry");
    setMinimumSize(1024, 768);
}

void MainWindow::showPlayerList() {
    if (!playerListView) {
        playerListView = new PlayerListView(ratingManager, this);
    }
    setCentralWidget(playerListView);
}

void MainWindow::showTeamManager() {
    if (!teamManagerView) {
        teamManagerView = new TeamManagerView(teamManager, this);
    }
    setCentralWidget(teamManagerView);
}

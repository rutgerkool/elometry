#include "gui/views/MainWindow.h"
#include "gui/views/PlayerListView.h"
#include "gui/views/TeamManagerView.h"
#include "gui/views/SettingsView.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel> 
#include <QtGui/QIcon>
#include <QtCore/QSize> 

MainWindow::MainWindow(RatingManager& rm, TeamManager& tm, Database& db, QWidget *parent)
    : QMainWindow(parent)
    , ratingManager(rm)
    , teamManager(tm)
    , database(db)
    , playerListView(nullptr)
    , teamManagerView(nullptr)
    , settingsView(nullptr)
{
    stackedWidget = new QStackedWidget(this);

    mainView = new QWidget(this);
    playerListView = new PlayerListView(ratingManager, this);
    teamManagerView = new TeamManagerView(teamManager, this);
    settingsView = new SettingsView(database, this);

    stackedWidget->addWidget(mainView);
    stackedWidget->addWidget(playerListView);
    stackedWidget->addWidget(teamManagerView);
    stackedWidget->addWidget(settingsView);

    setCentralWidget(stackedWidget);

    setupUi();
    setupConnections();

    stackedWidget->setCurrentWidget(mainView);
}

MainWindow::~MainWindow() {
    delete playerListView;
    delete teamManagerView;
    delete settingsView;
}

void MainWindow::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(mainView);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(16);

    QLabel* headerLabel = new QLabel("Elometry", this);
    headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #0078d4; margin-bottom: 16px;");
    layout->addWidget(headerLabel);

    QPushButton* playerListButton = new QPushButton("Player Ratings List", this);
    playerListButton->setIcon(QIcon(":/static/default.jpg"));  
    playerListButton->setIconSize(QSize(24, 24));

    QPushButton* teamManagerButton = new QPushButton("Team Manager", this);
    teamManagerButton->setIcon(QIcon(":/icons/team.png"));
    teamManagerButton->setIconSize(QSize(24, 24));

    QPushButton* settingsButton = new QPushButton("Settings", this);
    settingsButton->setIcon(QIcon(":/icons/settings.png"));
    settingsButton->setIconSize(QSize(24, 24));

    layout->addWidget(playerListButton);
    layout->addSpacing(8);
    layout->addWidget(teamManagerButton);
    layout->addSpacing(8);
    layout->addWidget(settingsButton);
    layout->addStretch();  

    connect(playerListButton, &QPushButton::clicked, this, &MainWindow::showPlayerList);
    connect(teamManagerButton, &QPushButton::clicked, this, &MainWindow::showTeamManager);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::showSettings);

    setWindowTitle("Elometry");
    setMinimumSize(1024, 768);
}

void MainWindow::setupConnections() {
    connect(playerListView, &PlayerListView::backToMain, this, &MainWindow::showMainView);
    connect(teamManagerView, &TeamManagerView::backToMain, this, &MainWindow::showMainView);
    connect(settingsView, &SettingsView::backToMain, this, &MainWindow::showMainView);
}

void MainWindow::showMainView() {
    stackedWidget->setCurrentWidget(mainView);
}

void MainWindow::showPlayerList() {
    stackedWidget->setCurrentWidget(playerListView);
}

void MainWindow::showTeamManager() {
    stackedWidget->setCurrentWidget(teamManagerView);
}

void MainWindow::showSettings() {
    stackedWidget->setCurrentWidget(settingsView);
}

#include "gui/views/MainWindow.h"
#include "gui/views/PlayerListView.h"
#include "gui/views/TeamManagerView.h"
#include "gui/views/SettingsView.h"
#include "gui/views/LoadingView.h"
#include "gui/components/DataLoader.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel> 
#include <QtGui/QIcon>
#include <QtCore/QSize> 
#include <QTimer>

MainWindow::MainWindow(RatingManager& rm, TeamManager& tm, Database& db, QWidget *parent)
    : QMainWindow(parent)
    , ratingManager(rm)
    , teamManager(tm)
    , database(db)
    , playerListView(nullptr)
    , teamManagerView(nullptr)
    , settingsView(nullptr)
    , loadingView(nullptr)
    , loadingThread(nullptr)
    , appInitialized(false)
{
    stackedWidget = new QStackedWidget(this);

    loadingView = new LoadingView(this);
    
    mainView = new QWidget(this);
    
    stackedWidget->addWidget(loadingView);
    stackedWidget->addWidget(mainView);
    
    setCentralWidget(stackedWidget);
    
    stackedWidget->setCurrentWidget(loadingView);
    
    initializeApp();

    setWindowTitle("Elometry");
    setMinimumSize(1024, 768);
}

MainWindow::~MainWindow() {
    if (playerListView) delete playerListView;
    if (teamManagerView) delete teamManagerView;
    if (settingsView) delete settingsView;
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
}

void MainWindow::setupConnections() {
    connect(playerListView, &PlayerListView::backToMain, this, &MainWindow::showMainView);
    connect(teamManagerView, &TeamManagerView::backToMain, this, &MainWindow::showMainView);
    connect(settingsView, &SettingsView::backToMain, this, &MainWindow::showMainView);
}

void MainWindow::showMainView() {
    if (!appInitialized) {
        playerListView = new PlayerListView(ratingManager, this);
        teamManagerView = new TeamManagerView(teamManager, this);
        settingsView = new SettingsView(database, this);
        
        stackedWidget->addWidget(playerListView);
        stackedWidget->addWidget(teamManagerView);
        stackedWidget->addWidget(settingsView);
        
        setupConnections();
        appInitialized = true;
    }
    
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

void MainWindow::initializeApp() {
    loadingThread = new QThread();
    DataLoader* dataLoader = new DataLoader(ratingManager, teamManager, database);
    dataLoader->moveToThread(loadingThread);
    
    connect(loadingThread, &QThread::started, [dataLoader]() {
        dataLoader->loadData("test.db");
    });
    
    connect(dataLoader, &DataLoader::progressUpdate, this, &MainWindow::onDataLoadProgress);
    connect(dataLoader, &DataLoader::loadingComplete, this, [=]() {
        setupUi();
        
        loadingThread->quit();
        loadingThread->wait();
        dataLoader->deleteLater();
        
        showMainView();
    });
    
    connect(loadingThread, &QThread::finished, loadingThread, &QThread::deleteLater);
    
    loadingThread->start();
}

void MainWindow::onDataLoadProgress(const QString& status, int progress) {
    loadingView->updateStatus(status);
    loadingView->updateProgress(progress);
    
    if (progress >= 100) {
        QTimer::singleShot(500, loadingView, &LoadingView::loadingFinished);
    }
}

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
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>

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
    
    setupAnimations();
    initializeApp();

    setWindowTitle("Elometry");
    setMinimumSize(1024, 768);
}

MainWindow::~MainWindow() {
    if (playerListView) delete playerListView;
    if (teamManagerView) delete teamManagerView;
    if (settingsView) delete settingsView;
}

void MainWindow::setupAnimations() {
    opacityEffect = new QGraphicsOpacityEffect(this);
    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnimation->setDuration(300);
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);
    fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void MainWindow::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(mainView);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(24);

    QLabel* headerLabel = new QLabel("Elometry", this);
    headerLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #0c7bb3; margin-bottom: 24px;");
    layout->addWidget(headerLabel, 0, Qt::AlignCenter);

    QPushButton* playerListButton = new QPushButton("Player Ratings List", this);
    playerListButton->setMinimumHeight(60);

    QPushButton* teamManagerButton = new QPushButton("Team Manager", this);
    teamManagerButton->setMinimumHeight(60);

    QPushButton* settingsButton = new QPushButton("Settings", this);
    settingsButton->setMinimumHeight(60);

    layout->addWidget(playerListButton);
    layout->addSpacing(16);
    layout->addWidget(teamManagerButton);
    layout->addSpacing(16);
    layout->addWidget(settingsButton);
    layout->addStretch();  

    mainView->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);

    connect(playerListButton, &QPushButton::clicked, this, &MainWindow::showPlayerList);
    connect(teamManagerButton, &QPushButton::clicked, this, &MainWindow::showTeamManager);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::showSettings);
}

void MainWindow::setupConnections() {
    connect(playerListView, &PlayerListView::backToMain, this, &MainWindow::showMainView);
    connect(teamManagerView, &TeamManagerView::backToMain, this, &MainWindow::showMainView);
    connect(settingsView, &SettingsView::backToMain, this, &MainWindow::showMainView);
}

void MainWindow::animateViewTransition(QWidget* newWidget) {
    fadeAnimation->setDirection(QPropertyAnimation::Backward);
    connect(fadeAnimation, &QPropertyAnimation::finished, this, [=]() {
        stackedWidget->setCurrentWidget(newWidget);
        fadeAnimation->setDirection(QPropertyAnimation::Forward);
        fadeAnimation->start();
        disconnect(fadeAnimation, &QPropertyAnimation::finished, this, nullptr);
    });
    fadeAnimation->start();
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
    
    animateViewTransition(mainView);
}

void MainWindow::showPlayerList() {
    animateViewTransition(playerListView);
}

void MainWindow::showTeamManager() {
    animateViewTransition(teamManagerView);
}

void MainWindow::showSettings() {
    animateViewTransition(settingsView);
}

void MainWindow::initializeApp() {
    loadingThread = new QThread();
    DataLoader* dataLoader = new DataLoader(ratingManager, teamManager, database);
    dataLoader->moveToThread(loadingThread);
    
    loadingView->updateStatus("Starting");
    loadingView->updateProgress(0);
    
    connect(loadingThread, &QThread::started, [dataLoader]() {
        dataLoader->loadData();
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
    
    QTimer::singleShot(100, [this]() {
        loadingThread->start();
    });
}

void MainWindow::onDataLoadProgress(const QString& status, int progress) {
    loadingView->updateStatus(status);
    loadingView->updateProgress(progress);
    
    if (progress >= 100) {
        QTimer::singleShot(500, loadingView, &LoadingView::markLoadingComplete);
    }
}

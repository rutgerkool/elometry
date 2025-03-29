#include "gui/views/MainWindow.h"
#include "gui/views/PlayerListView.h"
#include "gui/views/TeamManagerView.h"
#include "gui/views/SettingsView.h"
#include "gui/views/LoadingView.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel> 
#include <QtCore/QSize> 
#include <QTimer>
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QScreen>
#include <QApplication>

MainWindow::MainWindow(RatingManager& ratingManager, TeamManager& teamManager, Database& database, QWidget* parent)
    : QMainWindow(parent)
    , m_ratingManager(ratingManager)
    , m_teamManager(teamManager)
    , m_database(database)
    , m_loadingView(std::make_unique<LoadingView>())
    , m_loadingThread(std::make_unique<QThread>())
    , m_viewTransitionEffect(std::make_unique<QGraphicsOpacityEffect>())
    , m_fadeAnimation(nullptr)
{
    setFixedSize(1024, 848);
    
    m_stackedWidget = new QStackedWidget(this);
    m_mainMenuView = new QWidget(this);
    
    m_stackedWidget->addWidget(m_loadingView.get());
    m_stackedWidget->addWidget(m_mainMenuView);
    
    setCentralWidget(m_stackedWidget);
    m_stackedWidget->setCurrentWidget(m_loadingView.get());
    
    m_fadeAnimation = std::make_unique<QPropertyAnimation>(m_viewTransitionEffect.get(), "opacity");
    setupViewTransitions();
    initializeApplication();

    setWindowTitle("Elometry");
}

void MainWindow::setupUserInterface() {
    createMainMenu();
    
    m_mainMenuView->setGraphicsEffect(m_viewTransitionEffect.get());
    m_viewTransitionEffect->setOpacity(1.0);
}

void MainWindow::createMainMenu() {
    auto* layout = new QVBoxLayout(m_mainMenuView);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(24);

    auto* headerLabel = new QLabel("Elometry", m_mainMenuView);
    headerLabel->setObjectName("headerLabel");
    layout->addWidget(headerLabel, 0, Qt::AlignCenter);

    auto* playerListButton = new QPushButton("Player Ratings List", m_mainMenuView);
    playerListButton->setMinimumHeight(60);

    auto* teamManagerButton = new QPushButton("Team Manager", m_mainMenuView);
    teamManagerButton->setMinimumHeight(60);

    auto* settingsButton = new QPushButton("Settings", m_mainMenuView);
    settingsButton->setMinimumHeight(60);

    layout->addWidget(playerListButton);
    layout->addSpacing(16);
    layout->addWidget(teamManagerButton);
    layout->addSpacing(16);
    layout->addWidget(settingsButton);
    layout->addStretch();

    connect(playerListButton, &QPushButton::clicked, this, &MainWindow::navigateToPlayerList);
    connect(teamManagerButton, &QPushButton::clicked, this, &MainWindow::navigateToTeamManager);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::navigateToSettings);
}

void MainWindow::centerWindowOnScreen() {
    const QPoint cursorPos = QCursor::pos();
    QScreen* targetScreen = nullptr;

    for (QScreen* screen : QGuiApplication::screens()) {
        if (screen->geometry().contains(cursorPos)) {
            targetScreen = screen;
            break;
        }
    }
    
    if (!targetScreen) {
        targetScreen = QGuiApplication::primaryScreen();
    }
    
    if (targetScreen) {
        const QRect screenGeometry = targetScreen->availableGeometry();
        move(screenGeometry.center() - rect().center());
    }
}

void MainWindow::showEvent(QShowEvent* event) {
    centerWindowOnScreen();
    QMainWindow::showEvent(event);
}

void MainWindow::setupViewTransitions() {
    m_fadeAnimation->setDuration(200);
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void MainWindow::initializeViews() {
    m_playerListView = std::make_unique<PlayerListView>(m_ratingManager, m_teamManager, this);
    m_teamManagerView = std::make_unique<TeamManagerView>(m_teamManager, this);
    m_settingsView = std::make_unique<SettingsView>(m_database, this);
    
    m_stackedWidget->addWidget(m_playerListView.get());
    m_stackedWidget->addWidget(m_teamManagerView.get());
    m_stackedWidget->addWidget(m_settingsView.get());
    
    connectSignals();
    m_appInitialized = true;
}

void MainWindow::connectSignals() {
    connect(m_playerListView.get(), &PlayerListView::backToMain, this, &MainWindow::navigateToMainMenu);
    connect(m_teamManagerView.get(), &TeamManagerView::backToMain, this, &MainWindow::navigateToMainMenu);
    connect(m_settingsView.get(), &SettingsView::backToMain, this, &MainWindow::navigateToMainMenu);
}

void MainWindow::navigateToPlayerList() {
    QWidget* target = m_playerListView.get();
    m_fadeAnimation->setDirection(QPropertyAnimation::Backward);
    
    connect(m_fadeAnimation.get(), &QPropertyAnimation::finished, this, [this, target]() {
        m_stackedWidget->setCurrentWidget(target);
        m_fadeAnimation->setDirection(QPropertyAnimation::Forward);
        m_fadeAnimation->start();
        disconnect(m_fadeAnimation.get(), &QPropertyAnimation::finished, this, nullptr);
    });
    m_fadeAnimation->start();
}

void MainWindow::navigateToTeamManager() {
    QWidget* target = m_teamManagerView.get();
    m_fadeAnimation->setDirection(QPropertyAnimation::Backward);
    
    connect(m_fadeAnimation.get(), &QPropertyAnimation::finished, this, [this, target]() {
        m_stackedWidget->setCurrentWidget(target);
        m_fadeAnimation->setDirection(QPropertyAnimation::Forward);
        m_fadeAnimation->start();
        disconnect(m_fadeAnimation.get(), &QPropertyAnimation::finished, this, nullptr);
    });
    m_fadeAnimation->start();
}

void MainWindow::navigateToSettings() {
    QWidget* target = m_settingsView.get();
    m_fadeAnimation->setDirection(QPropertyAnimation::Backward);
    
    connect(m_fadeAnimation.get(), &QPropertyAnimation::finished, this, [this, target]() {
        m_stackedWidget->setCurrentWidget(target);
        m_fadeAnimation->setDirection(QPropertyAnimation::Forward);
        m_fadeAnimation->start();
        disconnect(m_fadeAnimation.get(), &QPropertyAnimation::finished, this, nullptr);
    });
    m_fadeAnimation->start();
}

void MainWindow::navigateToMainMenu() {
    QWidget* target = m_mainMenuView;
    m_fadeAnimation->setDirection(QPropertyAnimation::Backward);
    
    connect(m_fadeAnimation.get(), &QPropertyAnimation::finished, this, [this, target]() {
        m_stackedWidget->setCurrentWidget(target);
        m_fadeAnimation->setDirection(QPropertyAnimation::Forward);
        m_fadeAnimation->start();
        disconnect(m_fadeAnimation.get(), &QPropertyAnimation::finished, this, nullptr);
    });
    m_fadeAnimation->start();
}

void MainWindow::initializeApplication() {
    auto* dataLoader = new DataLoader(m_ratingManager, m_teamManager, m_database);
    dataLoader->moveToThread(m_loadingThread.get());
    m_loadingView->updateStatus("Starting");
    m_loadingView->updateProgress(0);
    
    connect(m_loadingThread.get(), &QThread::started, dataLoader, &DataLoader::loadData, 
            Qt::QueuedConnection);
    
    connect(dataLoader, &DataLoader::loadingComplete, this, &MainWindow::transitionToMainView, 
            Qt::QueuedConnection);
    
    connect(dataLoader, &DataLoader::progressUpdate, this, &MainWindow::handleDataLoadProgress, 
            Qt::QueuedConnection);
    
    QTimer::singleShot(100, [this]() {
        m_loadingThread->start();
    });
}

void MainWindow::handleDataLoadProgress(const QString& status, int progress) {
    m_loadingView->updateStatus(status);
    m_loadingView->updateProgress(progress);
    
    if (progress >= 100) {
        QTimer::singleShot(500, m_loadingView.get(), &LoadingView::markLoadingComplete);
    }
}

void MainWindow::transitionToMainView() {
    setupUserInterface();
    
    if (m_loadingThread->isRunning()) {
        m_loadingThread->quit();
        m_loadingThread->wait();
    }
    
    showMainView();
}

void MainWindow::showMainView() {
    if (!m_appInitialized) {
        initializeViews();
    }
    
    navigateToMainMenu();
}

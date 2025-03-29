#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/Database.h"
#include "gui/components/DataLoader.h"
#include "gui/views/PlayerListView.h"
#include "gui/views/TeamManagerView.h"
#include "gui/views/SettingsView.h"
#include "gui/views/LoadingView.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>
#include <QThread>
#include <QShowEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <memory>
#include <optional>

class MainWindow final : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(RatingManager& ratingManager, TeamManager& teamManager, Database& database, QWidget* parent = nullptr);
        ~MainWindow() override = default;
        
        MainWindow(const MainWindow&) = delete;
        MainWindow& operator=(const MainWindow&) = delete;
        MainWindow(MainWindow&&) noexcept = delete;
        MainWindow& operator=(MainWindow&&) noexcept = delete;

    protected:
        void showEvent(QShowEvent* event) override;

    private slots:
        void navigateToPlayerList();
        void navigateToTeamManager();
        void navigateToSettings();
        void navigateToMainMenu();
        void initializeApplication();
        void handleDataLoadProgress(const QString& status, int progress);
        void transitionToMainView();

    private:
        void setupUserInterface();
        void createMainMenu();
        void centerWindowOnScreen();
        void connectSignals();
        void setupViewTransitions();
        void initializeViews();
        void showMainView();

        RatingManager& m_ratingManager;
        TeamManager& m_teamManager;
        Database& m_database;
        
        std::unique_ptr<PlayerListView> m_playerListView;
        std::unique_ptr<TeamManagerView> m_teamManagerView;
        std::unique_ptr<SettingsView> m_settingsView;
        std::unique_ptr<LoadingView> m_loadingView;

        QWidget* m_mainMenuView{nullptr};
        QStackedWidget* m_stackedWidget{nullptr};
        
        std::unique_ptr<QThread> m_loadingThread;
        std::unique_ptr<QGraphicsOpacityEffect> m_viewTransitionEffect;
        std::unique_ptr<QPropertyAnimation> m_fadeAnimation;

        bool m_appInitialized{false};
};

#endif

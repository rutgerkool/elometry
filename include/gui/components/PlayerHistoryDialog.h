#ifndef PLAYERHISTORYDIALOG_H
#define PLAYERHISTORYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "services/RatingManager.h"

#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QChart>
#include <QScatterSeries>

class PlayerHistoryDialog : public QDialog {
    Q_OBJECT

    public:
        PlayerHistoryDialog(RatingManager& ratingManager, int playerId, QWidget* parent = nullptr);
        ~PlayerHistoryDialog();

    private slots:
        void setupHistoryTable();
        void setupHistoryChart();
        void animateContent();

    private:
        void initializeUI();
        void loadPlayerData();
        void setupTitle();
        void setupPlayerInfo();
        void setupChartView();
        void setupTableView();
        void setupAnimations();
        void centerDialog(QWidget* parent);
        void createTable();
        void populateTable();
        void configureChartAppearance();
        void createChartSeries(QLineSeries* series, QScatterSeries* pointSeries);
        void setupChartAxes(QLineSeries* series, QScatterSeries* pointSeries, 
                        double minRating, double maxRating, QDateTimeAxis* axisX, QValueAxis* axisY);

        RatingManager& ratingManager;
        int playerId;
        Player player;
        std::vector<RatingChange> playerHistory;
        std::vector<std::pair<int, double>> ratingProgression;

        QVBoxLayout* mainLayout;
        QLabel* titleLabel;
        QLabel* playerInfoLabel;
        QTableView* historyTable;
        QStandardItemModel* tableModel;
        QChartView* chartView;
        QChart* chart;
        QPushButton* closeButton;

        QGraphicsOpacityEffect* chartOpacityEffect;
        QPropertyAnimation* chartAnimation;
        QGraphicsOpacityEffect* tableOpacityEffect;
        QPropertyAnimation* tableAnimation;
        QParallelAnimationGroup* animationGroup;
};

#endif

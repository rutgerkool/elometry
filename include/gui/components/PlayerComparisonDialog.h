#ifndef PLAYERCOMPARISONDIALOG_H
#define PLAYERCOMPARISONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include "services/RatingManager.h"

#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QChart>
#include <QScatterSeries>

class PlayerComparisonDialog : public QDialog {
    Q_OBJECT

    public:
        PlayerComparisonDialog(RatingManager& ratingManager, int playerId1, int playerId2, QWidget* parent = nullptr);
        ~PlayerComparisonDialog();

    private:
        void initializeUI();
        void loadPlayerData();
        void createPlayerInfoLayouts();
        void createTableLayouts();
        void createButtonLayout();
        void centerDialogOnScreen(QWidget* parent);
        
        void setupComparisonChart();
        void setupPlayerTable(QTableView* table, const std::vector<RatingChange>& playerHistory);
        void setupXAxis(QValueAxis* axis, size_t maxSize);
        void setupYAxis(QValueAxis* axis, double minRating, double maxRating);
        void addSeriesToChart(QLineSeries* series1, QLineSeries* series2, QValueAxis* axisX, QValueAxis* axisY);
        QLineSeries* createPlayerSeries(const std::string& name, const QColor& color);

        RatingManager& ratingManager;
        int playerId1;
        int playerId2;
        Player player1;
        Player player2;
        std::vector<RatingChange> player1History;
        std::vector<RatingChange> player2History;
        std::vector<std::pair<int, double>> rating1Progression;
        std::vector<std::pair<int, double>> rating2Progression;

        QVBoxLayout* mainLayout;
        QChartView* chartView;
        QChart* chart;
        QPushButton* closeButton;
};

#endif

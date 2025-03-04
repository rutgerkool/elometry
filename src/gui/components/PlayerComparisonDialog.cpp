#include "gui/components/PlayerComparisonDialog.h"
#include <QHeaderView>
#include <QDateTime>
#include <QLabel>
#include <QFont>
#include <QScreen>
#include <QGuiApplication>
#include <limits>
#include <QtCharts/QLegendMarker>

PlayerComparisonDialog::PlayerComparisonDialog(RatingManager& rm, int pId1, int pId2, QWidget* parent)
    : QDialog(parent)
    , ratingManager(rm)
    , playerId1(pId1)
    , playerId2(pId2)
{
    setWindowTitle("Player Rating Comparison");
    resize(1024, 768);
    setModal(true);
    
    initializeUI();
    loadPlayerData();
    
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    
    createPlayerInfoLayouts();
    
    setupComparisonChart();
    mainLayout->addWidget(chartView);
    
    createTableLayouts();
    createButtonLayout();
    centerDialogOnScreen(parent);
}

PlayerComparisonDialog::~PlayerComparisonDialog() {}

void PlayerComparisonDialog::initializeUI()
{
    chart = new QChart();
    chartView = new QChartView(chart, this);
    
    chart->setTitle("Rating Progression by Appearance");
    chart->setTitleBrush(QBrush(Qt::white));
    QFont chartTitleFont("Segoe UI", 14);
    chartTitleFont.setBold(true);
    chart->setTitleFont(chartTitleFont);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QBrush(QColor(0x2a, 0x2a, 0x2a)));
    chart->setPlotAreaBackgroundBrush(QBrush(QColor(0x2a, 0x2a, 0x2a)));
    chart->setPlotAreaBackgroundVisible(true);
    
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(250);
}

void PlayerComparisonDialog::loadPlayerData()
{
    auto allPlayers = ratingManager.getSortedRatedPlayers();
    bool player1Found = false;
    bool player2Found = false;
    
    for (const auto& p : allPlayers) {
        if (p.second.playerId == playerId1) {
            player1 = p.second;
            player1Found = true;
        }
        if (p.second.playerId == playerId2) {
            player2 = p.second;
            player2Found = true;
        }
    }
    
    if (!player1Found || !player2Found) {
        close();
        return;
    }
    
    player1History = ratingManager.getPlayerRatingHistory(playerId1, 10);
    player2History = ratingManager.getPlayerRatingHistory(playerId2, 10);
    rating1Progression = ratingManager.getRecentRatingProgression(playerId1, 10);
    rating2Progression = ratingManager.getRecentRatingProgression(playerId2, 10);
}

void PlayerComparisonDialog::createPlayerInfoLayouts()
{
    QHBoxLayout* playerInfoLayout = new QHBoxLayout();
    playerInfoLayout->setSpacing(20);
    
    QVBoxLayout* player1Layout = new QVBoxLayout();
    player1Layout->setSpacing(2);
    
    QLabel* player1NameLabel = new QLabel(QString::fromStdString(player1.name), this);
    player1NameLabel->setStyleSheet("color: #0c7bb3; font-weight: bold;");
    
    QLabel* player1ClubLabel = new QLabel("Club: " + QString::fromStdString(player1.clubName), this);
    player1ClubLabel->setStyleSheet("color: #0c7bb3;");
    
    QLabel* player1PositionLabel = new QLabel("Position: " + QString::fromStdString(player1.position), this);
    player1PositionLabel->setStyleSheet("color: #0c7bb3;");
    
    QLabel* player1RatingLabel = new QLabel("Rating: " + QString::number(player1.rating, 'f', 1), this);
    player1RatingLabel->setStyleSheet("color: #0c7bb3;");
    
    player1Layout->addWidget(player1NameLabel, 0, Qt::AlignLeft);
    player1Layout->addWidget(player1ClubLabel, 0, Qt::AlignLeft);
    player1Layout->addWidget(player1PositionLabel, 0, Qt::AlignLeft);
    player1Layout->addWidget(player1RatingLabel, 0, Qt::AlignLeft);
    
    QVBoxLayout* player2Layout = new QVBoxLayout();
    player2Layout->setSpacing(2);
    
    QLabel* player2NameLabel = new QLabel(QString::fromStdString(player2.name), this);
    player2NameLabel->setStyleSheet("color: #2ea043; font-weight: bold;");
    
    QLabel* player2ClubLabel = new QLabel("Club: " + QString::fromStdString(player2.clubName), this);
    player2ClubLabel->setStyleSheet("color: #2ea043;");
    
    QLabel* player2PositionLabel = new QLabel("Position: " + QString::fromStdString(player2.position), this);
    player2PositionLabel->setStyleSheet("color: #2ea043;");
    
    QLabel* player2RatingLabel = new QLabel("Rating: " + QString::number(player2.rating, 'f', 1), this);
    player2RatingLabel->setStyleSheet("color: #2ea043;");
    
    player2Layout->addWidget(player2NameLabel, 0, Qt::AlignRight);
    player2Layout->addWidget(player2ClubLabel, 0, Qt::AlignRight);
    player2Layout->addWidget(player2PositionLabel, 0, Qt::AlignRight);
    player2Layout->addWidget(player2RatingLabel, 0, Qt::AlignRight);
    
    QWidget* player1Container = new QWidget();
    player1Container->setLayout(player1Layout);
    
    QWidget* player2Container = new QWidget();
    player2Container->setLayout(player2Layout);
    
    playerInfoLayout->addWidget(player1Container, 1);
    playerInfoLayout->addWidget(player2Container, 1);
    
    mainLayout->addLayout(playerInfoLayout);
}

void PlayerComparisonDialog::createTableLayouts()
{
    QHBoxLayout* tablesLayout = new QHBoxLayout();
    
    QTableView* table1 = new QTableView(this);
    setupPlayerTable(table1, player1History);
    
    QTableView* table2 = new QTableView(this);
    setupPlayerTable(table2, player2History);
    
    QVBoxLayout* player1TableLayout = new QVBoxLayout();
    
    QLabel* player1TableLabel = new QLabel(QString::fromStdString(player1.name), this);
    player1TableLabel->setStyleSheet("color: #0c7bb3; font-weight: bold;");
    player1TableLabel->setAlignment(Qt::AlignCenter);
    
    player1TableLayout->addWidget(player1TableLabel);
    player1TableLayout->addWidget(table1);
    
    QVBoxLayout* player2TableLayout = new QVBoxLayout();
    
    QLabel* player2TableLabel = new QLabel(QString::fromStdString(player2.name), this);
    player2TableLabel->setStyleSheet("color: #2ea043; font-weight: bold;");
    player2TableLabel->setAlignment(Qt::AlignCenter);
    
    player2TableLayout->addWidget(player2TableLabel);
    player2TableLayout->addWidget(table2);
    
    tablesLayout->addLayout(player1TableLayout);
    tablesLayout->addLayout(player2TableLayout);
    
    mainLayout->addLayout(tablesLayout);
}

void PlayerComparisonDialog::createButtonLayout()
{
    closeButton = new QPushButton("Close", this);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setMinimumWidth(120);
    closeButton->setMinimumHeight(36);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
}

void PlayerComparisonDialog::centerDialogOnScreen(QWidget* parent)
{
    if (parent) {
        QWidget* parentWidget = qobject_cast<QWidget*>(parent);
        if (parentWidget) {
            QScreen* screen = parentWidget->screen();
            if (screen) {
                QRect screenGeometry = screen->availableGeometry();
                move(screenGeometry.center() - rect().center());
            }
        }
    } else {
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            move(screenGeometry.center() - rect().center());
        }
    }
}

void PlayerComparisonDialog::setupPlayerTable(QTableView* table, const std::vector<RatingChange>& playerHistory)
{
    QStandardItemModel* model = new QStandardItemModel(this);
    
    QStringList headers = {"Date", "Rating", "Change"};
    model->setHorizontalHeaderLabels(headers);
    
    for (const auto& change : playerHistory) {
        QList<QStandardItem*> row;
        
        QDateTime date = QDateTime::fromString(QString::fromStdString(change.date), Qt::ISODate);
        QString formattedDate = date.isValid() ? date.toString("MMM dd, yyyy") : QString::fromStdString(change.date);
        row.append(new QStandardItem(formattedDate));
        
        QStandardItem* ratingItem = new QStandardItem(QString::number(change.newRating, 'f', 2));
        ratingItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        row.append(ratingItem);
        
        double ratingChange = change.newRating - change.previousRating;
        QStandardItem* changeItem = new QStandardItem((ratingChange >= 0 ? "+" : "") + 
                                                       QString::number(ratingChange, 'f', 2));
        changeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (ratingChange > 0) {
            changeItem->setForeground(QBrush(QColor(0x66, 0xbb, 0x6a)));
        } else if (ratingChange < 0) {
            changeItem->setForeground(QBrush(QColor(0xef, 0x53, 0x50)));
        }
        row.append(changeItem);
        
        model->appendRow(row);
    }
    
    table->setModel(model);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
}

QLineSeries* PlayerComparisonDialog::createPlayerSeries(const std::string& name, const QColor& color) {
    QLineSeries* series = new QLineSeries(chart);
    series->setName(QString::fromStdString(name));
    series->setColor(color);
    series->setPen(QPen(color, 3));
    series->setPointsVisible(true);
    return series;
}

void PlayerComparisonDialog::setupXAxis(QValueAxis* axis, size_t maxSize) {
    axis->setTitleText("Appearance");
    axis->setRange(0.5, maxSize + 0.5);
    axis->setTickType(QValueAxis::TickType::TicksDynamic);
    axis->setTickInterval(1.0);
    axis->setTickAnchor(1.0);
    axis->setLabelFormat("%d");
    axis->setMinorTickCount(0);
    axis->setLabelsColor(Qt::white);
    axis->setTitleBrush(QBrush(Qt::white));
    axis->setGridLineColor(QColor(0x3d, 0x3d, 0x3d));
    axis->setLinePenColor(QColor(0x3d, 0x3d, 0x3d));
}

void PlayerComparisonDialog::setupYAxis(QValueAxis* axis, double minRating, double maxRating) {
    axis->setRange(minRating, maxRating);
    axis->setLabelFormat("%.1f");
    axis->setTickCount(5);
    axis->setTitleText("Rating");
    axis->setLabelsColor(Qt::white);
    axis->setTitleBrush(QBrush(Qt::white));
    axis->setGridLineColor(QColor(0x3d, 0x3d, 0x3d));
    axis->setLinePenColor(QColor(0x3d, 0x3d, 0x3d));
}

void PlayerComparisonDialog::addSeriesToChart(QLineSeries* series1, QLineSeries* series2, QValueAxis* axisX, QValueAxis* axisY) {
    chart->addSeries(series1);
    chart->addSeries(series2);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series1->attachAxis(axisX);
    series1->attachAxis(axisY);
    series2->attachAxis(axisX);
    series2->attachAxis(axisY);
}

void PlayerComparisonDialog::setupComparisonChart()
{
    chart->removeAllSeries();

    if (rating1Progression.empty() || rating2Progression.empty()) {
        return;
    }

    QColor player1Color(0x0c, 0x7b, 0xb3);
    QColor player2Color(0x2e, 0xa0, 0x43);

    QLineSeries* series1 = createPlayerSeries(player1.name, player1Color);
    QLineSeries* series2 = createPlayerSeries(player2.name, player2Color);

    chart->legend()->setLabelColor(Qt::white);
    chart->legend()->setBackgroundVisible(false);
    chart->legend()->setFont(QFont("Segoe UI", 10));

    double minRating = std::numeric_limits<double>::max();
    double maxRating = std::numeric_limits<double>::lowest();

    for (size_t i = 0; i < rating1Progression.size(); ++i) {
        double rating = rating1Progression[i].second;
        series1->append(i + 1, rating);
        minRating = std::min(minRating, rating);
        maxRating = std::max(maxRating, rating);
    }

    for (size_t i = 0; i < rating2Progression.size(); ++i) {
        double rating = rating2Progression[i].second;
        series2->append(i + 1, rating);
        minRating = std::min(minRating, rating);
        maxRating = std::max(maxRating, rating);
    }

    double padding = std::max((maxRating - minRating) * 0.1, 10.0);
    minRating = std::max(0.0, minRating - padding);
    maxRating += padding;

    QValueAxis* gameAxisX = new QValueAxis(chart);
    setupXAxis(gameAxisX, std::max(rating1Progression.size(), rating2Progression.size()));

    QValueAxis* gameAxisY = new QValueAxis(chart);
    setupYAxis(gameAxisY, minRating, maxRating);

    addSeriesToChart(series1, series2, gameAxisX, gameAxisY);
}

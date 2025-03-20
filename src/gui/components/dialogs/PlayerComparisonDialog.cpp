#include "gui/components/dialogs/PlayerComparisonDialog.h"
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

void PlayerComparisonDialog::initializeUI() {
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

void PlayerComparisonDialog::loadPlayerData() {
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

QLabel* PlayerComparisonDialog::createPlayerLabel(const QString& text, const QString& color) {
    QLabel* label = new QLabel(text, this);
    label->setStyleSheet(QString("color: %1;").arg(color));
    return label;
}

QLabel* PlayerComparisonDialog::createPlayerInfoLabel(const QString& text, const QString& color) {
    QLabel* label = createPlayerLabel(text, color);
    return label;
}

QLabel* PlayerComparisonDialog::createBoldPlayerLabel(const QString& text, const QString& color) {
    QLabel* label = createPlayerLabel(text, color);
    label->setStyleSheet(label->styleSheet() + " font-weight: bold;");
    return label;
}

QVBoxLayout* PlayerComparisonDialog::createPlayerLayout(const Player& player, const QString& color, Qt::Alignment align) {
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(2);
    
    QLabel* nameLabel = createBoldPlayerLabel(QString::fromStdString(player.name), color);
    QLabel* clubLabel = createPlayerInfoLabel("Club: " + QString::fromStdString(player.clubName), color);
    QLabel* positionLabel = createPlayerInfoLabel("Position: " + QString::fromStdString(player.position), color);
    QLabel* ratingLabel = createPlayerInfoLabel("Rating: " + QString::number(player.rating, 'f', 1), color);
    
    layout->addWidget(nameLabel, 0, align);
    layout->addWidget(clubLabel, 0, align);
    layout->addWidget(positionLabel, 0, align);
    layout->addWidget(ratingLabel, 0, align);
    
    return layout;
}

void PlayerComparisonDialog::createPlayerInfoLayouts() {
    QHBoxLayout* playerInfoLayout = new QHBoxLayout();
    playerInfoLayout->setSpacing(20);
    
    QVBoxLayout* player1Layout = createPlayerLayout(player1, "#0c7bb3", Qt::AlignLeft);
    QVBoxLayout* player2Layout = createPlayerLayout(player2, "#2ea043", Qt::AlignRight);
    
    QWidget* player1Container = new QWidget(this);
    player1Container->setLayout(player1Layout);
    
    QWidget* player2Container = new QWidget(this);
    player2Container->setLayout(player2Layout);
    
    playerInfoLayout->addWidget(player1Container, 1);
    playerInfoLayout->addWidget(player2Container, 1);
    
    mainLayout->addLayout(playerInfoLayout);
}

void PlayerComparisonDialog::createTableLayouts() {
    QHBoxLayout* tablesLayout = new QHBoxLayout();
    QTableView* table1 = new QTableView(this);
    
    setupPlayerTable(table1, player1History);
    
    QVBoxLayout* player1TableLayout = new QVBoxLayout();
    QLabel* player1TableLabel = createBoldPlayerLabel(QString::fromStdString(player1.name), "#0c7bb3");
    player1TableLabel->setAlignment(Qt::AlignCenter);
    
    player1TableLayout->addWidget(player1TableLabel);
    player1TableLayout->addWidget(table1);
    
    QTableView* table2 = new QTableView(this);
    setupPlayerTable(table2, player2History);
    
    QVBoxLayout* player2TableLayout = new QVBoxLayout();
    QLabel* player2TableLabel = createBoldPlayerLabel(QString::fromStdString(player2.name), "#2ea043");
    player2TableLabel->setAlignment(Qt::AlignCenter);
    
    player2TableLayout->addWidget(player2TableLabel);
    player2TableLayout->addWidget(table2);
    
    tablesLayout->addLayout(player1TableLayout);
    tablesLayout->addLayout(player2TableLayout);
    
    mainLayout->addLayout(tablesLayout);
}

void PlayerComparisonDialog::createButtonLayout() {
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

void PlayerComparisonDialog::centerDialogOnScreen(QWidget* parent) {
    QScreen* screen = nullptr;
    
    if (parent) {
        QWidget* parentWidget = qobject_cast<QWidget*>(parent);
        if (parentWidget) {
            screen = parentWidget->screen();
        }
    }
    
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        move(screenGeometry.center() - rect().center());
    }
}

QStandardItem* PlayerComparisonDialog::createRatingItem(double rating) {
    QStandardItem* item = new QStandardItem(QString::number(rating, 'f', 2));
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return item;
}

QStandardItem* PlayerComparisonDialog::createChangeItem(double newRating, double previousRating) {
    double ratingChange = newRating - previousRating;
    QString prefix = ratingChange >= 0 ? "+" : "";
    QStandardItem* item = new QStandardItem(prefix + QString::number(ratingChange, 'f', 2));
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    if (ratingChange > 0) {
        item->setForeground(QBrush(QColor(0x66, 0xbb, 0x6a)));
    } else if (ratingChange < 0) {
        item->setForeground(QBrush(QColor(0xef, 0x53, 0x50)));
    }
    
    return item;
}

QStandardItem* PlayerComparisonDialog::createDateItem(const std::string& dateStr) {
    QDateTime date = QDateTime::fromString(QString::fromStdString(dateStr), Qt::ISODate);
    QString formattedDate = date.isValid() ? date.toString("MMM dd, yyyy") : QString::fromStdString(dateStr);
    return new QStandardItem(formattedDate);
}

QStandardItemModel* PlayerComparisonDialog::createTableModel(const std::vector<RatingChange>& playerHistory) {
    QStandardItemModel* model = new QStandardItemModel(this);
    QStringList headers = {"Date", "Rating", "Change"};
    model->setHorizontalHeaderLabels(headers);
    
    for (const auto& change : playerHistory) {
        QList<QStandardItem*> row;
        row.append(createDateItem(change.date));
        row.append(createRatingItem(change.newRating));
        row.append(createChangeItem(change.newRating, change.previousRating));
        model->appendRow(row);
    }
    
    return model;
}

void PlayerComparisonDialog::setupPlayerTable(QTableView* table, const std::vector<RatingChange>& playerHistory) {
    QStandardItemModel* model = createTableModel(playerHistory);
    
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

void PlayerComparisonDialog::setupLegend() {
    chart->legend()->setLabelColor(Qt::white);
    chart->legend()->setBackgroundVisible(false);
    chart->legend()->setFont(QFont("Segoe UI", 10));
}

void PlayerComparisonDialog::populateSeries(QLineSeries* series, const std::vector<std::pair<int, double>>& progression, 
                                         double& minRating, double& maxRating) {
    for (size_t i = 0; i < progression.size(); ++i) {
        double rating = progression[i].second;
        series->append(i + 1, rating);
        minRating = std::min(minRating, rating);
        maxRating = std::max(maxRating, rating);
    }
}

std::pair<double, double> PlayerComparisonDialog::findRatingBounds() {
    double minRating = std::numeric_limits<double>::max();
    double maxRating = std::numeric_limits<double>::lowest();
    
    return {minRating, maxRating};
}

void PlayerComparisonDialog::prepareChartSeries(QLineSeries*& series1, QLineSeries*& series2) {
    QColor player1Color(0x0c, 0x7b, 0xb3);
    QColor player2Color(0x2e, 0xa0, 0x43);

    series1 = createPlayerSeries(player1.name, player1Color);
    series2 = createPlayerSeries(player2.name, player2Color);
    
    setupLegend();
}

void PlayerComparisonDialog::calculateRatingRange(QLineSeries* series1, QLineSeries* series2, double& minRating, double& maxRating) {
    minRating = std::numeric_limits<double>::max();
    maxRating = std::numeric_limits<double>::lowest();
    
    populateSeries(series1, rating1Progression, minRating, maxRating);
    populateSeries(series2, rating2Progression, minRating, maxRating);
    
    double padding = std::max((maxRating - minRating) * 0.1, 10.0);
    minRating = std::max(0.0, minRating - padding);
    maxRating += padding;
}

void PlayerComparisonDialog::setupComparisonChart() {
    chart->removeAllSeries();

    if (rating1Progression.empty() || rating2Progression.empty()) {
        return;
    }

    QLineSeries* series1 = nullptr;
    QLineSeries* series2 = nullptr;
    prepareChartSeries(series1, series2);
    
    double minRating, maxRating;
    calculateRatingRange(series1, series2, minRating, maxRating);
    
    QValueAxis* gameAxisX = new QValueAxis(chart);
    setupXAxis(gameAxisX, std::max(rating1Progression.size(), rating2Progression.size()));

    QValueAxis* gameAxisY = new QValueAxis(chart);
    setupYAxis(gameAxisY, minRating, maxRating);

    addSeriesToChart(series1, series2, gameAxisX, gameAxisY);
}

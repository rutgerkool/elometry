#include "gui/components/dialogs/PlayerHistoryDialog.h"
#include <QHeaderView>
#include <QDateTime>
#include <QEasingCurve>
#include <QScrollArea>
#include <QLabel>
#include <QFont>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <algorithm>

PlayerHistoryDialog::PlayerHistoryDialog(RatingManager& rm, int pId, QWidget* parent)
    : QDialog(parent)
    , ratingManager(rm)
    , playerId(pId)
    , tableModel(nullptr)
    , chart(nullptr)
{
    setWindowTitle("Player Rating History");
    resize(900, 700);
    setModal(true);
    
    loadPlayerData();
    initializeUI();
    setupAnimations();
    setupHistoryChart();
    setupHistoryTable();
    centerDialog(parent);
    
    QTimer::singleShot(100, this, &PlayerHistoryDialog::animateContent);
}

PlayerHistoryDialog::~PlayerHistoryDialog() {}

void PlayerHistoryDialog::loadPlayerData() {
    auto allPlayers = ratingManager.getSortedRatedPlayers();
    
    for (const auto& p : allPlayers) {
        if (p.second.playerId == playerId) {
            player = p.second;
            playerHistory = ratingManager.getPlayerRatingHistory(playerId, 10);
            ratingProgression = ratingManager.getRecentRatingProgression(playerId, 10);
            return;
        }
    }
    
    close();
}

void PlayerHistoryDialog::initializeUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    setupTitle();
    setupPlayerInfo();
    setupChartView();
    setupTableView();
    
    closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeButton, 0, Qt::AlignCenter);
}

void PlayerHistoryDialog::setupTitle() {
    titleLabel = new QLabel(QString("Rating History for %1").arg(QString::fromStdString(player.name)), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
}

void PlayerHistoryDialog::setupPlayerInfo() {
    playerInfoLabel = new QLabel(QString("Club: %1 | Position: %2 | Current Rating: %3")
                               .arg(QString::fromStdString(player.clubName))
                               .arg(QString::fromStdString(player.position))
                               .arg(player.rating, 0, 'f', 1), this);
    playerInfoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(playerInfoLabel);
}

void PlayerHistoryDialog::setupChartView() {
    chart = new QChart();
    chart->setTitle("Rating Progression");
    chart->setTitleBrush(QBrush(Qt::white));
    chart->setTitleFont(QFont("Segoe UI", 14, QFont::Bold));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);
    
    mainLayout->addWidget(chartView);
}

void PlayerHistoryDialog::setupTableView() {
    historyTable = new QTableView(this);
    historyTable->setMinimumHeight(200);
    historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable->setAlternatingRowColors(true);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable->setTextElideMode(Qt::ElideRight);
    historyTable->verticalHeader()->setVisible(false);
    
    mainLayout->addWidget(historyTable);
}

void PlayerHistoryDialog::setupAnimations() {
    chartOpacityEffect = new QGraphicsOpacityEffect(chartView);
    chartView->setGraphicsEffect(chartOpacityEffect);
    chartOpacityEffect->setOpacity(0.0);
    
    chartAnimation = new QPropertyAnimation(chartOpacityEffect, "opacity", this);
    chartAnimation->setDuration(500);
    chartAnimation->setStartValue(0.0);
    chartAnimation->setEndValue(1.0);
    chartAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    tableOpacityEffect = new QGraphicsOpacityEffect(historyTable);
    historyTable->setGraphicsEffect(tableOpacityEffect);
    tableOpacityEffect->setOpacity(0.0);
    
    tableAnimation = new QPropertyAnimation(tableOpacityEffect, "opacity", this);
    tableAnimation->setDuration(500);
    tableAnimation->setStartValue(0.0);
    tableAnimation->setEndValue(1.0);
    tableAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    animationGroup = new QParallelAnimationGroup(this);
    animationGroup->addAnimation(chartAnimation);
    animationGroup->addAnimation(tableAnimation);
}

void PlayerHistoryDialog::centerDialog(QWidget* parent) {
    QScreen* screen = parent ? qobject_cast<QWidget*>(parent)->screen() : QGuiApplication::primaryScreen();
    
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        move(screenGeometry.center() - rect().center());
    }
}

void PlayerHistoryDialog::createTable() {
    if (tableModel) {
        delete tableModel;
    }
    
    tableModel = new QStandardItemModel(this);
    
    QStringList headers;
    headers << "Date" << "Opponent" << "Rating" << "Change" << "Minutes" << "Goals" << "Assists";
    tableModel->setHorizontalHeaderLabels(headers);
}

QStandardItem* PlayerHistoryDialog::createRatingItem(double rating) {
    QStandardItem* item = new QStandardItem(QString::number(rating, 'f', 2));
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    return item;
}

QStandardItem* PlayerHistoryDialog::createChangeItem(double previousRating, double newRating) {
    double ratingChange = newRating - previousRating;
    QStandardItem* item = new QStandardItem((ratingChange >= 0 ? "+" : "") + 
                                             QString::number(ratingChange, 'f', 2));
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    if (ratingChange > 0) {
        item->setForeground(QBrush(QColor(0, 150, 0)));
    } else if (ratingChange < 0) {
        item->setForeground(QBrush(QColor(200, 0, 0)));
    }
    
    return item;
}

void PlayerHistoryDialog::populateTable() {
    for (const auto& change : playerHistory) {
        QList<QStandardItem*> row;
        
        row.append(new QStandardItem(QString::fromStdString(change.date)));
        
        QStandardItem* opponentItem = new QStandardItem(QString::fromStdString(change.opponent));
        opponentItem->setToolTip(QString::fromStdString(change.opponent));
        row.append(opponentItem);
        
        row.append(createRatingItem(change.newRating));
        row.append(createChangeItem(change.previousRating, change.newRating));
        
        row.append(new QStandardItem(QString::number(change.minutesPlayed)));
        row.append(new QStandardItem(QString::number(change.goals)));
        row.append(new QStandardItem(QString::number(change.assists)));
        
        tableModel->appendRow(row);
    }
    
    historyTable->setModel(tableModel);
    setupTableColumns();
}

void PlayerHistoryDialog::setupTableColumns() {
    for (int i = 0; i < tableModel->columnCount(); i++) {
        if (i == 1) {
            historyTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
            historyTable->setColumnWidth(i, 350);
        } else if (i == 2) {
            historyTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        } else {
            historyTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
        }
    }
}

void PlayerHistoryDialog::setupHistoryTable() {
    createTable();
    populateTable();
}

void PlayerHistoryDialog::configureChartAppearance() {
    chart->removeAllSeries();
    
    chart->setBackgroundBrush(QBrush(QColor(0x25, 0x25, 0x25)));
    chart->setPlotAreaBackgroundBrush(QBrush(QColor(0x25, 0x25, 0x25)));
    chart->setPlotAreaBackgroundVisible(true);
    chart->setTitleBrush(QBrush(Qt::white));
    chart->setTitleFont(QFont("Segoe UI", 14, QFont::Bold));
    
    chart->legend()->setLabelColor(Qt::white);
    chart->legend()->setBackgroundVisible(false);
    chart->legend()->setFont(QFont("Segoe UI", 10));
}

void PlayerHistoryDialog::createChartSeries(QLineSeries* series, QScatterSeries* pointSeries) {
    series->setName("Rating");
    series->setColor(QColor(0x0c, 0x7b, 0xb3));
    series->setPen(QPen(QColor(0x0c, 0x7b, 0xb3), 3));

    pointSeries->setMarkerSize(10);
    pointSeries->setName("Games");
    pointSeries->setColor(QColor(0x2e, 0xa0, 0x43));
    pointSeries->setBorderColor(Qt::white);
    pointSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
}

void PlayerHistoryDialog::applyChartAxisStyling(QAbstractAxis* axis, const QString& title) {
    axis->setTitleText(title);
    axis->setLabelsColor(Qt::white);
    axis->setTitleBrush(QBrush(Qt::white));
    axis->setGridLineColor(QColor(0x3d, 0x3d, 0x3d));
    axis->setLinePenColor(QColor(0x3d, 0x3d, 0x3d));
}

double PlayerHistoryDialog::calculatePadding(double minRating, double maxRating) {
    double padding = (maxRating - minRating) * 0.1;
    return (padding < 10) ? 10 : padding;
}

void PlayerHistoryDialog::setupChartAxes(
    QLineSeries* series, 
    QScatterSeries* pointSeries, 
    double minRating, 
    double maxRating, 
    QDateTimeAxis* axisX, 
    QValueAxis* axisY
) {
    axisX->setTickCount(ratingProgression.size() > 5 ? 5 : ratingProgression.size());
    axisX->setFormat("MMM dd");
    applyChartAxisStyling(axisX, "Date");
    
    double padding = calculatePadding(minRating, maxRating);
    minRating = std::max(0.0, minRating - padding);
    maxRating = maxRating + padding;
    
    axisY->setRange(minRating, maxRating);
    axisY->setLabelFormat("%.1f");
    axisY->setTickCount(5);
    applyChartAxisStyling(axisY, "Rating");
    
    chart->addSeries(series);
    chart->addSeries(pointSeries);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    pointSeries->attachAxis(axisX);
    pointSeries->attachAxis(axisY);
}

void PlayerHistoryDialog::addPointToSeries(
    QLineSeries* series, 
    QScatterSeries* pointSeries, 
    const RatingChange& change, 
    int gameId, 
    double value, 
    double& minRating, 
    double& maxRating, 
    bool& firstPoint
) {
    if (change.gameId != gameId) return;
    
    QDateTime date = QDateTime::fromString(QString::fromStdString(change.date), Qt::ISODate);
    if (!date.isValid()) return;
    
    qreal x = date.toMSecsSinceEpoch();
    qreal y = value;
    
    if (firstPoint) {
        minRating = maxRating = y;
        firstPoint = false;
    } else {
        minRating = std::min(minRating, y);
        maxRating = std::max(maxRating, y);
    }
    
    *series << QPointF(x, y);
    *pointSeries << QPointF(x, y);
}

void PlayerHistoryDialog::createAndPopulateChartSeries(
    QLineSeries* series, 
    QScatterSeries* pointSeries, 
    double& minRating, 
    double& maxRating
) {
    bool firstPoint = true;
    
    for (size_t i = 0; i < ratingProgression.size(); i++) {
        if (i >= playerHistory.size()) continue;
        
        int gameId = ratingProgression[i].first;
        double value = ratingProgression[i].second;
        
        for (const auto& change : playerHistory) {
            addPointToSeries(series, pointSeries, change, gameId, value, 
                            minRating, maxRating, firstPoint);
            
            if (change.gameId == gameId) break;
        }
    }
}

void PlayerHistoryDialog::setupHistoryChart() {
    configureChartAppearance();
    
    QLineSeries* series = new QLineSeries(this);
    QScatterSeries* pointSeries = new QScatterSeries(this);
    
    createChartSeries(series, pointSeries);
    
    double minRating = 0;
    double maxRating = 0;
    
    createAndPopulateChartSeries(series, pointSeries, minRating, maxRating);
    
    QDateTimeAxis* axisX = new QDateTimeAxis(this);
    QValueAxis* axisY = new QValueAxis(this);
    
    setupChartAxes(series, pointSeries, minRating, maxRating, axisX, axisY);
}

void PlayerHistoryDialog::animateContent() {
    animationGroup->start();
}

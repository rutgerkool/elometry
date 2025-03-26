#include "gui/components/dialogs/PlayerHistoryDialog.h"
#include <QHeaderView>
#include <QDateTime>
#include <QEasingCurve>
#include <QScreen>
#include <QGuiApplication>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTimer>
#include <algorithm>
#include <ranges>

PlayerHistoryDialog::PlayerHistoryDialog(const RatingManager& ratingManager, int playerId, QWidget* parent)
    : QDialog(parent)
    , m_ratingManager(ratingManager)
    , m_playerId(playerId)
    , m_tableModel(nullptr)
    , m_chart(nullptr)
{
    setWindowTitle("Player Rating History");
    resize(900, 700);
    setModal(true);
    
    loadPlayerData();
    
    if (!m_player) {
        QTimer::singleShot(0, this, &QDialog::reject);
        return;
    }
    
    initializeUi();
    setupAnimations();
    setupChart();
    setupTable();
    centerOnParent();
    
    QTimer::singleShot(100, this, &PlayerHistoryDialog::animateContent);
}

void PlayerHistoryDialog::loadPlayerData() {
    const auto allPlayers = m_ratingManager.getSortedRatedPlayers();
    
    for (const auto& [id, player] : allPlayers) {
        if (player.playerId == m_playerId) {
            m_player = player;
            m_playerHistory = m_ratingManager.getPlayerRatingHistory(m_playerId, 10);
            m_ratingProgression = m_ratingManager.getRecentRatingProgression(m_playerId, 10);
            return;
        }
    }
}

void PlayerHistoryDialog::initializeUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    m_titleLabel = new QLabel(QString("Rating History for %1").arg(QString::fromStdString(m_player->name)), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    
    m_playerInfoLabel = new QLabel(QString("Club: %1 | Position: %2 | Current Rating: %3")
                               .arg(QString::fromStdString(m_player->clubName))
                               .arg(QString::fromStdString(m_player->position))
                               .arg(m_player->rating, 0, 'f', 1), this);
    m_playerInfoLabel->setAlignment(Qt::AlignCenter);
    
    setupChartView();
    setupTableView();
    
    m_closeButton = new QPushButton("Close", this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_playerInfoLabel);
    m_mainLayout->addWidget(m_chartView);
    m_mainLayout->addWidget(m_historyTable);
    m_mainLayout->addWidget(m_closeButton, 0, Qt::AlignCenter);
}

void PlayerHistoryDialog::setupChartView() {
    m_chart = new QChart();
    m_chart->setTitle("Rating Progression");
    m_chart->setTitleBrush(QBrush(Qt::white));
    m_chart->setTitleFont(QFont("Segoe UI", 14, QFont::Bold));
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    
    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(300);
}

void PlayerHistoryDialog::setupTableView() {
    m_historyTable = new QTableView(this);
    m_historyTable->setMinimumHeight(200);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setTextElideMode(Qt::ElideRight);
    m_historyTable->verticalHeader()->setVisible(false);
}

void PlayerHistoryDialog::setupAnimations() {
    m_chartOpacityEffect = new QGraphicsOpacityEffect(m_chartView);
    m_chartView->setGraphicsEffect(m_chartOpacityEffect);
    m_chartOpacityEffect->setOpacity(0.0);
    
    m_chartAnimation = new QPropertyAnimation(m_chartOpacityEffect, "opacity", this);
    m_chartAnimation->setDuration(500);
    m_chartAnimation->setStartValue(0.0);
    m_chartAnimation->setEndValue(1.0);
    m_chartAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    m_tableOpacityEffect = new QGraphicsOpacityEffect(m_historyTable);
    m_historyTable->setGraphicsEffect(m_tableOpacityEffect);
    m_tableOpacityEffect->setOpacity(0.0);
    
    m_tableAnimation = new QPropertyAnimation(m_tableOpacityEffect, "opacity", this);
    m_tableAnimation->setDuration(500);
    m_tableAnimation->setStartValue(0.0);
    m_tableAnimation->setEndValue(1.0);
    m_tableAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    m_animationGroup = new QParallelAnimationGroup(this);
    m_animationGroup->addAnimation(m_chartAnimation);
    m_animationGroup->addAnimation(m_tableAnimation);
}

void PlayerHistoryDialog::centerOnParent() {
    const QScreen* screen = parentWidget() 
        ? parentWidget()->screen() 
        : QGuiApplication::primaryScreen();
    
    if (screen) {
        const QRect screenGeometry = screen->availableGeometry();
        move(screenGeometry.center() - rect().center());
    }
}

void PlayerHistoryDialog::setupChart() {
    configureChartAppearance();
    createChartSeries();
    createChartAxes();
}

void PlayerHistoryDialog::configureChartAppearance() {
    m_chart->removeAllSeries();
    
    m_chart->setBackgroundBrush(QBrush(QColor(0x25, 0x25, 0x25)));
    m_chart->setPlotAreaBackgroundBrush(QBrush(QColor(0x25, 0x25, 0x25)));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->setTitleBrush(QBrush(Qt::white));
    m_chart->setTitleFont(QFont("Segoe UI", 14, QFont::Bold));
    
    m_chart->legend()->setLabelColor(Qt::white);
    m_chart->legend()->setBackgroundVisible(false);
    m_chart->legend()->setFont(QFont("Segoe UI", 10));
}

void PlayerHistoryDialog::createChartSeries() {
    auto* series = new QLineSeries(m_chart);
    auto* pointSeries = new QScatterSeries(m_chart);
    
    series->setName("Rating");
    series->setColor(QColor(0x0c, 0x7b, 0xb3));
    series->setPen(QPen(QColor(0x0c, 0x7b, 0xb3), 3));

    pointSeries->setMarkerSize(10);
    pointSeries->setName("Games");
    pointSeries->setColor(QColor(0x2e, 0xa0, 0x43));
    pointSeries->setBorderColor(Qt::white);
    pointSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    
    for (size_t i = 0; i < m_ratingProgression.size(); i++) {
        if (i >= m_playerHistory.size()) continue;
        
        int gameId = m_ratingProgression[i].first;
        double value = m_ratingProgression[i].second;
        
        for (const auto& change : m_playerHistory) {
            if (change.gameId != gameId) continue;
            
            QDateTime date = QDateTime::fromString(QString::fromStdString(change.date), Qt::ISODate);
            if (!date.isValid()) continue;
            
            qreal x = date.toMSecsSinceEpoch();
            qreal y = value;
            
            *series << QPointF(x, y);
            *pointSeries << QPointF(x, y);
            
            break;
        }
    }
    
    m_chart->addSeries(series);
    m_chart->addSeries(pointSeries);
}

void PlayerHistoryDialog::createChartAxes() {
    auto* axisX = new QDateTimeAxis(this);
    auto* axisY = new QValueAxis(this);
    
    axisX->setTickCount(m_ratingProgression.size() > 5 ? 5 : m_ratingProgression.size());
    axisX->setFormat("MMM dd");
    
    axisX->setTitleText("Date");
    axisX->setLabelsColor(Qt::white);
    axisX->setTitleBrush(QBrush(Qt::white));
    axisX->setGridLineColor(QColor(0x3d, 0x3d, 0x3d));
    axisX->setLinePenColor(QColor(0x3d, 0x3d, 0x3d));
    
    const auto [minRating, maxRating] = calculateRatingRange();
    double padding = calculatePadding(minRating, maxRating);
    
    double yMin = std::max(0.0, minRating - padding);
    double yMax = maxRating + padding;
    
    axisY->setRange(yMin, yMax);
    axisY->setLabelFormat("%.1f");
    axisY->setTickCount(5);
    
    axisY->setTitleText("Rating");
    axisY->setLabelsColor(Qt::white);
    axisY->setTitleBrush(QBrush(Qt::white));
    axisY->setGridLineColor(QColor(0x3d, 0x3d, 0x3d));
    axisY->setLinePenColor(QColor(0x3d, 0x3d, 0x3d));
    
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    
    for (const auto& series : m_chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
}

std::pair<double, double> PlayerHistoryDialog::calculateRatingRange() const {
    if (m_ratingProgression.empty()) {
        return {1400.0, 1600.0};
    }
    
    double minRating = std::numeric_limits<double>::max();
    double maxRating = std::numeric_limits<double>::lowest();
    
    for (const auto& [_, rating] : m_ratingProgression) {
        minRating = std::min(minRating, rating);
        maxRating = std::max(maxRating, rating);
    }
    
    return {minRating, maxRating};
}

double PlayerHistoryDialog::calculatePadding(double minRating, double maxRating) const {
    double padding = (maxRating - minRating) * 0.1;
    return (padding < 10) ? 10.0 : padding;
}

void PlayerHistoryDialog::setupTable() {
    m_tableModel = new QStandardItemModel(this);
    
    QStringList headers;
    headers << "Date" << "Opponent" << "Rating" << "Change" << "Minutes" << "Goals" << "Assists";
    m_tableModel->setHorizontalHeaderLabels(headers);
    
    populateTableWithHistory();
    m_historyTable->setModel(m_tableModel);
    configureTableColumns();
}

void PlayerHistoryDialog::populateTableWithHistory() {
    for (const auto& change : m_playerHistory) {
        QList<QStandardItem*> row;
        
        row.append(new QStandardItem(QString::fromStdString(change.date)));
        
        QStandardItem* opponentItem = new QStandardItem(QString::fromStdString(change.opponent));
        opponentItem->setToolTip(QString::fromStdString(change.opponent));
        row.append(opponentItem);
        
        QStandardItem* ratingItem = new QStandardItem(QString::number(change.newRating, 'f', 2));
        ratingItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        row.append(ratingItem);
        
        QStandardItem* changeItem = new QStandardItem();
        double ratingChange = change.newRating - change.previousRating;
        QString changeText = (ratingChange >= 0 ? "+" : "") + QString::number(ratingChange, 'f', 2);
        changeItem->setText(changeText);
        changeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        
        if (ratingChange > 0) {
            changeItem->setForeground(QBrush(QColor(0, 150, 0)));
        } else if (ratingChange < 0) {
            changeItem->setForeground(QBrush(QColor(200, 0, 0)));
        }
        
        row.append(changeItem);
        
        row.append(new QStandardItem(QString::number(change.minutesPlayed)));
        row.append(new QStandardItem(QString::number(change.goals)));
        row.append(new QStandardItem(QString::number(change.assists)));
        
        m_tableModel->appendRow(row);
    }
}

void PlayerHistoryDialog::configureTableColumns() {
    for (int i = 0; i < m_tableModel->columnCount(); i++) {
        if (i == 1) {
            m_historyTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
            m_historyTable->setColumnWidth(i, 350);
        } else if (i == 2) {
            m_historyTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        } else {
            m_historyTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
        }
    }
}

void PlayerHistoryDialog::animateContent() {
    m_animationGroup->start();
}

#include "gui/components/dialogs/PlayerComparisonDialog.h"

#include <QHeaderView>
#include <QDateTime>
#include <QLabel>
#include <QFont>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QPushButton>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLegendMarker>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <algorithm>
#include <limits>

PlayerComparisonDialog::PlayerComparisonDialog(RatingManager& ratingManager, int playerId1, int playerId2, QWidget* parent)
    : QDialog(parent)
    , m_ratingManager(ratingManager)
    , m_playerId1(playerId1)
    , m_playerId2(playerId2)
{
    setWindowTitle(tr("Player Rating Comparison"));
    resize(1024, 768);
    setModal(true);
    
    initializeUI();
    loadPlayerData();
    
    if (!validatePlayerData()) {
        QTimer::singleShot(0, this, &QDialog::reject);
        return;
    }
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(24, 24, 24, 24);
    
    createPlayerInfoSection();
    createChartSection();
    createTableSection();
    createButtonSection();
    centerOnParent(parent);
}

void PlayerComparisonDialog::initializeUI() {
    m_chart = new QChart();
    m_chartView = new QChartView(m_chart, this);
    
    m_chart->setTitle(tr("Rating Progression by Appearance"));
    m_chart->setTitleBrush(QBrush(Qt::white));
    
    QFont chartTitleFont("Segoe UI", 14);
    chartTitleFont.setBold(true);
    m_chart->setTitleFont(chartTitleFont);
    
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->setBackgroundBrush(QBrush(QColor(0x2a, 0x2a, 0x2a)));
    m_chart->setPlotAreaBackgroundBrush(QBrush(QColor(0x2a, 0x2a, 0x2a)));
    m_chart->setPlotAreaBackgroundVisible(true);
    
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(250);
}

void PlayerComparisonDialog::loadPlayerData() {
    const auto allPlayers = m_ratingManager.getSortedRatedPlayers();
    
    for (const auto& [id, player] : allPlayers) {
        if (player.playerId == m_playerId1) {
            m_player1 = player;
        }
        if (player.playerId == m_playerId2) {
            m_player2 = player;
        }
    }
    
    if (m_player1 && m_player2) {
        m_player1History = m_ratingManager.getPlayerRatingHistory(m_playerId1, 10);
        m_player2History = m_ratingManager.getPlayerRatingHistory(m_playerId2, 10);
        m_rating1Progression = m_ratingManager.getRecentRatingProgression(m_playerId1, 10);
        m_rating2Progression = m_ratingManager.getRecentRatingProgression(m_playerId2, 10);
    }
}

bool PlayerComparisonDialog::validatePlayerData() const {
    return m_player1.has_value() && m_player2.has_value() && 
           !m_rating1Progression.empty() && !m_rating2Progression.empty();
}

QLabel* PlayerComparisonDialog::createPlayerNameLabel(const QString& text, const QColor& color) const {
    auto* label = new QLabel(text);
    label->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color.name()));
    return label;
}

QLabel* PlayerComparisonDialog::createPlayerInfoLabel(const QString& text, const QColor& color) const {
    auto* label = new QLabel(text);
    label->setStyleSheet(QString("color: %1;").arg(color.name()));
    return label;
}

QHBoxLayout* PlayerComparisonDialog::createPlayerInfoLayout() const {
    auto* layout = new QHBoxLayout();
    layout->setSpacing(20);
    
    auto* player1Layout = new QVBoxLayout();
    player1Layout->setSpacing(2);
    
    auto* player2Layout = new QVBoxLayout();
    player2Layout->setSpacing(2);
    
    const QString player1Name = QString::fromStdString(m_player1->name);
    const QString player2Name = QString::fromStdString(m_player2->name);
    
    auto* player1NameLabel = createPlayerNameLabel(player1Name, PLAYER1_COLOR);
    auto* player2NameLabel = createPlayerNameLabel(player2Name, PLAYER2_COLOR);
    
    player1Layout->addWidget(player1NameLabel, 0, Qt::AlignLeft);
    player1Layout->addWidget(createPlayerInfoLabel(tr("Club: %1").arg(QString::fromStdString(m_player1->clubName)), PLAYER1_COLOR), 0, Qt::AlignLeft);
    player1Layout->addWidget(createPlayerInfoLabel(tr("Position: %1").arg(QString::fromStdString(m_player1->position)), PLAYER1_COLOR), 0, Qt::AlignLeft);
    player1Layout->addWidget(createPlayerInfoLabel(tr("Rating: %1").arg(m_player1->rating, 0, 'f', 1), PLAYER1_COLOR), 0, Qt::AlignLeft);
    
    player2Layout->addWidget(player2NameLabel, 0, Qt::AlignRight);
    player2Layout->addWidget(createPlayerInfoLabel(tr("Club: %1").arg(QString::fromStdString(m_player2->clubName)), PLAYER2_COLOR), 0, Qt::AlignRight);
    player2Layout->addWidget(createPlayerInfoLabel(tr("Position: %1").arg(QString::fromStdString(m_player2->position)), PLAYER2_COLOR), 0, Qt::AlignRight);
    player2Layout->addWidget(createPlayerInfoLabel(tr("Rating: %1").arg(m_player2->rating, 0, 'f', 1), PLAYER2_COLOR), 0, Qt::AlignRight);
    
    auto* player1Container = new QWidget();
    player1Container->setLayout(player1Layout);
    
    auto* player2Container = new QWidget();
    player2Container->setLayout(player2Layout);
    
    layout->addWidget(player1Container, 1);
    layout->addWidget(player2Container, 1);
    
    return layout;
}

void PlayerComparisonDialog::createPlayerInfoSection() {
    m_mainLayout->addLayout(createPlayerInfoLayout());
}

void PlayerComparisonDialog::createChartSection() {
    setupChart();
    m_mainLayout->addWidget(m_chartView);
}

void PlayerComparisonDialog::createTableSection() {
    auto* tablesLayout = new QHBoxLayout();
    
    auto* table1 = new QTableView(this);
    setupPlayerTable(table1, m_player1History);
    
    auto* player1TableLayout = new QVBoxLayout();
    auto* player1TableLabel = createPlayerNameLabel(QString::fromStdString(m_player1->name), PLAYER1_COLOR);
    player1TableLabel->setAlignment(Qt::AlignCenter);
    
    player1TableLayout->addWidget(player1TableLabel);
    player1TableLayout->addWidget(table1);
    
    auto* table2 = new QTableView(this);
    setupPlayerTable(table2, m_player2History);
    
    auto* player2TableLayout = new QVBoxLayout();
    auto* player2TableLabel = createPlayerNameLabel(QString::fromStdString(m_player2->name), PLAYER2_COLOR);
    player2TableLabel->setAlignment(Qt::AlignCenter);
    
    player2TableLayout->addWidget(player2TableLabel);
    player2TableLayout->addWidget(table2);
    
    tablesLayout->addLayout(player1TableLayout);
    tablesLayout->addLayout(player2TableLayout);
    
    m_mainLayout->addLayout(tablesLayout);
}

void PlayerComparisonDialog::createButtonSection() {
    m_closeButton = new QPushButton(tr("Close"), this);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setMinimumWidth(120);
    m_closeButton->setMinimumHeight(36);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    buttonLayout->addStretch();
    
    m_mainLayout->addLayout(buttonLayout);
}

void PlayerComparisonDialog::centerOnParent(QWidget* parent) {
    const QScreen* screen = parent 
        ? parent->screen() 
        : QGuiApplication::primaryScreen();
    
    if (screen) {
        const QRect screenGeometry = screen->availableGeometry();
        move(screenGeometry.center() - rect().center());
    }
}

void PlayerComparisonDialog::setupPlayerTable(QTableView* tableView, std::span<const RatingChange> playerHistory) {
    auto* model = createTableModel(playerHistory);
    
    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setAlternatingRowColors(true);
    tableView->verticalHeader()->setVisible(false);
}

QStandardItemModel* PlayerComparisonDialog::createTableModel(std::span<const RatingChange> playerHistory) const {
    auto* model = new QStandardItemModel(static_cast<int>(playerHistory.size()), 3, const_cast<PlayerComparisonDialog*>(this));
    QStringList headers = {tr("Date"), tr("Rating"), tr("Change")};
    model->setHorizontalHeaderLabels(headers);
    
    for (int row = 0; row < static_cast<int>(playerHistory.size()); ++row) {
        const auto& change = playerHistory[row];
        
        QDateTime date = QDateTime::fromString(QString::fromStdString(change.date), Qt::ISODate);
        QString formattedDate = date.isValid() ? date.toString("MMM dd, yyyy") : QString::fromStdString(change.date);
        model->setItem(row, 0, new QStandardItem(formattedDate));
        
        auto* ratingItem = new QStandardItem(QString::number(change.newRating, 'f', 2));
        ratingItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        model->setItem(row, 1, ratingItem);
        
        double ratingChange = change.newRating - change.previousRating;
        QString prefix = ratingChange >= 0 ? "+" : "";
        auto* changeItem = new QStandardItem(prefix + QString::number(ratingChange, 'f', 2));
        changeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        
        if (ratingChange > 0) {
            changeItem->setForeground(QBrush(QColor(0x66, 0xbb, 0x6a)));
        } else if (ratingChange < 0) {
            changeItem->setForeground(QBrush(QColor(0xef, 0x53, 0x50)));
        }
        
        model->setItem(row, 2, changeItem);
    }
    
    return model;
}

void PlayerComparisonDialog::setupChart() {
    m_chart->removeAllSeries();
    
    if (m_rating1Progression.empty() || m_rating2Progression.empty()) {
        return;
    }
    
    populateChart();
}

void PlayerComparisonDialog::populateChart() {
    auto* series1 = new QLineSeries(m_chart);
    auto* series2 = new QLineSeries(m_chart);
    
    configureSeries(series1, series2);
    
    double minRating = 0.0;
    double maxRating = 0.0;
    
    auto [min, max] = calculateRatingRange(series1, series2);
    minRating = min;
    maxRating = max;
    
    auto* xAxis = new QValueAxis(m_chart);
    auto* yAxis = new QValueAxis(m_chart);
    
    configureAxes(xAxis, yAxis, minRating, maxRating);
    
    m_chart->addSeries(series1);
    m_chart->addSeries(series2);
    m_chart->addAxis(xAxis, Qt::AlignBottom);
    m_chart->addAxis(yAxis, Qt::AlignLeft);
    
    series1->attachAxis(xAxis);
    series1->attachAxis(yAxis);
    series2->attachAxis(xAxis);
    series2->attachAxis(yAxis);
}

void PlayerComparisonDialog::configureSeries(QLineSeries* series1, QLineSeries* series2) {
    series1->setName(QString::fromStdString(m_player1->name));
    series1->setColor(PLAYER1_COLOR);
    series1->setPen(QPen(PLAYER1_COLOR, 3));
    series1->setPointsVisible(true);
    
    series2->setName(QString::fromStdString(m_player2->name));
    series2->setColor(PLAYER2_COLOR);
    series2->setPen(QPen(PLAYER2_COLOR, 3));
    series2->setPointsVisible(true);
    
    m_chart->legend()->setLabelColor(Qt::white);
    m_chart->legend()->setBackgroundVisible(false);
    m_chart->legend()->setFont(QFont("Segoe UI", 10));
}

void PlayerComparisonDialog::configureAxes(QValueAxis* xAxis, QValueAxis* yAxis, double minRating, double maxRating) {
    size_t maxSize = std::max(m_rating1Progression.size(), m_rating2Progression.size());
    xAxis->setTitleText(tr("Appearance"));
    xAxis->setRange(0.5, maxSize + 0.5);
    xAxis->setTickType(QValueAxis::TickType::TicksDynamic);
    xAxis->setTickInterval(1.0);
    xAxis->setTickAnchor(1.0);
    xAxis->setLabelFormat("%d");
    xAxis->setMinorTickCount(0);
    xAxis->setLabelsColor(Qt::white);
    xAxis->setTitleBrush(QBrush(Qt::white));
    xAxis->setGridLineColor(QColor(0x3d, 0x3d, 0x3d));
    xAxis->setLinePenColor(QColor(0x3d, 0x3d, 0x3d));
    
    double padding = calculateRatingPadding(minRating, maxRating);
    minRating = std::max(0.0, minRating - padding);
    maxRating = maxRating + padding;
    
    yAxis->setRange(minRating, maxRating);
    yAxis->setLabelFormat("%.1f");
    yAxis->setTickCount(5);
    yAxis->setTitleText(tr("Rating"));
    yAxis->setLabelsColor(Qt::white);
    yAxis->setTitleBrush(QBrush(Qt::white));
    yAxis->setGridLineColor(QColor(0x3d, 0x3d, 0x3d));
    yAxis->setLinePenColor(QColor(0x3d, 0x3d, 0x3d));
}

double PlayerComparisonDialog::calculateRatingPadding(double minRating, double maxRating) const {
    return std::max((maxRating - minRating) * 0.1, 10.0);
}

void PlayerComparisonDialog::populateRatingData(QLineSeries* series, const std::vector<std::pair<int, double>>& progression, 
                                             double& minRating, double& maxRating) {
    for (size_t i = 0; i < progression.size(); ++i) {
        double rating = progression[i].second;
        series->append(i + 1, rating);
        minRating = std::min(minRating, rating);
        maxRating = std::max(maxRating, rating);
    }
}

std::pair<double, double> PlayerComparisonDialog::calculateRatingRange(QLineSeries* series1, QLineSeries* series2) {
    double minRating = std::numeric_limits<double>::max();
    double maxRating = std::numeric_limits<double>::lowest();
    
    populateRatingData(series1, m_rating1Progression, minRating, maxRating);
    populateRatingData(series2, m_rating2Progression, minRating, maxRating);
    
    return {minRating, maxRating};
}

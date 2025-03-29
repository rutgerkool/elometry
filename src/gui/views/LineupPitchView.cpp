#include "gui/views/LineupPitchView.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QtWidgets/QListWidget>
#include <QDrag>
#include <algorithm>

LineupPitchView::LineupPitchView(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(new QGridLayout(this))
{
    setMinimumHeight(500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(5);
    
    initializeFormations();
    createPlayerWidgets();
    
    setFormation("4-3-3");
}

LineupPitchView::~LineupPitchView() {
    for (auto it = m_positionWidgets.begin(); it != m_positionWidgets.end(); ++it) {
        delete it.value();
    }
    m_positionWidgets.clear();
}

void LineupPitchView::initializeFormations() {
    m_formationPositionsMap["4-3-3"] = {"GK", "LB", "CB1", "CB2", "RB", "CM1", "CM2", "CM3", "LW", "ST", "RW"};
    m_formationPositionsMap["4-4-2"] = {"GK", "LB", "CB1", "CB2", "RB", "LM", "CM1", "CM2", "RM", "ST1", "ST2"};
    m_formationPositionsMap["5-3-2"] = {"GK", "LWB", "CB1", "CB2", "CB3", "RWB", "CM1", "CM2", "CM3", "ST1", "ST2"};
    m_formationPositionsMap["3-5-2"] = {"GK", "CB1", "CB2", "CB3", "LM", "CM1", "CM2", "CM3", "RM", "ST1", "ST2"};
    m_formationPositionsMap["4-2-3-1"] = {"GK", "LB", "CB1", "CB2", "RB", "CDM1", "CDM2", "CAM1", "CAM2", "CAM3", "ST"};
}

QStringList LineupPitchView::getAllPositions() const {
    return {"GK", 
            "LB", "CB1", "CB2", "CB3", "RB", 
            "LWB", "RWB",
            "CDM1", "CDM2", 
            "CM1", "CM2", "CM3", 
            "LM", "RM", 
            "CAM1", "CAM2", "CAM3", 
            "LW", "RW", 
            "ST", "ST1", "ST2"};
}

void LineupPitchView::setupPlayerPositionWidget(PlayerPositionWidget* posWidget) {
    connect(posWidget, &PlayerPositionWidget::playerDropped,
            this, &LineupPitchView::playerDragDropped);
    
    connect(posWidget, &PlayerPositionWidget::playerClicked, 
            this, &LineupPitchView::playerClicked);
}

void LineupPitchView::createPlayerWidgets() {
    const QStringList allPositions = getAllPositions();
    
    for (const QString& position : allPositions) {
        auto* posWidget = new PlayerPositionWidget(position, this);
        posWidget->setVisible(false);
        
        setupPlayerPositionWidget(posWidget);
        m_positionWidgets.insert(position, posWidget);
    }
}

void LineupPitchView::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawPitch(painter);
    
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void LineupPitchView::drawPitch(QPainter& painter) {
    QRect fieldRect = rect().adjusted(20, 20, -20, -20);
    
    QLinearGradient gradient(fieldRect.topLeft(), fieldRect.bottomRight());
    gradient.setColorAt(0, QColor(80, 170, 80));
    gradient.setColorAt(1, QColor(50, 150, 50));
    painter.fillRect(fieldRect, gradient);
    
    painter.setPen(QPen(Qt::white, 3));
    painter.drawRect(fieldRect);
    
    drawCenterLine(painter, fieldRect);
    drawCenterCircle(painter, fieldRect);
    drawPenaltyAreas(painter, fieldRect);
    drawGoals(painter, fieldRect);
    drawGridLines(painter, fieldRect);
}

void LineupPitchView::drawCenterLine(QPainter& painter, const QRect& fieldRect) {
    int centerY = fieldRect.top() + fieldRect.height() / 2;
    painter.drawLine(fieldRect.left(), centerY, fieldRect.right(), centerY);
}

void LineupPitchView::drawCenterCircle(QPainter& painter, const QRect& fieldRect) {
    int centerX = fieldRect.left() + fieldRect.width() / 2;
    int centerY = fieldRect.top() + fieldRect.height() / 2;
    
    int radius = fieldRect.width() / 10;
    painter.drawEllipse(QPoint(centerX, centerY), radius, radius);
    
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPoint(centerX, centerY), 5, 5);
}

void LineupPitchView::drawPenaltyAreas(QPainter& painter, const QRect& fieldRect) {
    int centerX = fieldRect.left() + fieldRect.width() / 2;
    int penaltyWidth = fieldRect.width() / 3;
    int penaltyHeight = fieldRect.height() / 5;
    
    QRect topPenaltyArea(
        centerX - penaltyWidth / 2,
        fieldRect.top(),
        penaltyWidth,
        penaltyHeight
    );
    painter.drawRect(topPenaltyArea);
    
    QRect bottomPenaltyArea(
        centerX - penaltyWidth / 2,
        fieldRect.bottom() - penaltyHeight,
        penaltyWidth,
        penaltyHeight
    );
    painter.drawRect(bottomPenaltyArea);
    
    int goalWidth = penaltyWidth / 2;
    int goalHeight = penaltyHeight / 2;
    
    painter.drawRect(
        centerX - goalWidth / 2,
        fieldRect.top(),
        goalWidth,
        goalHeight
    );
    
    painter.drawRect(
        centerX - goalWidth / 2,
        fieldRect.bottom() - goalHeight,
        goalWidth,
        goalHeight
    );
}

void LineupPitchView::drawGoals(QPainter& painter, const QRect& fieldRect) {
    int centerX = fieldRect.left() + fieldRect.width() / 2;
    int penaltyWidth = fieldRect.width() / 3;
    int goalLineWidth = penaltyWidth / 3;
    
    QPen goalPen(Qt::white, 4);
    painter.setPen(goalPen);
    
    QRect topGoalRect(
        centerX - goalLineWidth / 2,
        fieldRect.top() - 15,
        goalLineWidth,
        15
    );
    painter.drawRect(topGoalRect);
    
    QRect bottomGoalRect(
        centerX - goalLineWidth / 2,
        fieldRect.bottom(),
        goalLineWidth,
        15
    );
    painter.drawRect(bottomGoalRect);
}

void LineupPitchView::drawGridLines(QPainter& painter, const QRect& fieldRect) {
    painter.setPen(QPen(QColor(255, 255, 255, 60), 1, Qt::DashLine));
    
    for (int i = 1; i < 10; i++) {
        int x = fieldRect.left() + (fieldRect.width() * i) / 10;
        painter.drawLine(x, fieldRect.top(), x, fieldRect.bottom());
        
        int y = fieldRect.top() + (fieldRect.height() * i) / 10;
        painter.drawLine(fieldRect.left(), y, fieldRect.right(), y);
    }
}

void LineupPitchView::setFormation(const QString& formation) {
    if (!m_formationPositionsMap.contains(formation)) {
        return;
    }
    
    m_currentFormation = formation;
    clearPositions();
    setupFormationPositions(formation);
}

void LineupPitchView::clearPositions() {
    for (auto it = m_positionWidgets.begin(); it != m_positionWidgets.end(); ++it) {
        it.value()->clearPlayer();
        it.value()->setVisible(false);
        
        if (it.value()->parentWidget() == this) {
            m_mainLayout->removeWidget(it.value());
        }
    }
}

void LineupPitchView::clearPosition(const QString& position) {
    if (m_positionWidgets.contains(position)) {
        m_positionWidgets[position]->clearPlayer();
    }
}

QMap<QString, QPoint> LineupPitchView::getFormation433Coordinates() const {
    QMap<QString, QPoint> positionCoords;
    
    positionCoords["LB"] = QPoint(1, 7);
    positionCoords["CB1"] = QPoint(3, 7);
    positionCoords["CB2"] = QPoint(5, 7);
    positionCoords["RB"] = QPoint(7, 7);
    
    positionCoords["CM1"] = QPoint(2, 5);
    positionCoords["CM2"] = QPoint(4, 5);
    positionCoords["CM3"] = QPoint(6, 5);
    
    positionCoords["LW"] = QPoint(1, 2);
    positionCoords["ST"] = QPoint(4, 2);
    positionCoords["RW"] = QPoint(7, 2);
    
    return positionCoords;
}

QMap<QString, QPoint> LineupPitchView::getFormation442Coordinates() const {
    QMap<QString, QPoint> positionCoords;
    
    positionCoords["LB"] = QPoint(1, 7);
    positionCoords["CB1"] = QPoint(3, 7);
    positionCoords["CB2"] = QPoint(5, 7);
    positionCoords["RB"] = QPoint(7, 7);
    
    positionCoords["LM"] = QPoint(1, 5);
    positionCoords["CM1"] = QPoint(3, 5);
    positionCoords["CM2"] = QPoint(5, 5);
    positionCoords["RM"] = QPoint(7, 5);
    
    positionCoords["ST1"] = QPoint(3, 2);
    positionCoords["ST2"] = QPoint(5, 2);
    
    return positionCoords;
}

QMap<QString, QPoint> LineupPitchView::getFormation532Coordinates() const {
    QMap<QString, QPoint> positionCoords;
    
    positionCoords["LWB"] = QPoint(0, 7);
    positionCoords["CB1"] = QPoint(2, 7);
    positionCoords["CB2"] = QPoint(4, 7);
    positionCoords["CB3"] = QPoint(6, 7);
    positionCoords["RWB"] = QPoint(8, 7);
    
    positionCoords["CM1"] = QPoint(2, 5);
    positionCoords["CM2"] = QPoint(4, 5);
    positionCoords["CM3"] = QPoint(6, 5);
    
    positionCoords["ST1"] = QPoint(3, 2);
    positionCoords["ST2"] = QPoint(5, 2);
    
    return positionCoords;
}

QMap<QString, QPoint> LineupPitchView::getFormation352Coordinates() const {
    QMap<QString, QPoint> positionCoords;
    
    positionCoords["CB1"] = QPoint(2, 7);
    positionCoords["CB2"] = QPoint(4, 7);
    positionCoords["CB3"] = QPoint(6, 7);
    
    positionCoords["LM"] = QPoint(0, 5);
    positionCoords["CM1"] = QPoint(2, 5);
    positionCoords["CM2"] = QPoint(4, 5);
    positionCoords["CM3"] = QPoint(6, 5);
    positionCoords["RM"] = QPoint(8, 5);
    
    positionCoords["ST1"] = QPoint(3, 2);
    positionCoords["ST2"] = QPoint(5, 2);
    
    return positionCoords;
}

QMap<QString, QPoint> LineupPitchView::getFormation4231Coordinates() const {
    QMap<QString, QPoint> positionCoords;
    
    positionCoords["LB"] = QPoint(1, 7);
    positionCoords["CB1"] = QPoint(3, 7);
    positionCoords["CB2"] = QPoint(5, 7);
    positionCoords["RB"] = QPoint(7, 7);
    
    positionCoords["CDM1"] = QPoint(3, 5);
    positionCoords["CDM2"] = QPoint(5, 5);
    
    positionCoords["CAM1"] = QPoint(2, 3);
    positionCoords["CAM2"] = QPoint(4, 3);
    positionCoords["CAM3"] = QPoint(6, 3);
    
    positionCoords["ST"] = QPoint(4, 1);
    
    return positionCoords;
}

QMap<QString, QPoint> LineupPitchView::getFormationCoordinates(const QString& formation) const {
    QMap<QString, QPoint> positionCoords;
    
    positionCoords["GK"] = QPoint(4, 9);
    
    if (formation == "4-3-3") {
        const auto formationCoords = getFormation433Coordinates();
        for (auto it = formationCoords.constBegin(); it != formationCoords.constEnd(); ++it) {
            positionCoords[it.key()] = it.value();
        }
    }
    else if (formation == "4-4-2") {
        const auto formationCoords = getFormation442Coordinates();
        for (auto it = formationCoords.constBegin(); it != formationCoords.constEnd(); ++it) {
            positionCoords[it.key()] = it.value();
        }
    }
    else if (formation == "5-3-2") {
        const auto formationCoords = getFormation532Coordinates();
        for (auto it = formationCoords.constBegin(); it != formationCoords.constEnd(); ++it) {
            positionCoords[it.key()] = it.value();
        }
    }
    else if (formation == "3-5-2") {
        const auto formationCoords = getFormation352Coordinates();
        for (auto it = formationCoords.constBegin(); it != formationCoords.constEnd(); ++it) {
            positionCoords[it.key()] = it.value();
        }
    }
    else if (formation == "4-2-3-1") {
        const auto formationCoords = getFormation4231Coordinates();
        for (auto it = formationCoords.constBegin(); it != formationCoords.constEnd(); ++it) {
            positionCoords[it.key()] = it.value();
        }
    }
    
    return positionCoords;
}

QPoint LineupPitchView::getPositionCoordinates(const QString& position) const {
    const auto positionCoords = getFormationCoordinates(m_currentFormation);
    
    if (!positionCoords.contains(position)) {
        return QPoint(4, 5);
    }
    
    return positionCoords[position];
}

void LineupPitchView::setupFormationPositions(const QString& formation) {
    if (!m_formationPositionsMap.contains(formation)) {
        return;
    }
    
    m_mainLayout->setSpacing(4);
    
    for (int i = 0; i < 10; i++) {
        m_mainLayout->setColumnStretch(i, 1);
        m_mainLayout->setRowStretch(i, 1);
    }
    
    m_mainLayout->setColumnStretch(0, 1);
    m_mainLayout->setColumnStretch(8, 1);
    
    const QStringList positions = m_formationPositionsMap[formation];
    
    for (const QString& pos : positions) {
        auto* widget = m_positionWidgets.value(pos);
        if (widget) {
            QPoint coords = getPositionCoordinates(pos);
            m_mainLayout->addWidget(widget, coords.y(), coords.x(), Qt::AlignCenter);
            widget->setVisible(true);
        }
    }
}

void LineupPitchView::clearPlayerFromOtherPositions(int playerId, const QString& currentPosition) {
    for (auto it = m_positionWidgets.constBegin(); it != m_positionWidgets.constEnd(); ++it) {
        if (it.value()->getPlayerId() == playerId && it.key() != currentPosition) {
            it.value()->clearPlayer();
        }
    }
}

void LineupPitchView::setPlayerAtPosition(int playerId, const QString& playerName, const QPixmap& playerImage, const QString& position) {
    if (!m_positionWidgets.contains(position)) {
        return;
    }
    
    clearPlayerFromOtherPositions(playerId, position);
    m_positionWidgets[position]->setPlayer(playerId, playerName, playerImage);
}

void LineupPitchView::setPlayerAtPosition(int playerId, const QString& position) {
    setPlayerAtPosition(playerId, QString::number(playerId), QPixmap(), position);
}

QStringList LineupPitchView::getPositionsForFormation(const QString& formation) const {
    if (m_formationPositionsMap.contains(formation)) {
        return m_formationPositionsMap[formation];
    }
    return QStringList();
}

QMap<QString, int> LineupPitchView::getPlayersPositions() const {
    QMap<QString, int> result;
    
    for (auto it = m_positionWidgets.constBegin(); it != m_positionWidgets.constEnd(); ++it) {
        if (it.value()->isVisible() && it.value()->hasPlayer()) {
            result[it.key()] = it.value()->getPlayerId();
        }
    }
    
    return result;
}

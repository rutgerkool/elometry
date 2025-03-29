#include "gui/components/widgets/PlayerPositionWidget.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QDrag>
#include <QtCore/QDataStream>

#include <algorithm>
#include <ranges>

PlayerPositionWidget::PlayerPositionWidget(QString positionName, QWidget* parent)
    : QLabel(parent)
    , m_positionName(std::move(positionName))
{
    setupInitialState();
    configureWithoutPlayer();
}

void PlayerPositionWidget::setupInitialState() {
    setMinimumSize(80, 110);
    setMaximumSize(80, 110);
    setAlignment(Qt::AlignCenter);
    setText(m_positionName);
    setAcceptDrops(true);
    setToolTip(tr("Drag & drop players here or click for player details"));
}

void PlayerPositionWidget::setPlayer(int playerId, const QString& playerName, const QPixmap& playerImage) {
    m_playerId = playerId;
    m_playerName = playerName;
    
    clearLayout();
    createPlayerLayout(playerImage);
    configureWithPlayer();
}

void PlayerPositionWidget::clearPlayer() {
    m_playerId = -1;
    m_playerName.clear();
    
    clearLayout();
    setText(m_positionName);
    setCursor(Qt::ArrowCursor);
    configureWithoutPlayer();
}

void PlayerPositionWidget::clearLayout() {
    if (QLayout* existingLayout = layout()) {
        while (QLayoutItem* item = existingLayout->takeAt(0)) {
            if (QWidget* widget = item->widget()) {
                delete widget;
            }
            delete item;
        }
        delete existingLayout;
    }
}

void PlayerPositionWidget::createPlayerLayout(const QPixmap& playerImage) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(2);
    
    auto* contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: transparent;");
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(2);
    
    if (!playerImage.isNull() && !playerImage.size().isEmpty()) {
        createPlayerWithImage(playerImage);
    } else {
        createPlayerWithInitials();
    }
    
    layout->addWidget(contentWidget);
    setLayout(layout);
}

void PlayerPositionWidget::createPlayerWithImage(const QPixmap& playerImage) {
    auto* contentWidget = findChild<QWidget*>();
    auto* contentLayout = qobject_cast<QVBoxLayout*>(contentWidget->layout());
    
    auto* imageLabel = new QLabel(this);
    imageLabel->setFixedSize(60, 60);
    imageLabel->setScaledContents(false);
    
    QPixmap circularPixmap = createCircularAvatar(playerImage);
    
    imageLabel->setPixmap(circularPixmap);
    imageLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    
    setupContentLayout(contentLayout);
}

QPixmap PlayerPositionWidget::createCircularAvatar(const QPixmap& playerImage) const {
    QPixmap scaledPixmap = playerImage.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    QPixmap circularPixmap(60, 60);
    circularPixmap.fill(Qt::transparent);
    
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    QPainterPath path;
    path.addEllipse(0, 0, 60, 60);
    painter.setClipPath(path);
    
    painter.drawPixmap(0, 0, 60, 60, scaledPixmap);
    
    return circularPixmap;
}

void PlayerPositionWidget::createPlayerWithInitials() {
    auto* contentWidget = findChild<QWidget*>();
    auto* contentLayout = qobject_cast<QVBoxLayout*>(contentWidget->layout());
    
    auto* initialsLabel = new QLabel(this);
    QString initials = generateInitials();
    
    initialsLabel->setText(initials);
    initialsLabel->setAlignment(Qt::AlignCenter);
    initialsLabel->setStyleSheet("color: white; font-weight: bold; font-size: 24px; background-color: transparent;");
    contentLayout->addWidget(initialsLabel, 0, Qt::AlignCenter);
    
    setupContentLayout(contentLayout);
}

void PlayerPositionWidget::setupContentLayout(QVBoxLayout* contentLayout) {
    auto* nameLabel = new QLabel(this);
    QString displayName = truncateNameForDisplay(m_playerName.toStdString());
    
    nameLabel->setText(displayName);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("color: white; font-weight: bold; font-size: 12px; background-color: transparent;");
    contentLayout->addWidget(nameLabel, 0, Qt::AlignCenter);
    
    auto* posLabel = new QLabel(this);
    posLabel->setText(m_positionName);
    posLabel->setAlignment(Qt::AlignCenter);
    posLabel->setStyleSheet("color: rgba(255, 255, 255, 0.7); font-size: 10px; background-color: transparent;");
    contentLayout->addWidget(posLabel, 0, Qt::AlignCenter);
}

QString PlayerPositionWidget::truncateNameForDisplay(std::string_view name) const {
    if (name.empty()) {
        return {};
    }
    
    auto parts = std::string_view(name).substr(0, name.size());
    size_t spacePos = parts.find(' ');
    
    QString displayName;
    if (spacePos != std::string_view::npos) {
        displayName = QString::fromUtf8(parts.substr(spacePos + 1).data(), parts.substr(spacePos + 1).size());
    } else {
        displayName = QString::fromUtf8(parts.data(), parts.size());
    }
    
    if (displayName.length() > 8) {
        displayName = displayName.left(6) + "..";
    }
    
    return displayName;
}

QString PlayerPositionWidget::generateInitials() const {
    if (m_playerName.isEmpty()) {
        return QString::number(m_playerId);
    }
    
    const QStringList nameParts = m_playerName.split(' ');
    if (nameParts.isEmpty()) {
        return QString::number(m_playerId);
    }
    
    if (nameParts.size() > 1) {
        return QString(nameParts.first().at(0)) + 
               QString(nameParts.last().at(0));
    }
    
    return m_playerName.left(2).toUpper();
}

void PlayerPositionWidget::configureWithPlayer() {
    setText("");
    setCursor(Qt::PointingHandCursor);
    
    setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                 "stop:0 #3498db, stop:1 #2980b9); "
                 "border-radius: 8px; "
                 "border: 1px solid #2471a3;");
    
    if (m_playerName.isEmpty()) {
        setToolTip(tr("Player #%1 - Click for details or drag to another position").arg(m_playerId));
    } else {
        setToolTip(tr("%1 - Click for details or drag to another position").arg(m_playerName));
    }
}

void PlayerPositionWidget::configureWithoutPlayer() {
    setStyleSheet("background-color: rgba(255, 255, 255, 0.85); "
                  "border-radius: 8px; "
                  "border: 1px dashed #ccc; "
                  "font-weight: bold; "
                  "font-size: 14px; "
                  "color: #333;");
    
    setToolTip(tr("Position: %1 - Drag a player here").arg(m_positionName));
}

void PlayerPositionWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_playerId > 0) {
        m_dragStartPosition = event->pos();
    }
    
    QLabel::mousePressEvent(event);
}

void PlayerPositionWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton) || m_playerId <= 0) {
        return;
    }
        
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }
    
    auto drag = std::make_unique<QDrag>(this);
    drag->setMimeData(createDragMimeData().release());
    
    QPixmap dragPixmap = createDragPixmap();
    drag->setPixmap(dragPixmap);
    drag->setHotSpot(event->pos());
    
    emit playerDragged(m_playerId, m_positionName);
    
    drag->exec(Qt::MoveAction);
}

std::unique_ptr<QMimeData> PlayerPositionWidget::createDragMimeData() const {
    auto mimeData = std::make_unique<QMimeData>();
    
    mimeData->setText(QString("%1|%2").arg(m_playerId).arg(m_positionName));
    
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << m_playerId;
    mimeData->setData("application/x-qabstractitemmodeldatalist", itemData);
    
    return mimeData;
}

QPixmap PlayerPositionWidget::createDragPixmap() const {
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    
    painter.setClipPath(path);
    const_cast<PlayerPositionWidget*>(this)->render(&painter);
    
    return pixmap;
}

void PlayerPositionWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && 
        m_playerId > 0 && 
        (event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        emit playerClicked(m_playerId);
    }
    
    QLabel::mouseReleaseEvent(event);
}

void PlayerPositionWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void PlayerPositionWidget::dropEvent(QDropEvent* event) {
    if (!event->mimeData()->hasText()) {
        return;
    }
    
    const QString mimeText = event->mimeData()->text();
    int droppedPlayerId = -1;
    QString fromPosition;
    
    if (mimeText.contains('|')) {
        const QStringList data = mimeText.split('|');
        droppedPlayerId = data[0].toInt();
        fromPosition = data[1];
    } else if (const int potentialId = mimeText.toInt(); potentialId > 0) {
        droppedPlayerId = potentialId;
        fromPosition = "UNKNOWN";
    } else {
        return;
    }
    
    emit playerDropped(droppedPlayerId, fromPosition, m_positionName);
    event->acceptProposedAction();
}

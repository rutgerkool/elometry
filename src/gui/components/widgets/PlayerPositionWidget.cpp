#include "gui/views/LineupPitchView.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QtWidgets/QListWidget>
#include <QDrag>

PlayerPositionWidget::PlayerPositionWidget(const QString& posName, QWidget *parent)
    : QLabel(parent)
    , positionName(posName)
    , playerId(-1)
{
    setMinimumSize(80, 110);
    setMaximumSize(80, 110);
    setAlignment(Qt::AlignCenter);
    setText(posName);
    
    configureWithoutPlayer();
    
    setAcceptDrops(true);
    setToolTip("Drag & drop players here or click for player details");
}

void PlayerPositionWidget::setPlayer(int id, const QString& name, const QPixmap& playerImage) {
    playerId = id;
    playerName = name;
    
    QLayout* existingLayout = layout();
    if (existingLayout) {
        QLayoutItem* item;
        while ((item = existingLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
        delete existingLayout;
    }
    
    createPlayerLayout(playerImage);
    configureWithPlayer();
}

void PlayerPositionWidget::createPlayerLayout(const QPixmap& playerImage) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(2);
    
    QWidget* contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: transparent;");
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
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
    QWidget* contentWidget = findChild<QWidget*>();
    QVBoxLayout* contentLayout = qobject_cast<QVBoxLayout*>(contentWidget->layout());
    
    QLabel* imageLabel = new QLabel(this);
    imageLabel->setFixedSize(60, 60);
    imageLabel->setScaledContents(false);
    
    QPixmap circularPixmap = createCircularAvatar(playerImage);
    
    imageLabel->setPixmap(circularPixmap);
    imageLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    
    setupContentWidget(contentWidget, contentLayout);
}

QPixmap PlayerPositionWidget::createCircularAvatar(const QPixmap& playerImage) {
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
    QWidget* contentWidget = findChild<QWidget*>();
    QVBoxLayout* contentLayout = qobject_cast<QVBoxLayout*>(contentWidget->layout());
    
    QLabel* initialsLabel = new QLabel(this);
    QString initials = generateInitials();
    
    initialsLabel->setText(initials);
    initialsLabel->setAlignment(Qt::AlignCenter);
    initialsLabel->setStyleSheet("color: white; font-weight: bold; font-size: 24px; background-color: transparent;");
    contentLayout->addWidget(initialsLabel, 0, Qt::AlignCenter);
    
    setupContentWidget(contentWidget, contentLayout);
}

void PlayerPositionWidget::setupContentWidget(QWidget* contentWidget, QVBoxLayout* contentLayout) {
    QLabel* nameLabel = new QLabel(this);
    QString displayName = truncateNameForDisplay(playerName);
    
    nameLabel->setText(displayName);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("color: white; font-weight: bold; font-size: 12px; background-color: transparent;");
    contentLayout->addWidget(nameLabel, 0, Qt::AlignCenter);
    
    QLabel* posLabel = new QLabel(this);
    posLabel->setText(positionName);
    posLabel->setAlignment(Qt::AlignCenter);
    posLabel->setStyleSheet("color: rgba(255, 255, 255, 0.7); font-size: 10px; background-color: transparent;");
    contentLayout->addWidget(posLabel, 0, Qt::AlignCenter);
}

QString PlayerPositionWidget::truncateNameForDisplay(const QString& name) const {
    QString displayName;
    QStringList nameParts = name.split(' ');
    
    if (nameParts.size() > 1) {
        displayName = nameParts.last();
    } else {
        displayName = name;
    }
    
    if (displayName.length() > 8) {
        displayName = displayName.left(6) + "..";
    }
    
    return displayName;
}

QString PlayerPositionWidget::generateInitials() const {
    if (playerName.isEmpty()) {
        return QString::number(playerId);
    }
    
    QStringList nameParts = playerName.split(' ');
    if (nameParts.isEmpty()) {
        return QString::number(playerId);
    }
    
    if (nameParts.size() > 1) {
        return QString(nameParts.first().at(0)) + 
               QString(nameParts.last().at(0));
    } else {
        return playerName.left(2).toUpper();
    }
}

void PlayerPositionWidget::configureWithPlayer() {
    setText("");
    setCursor(Qt::PointingHandCursor);
    
    setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                 "stop:0 #3498db, stop:1 #2980b9); "
                 "border-radius: 8px; "
                 "border: 1px solid #2471a3;");
    
    if (playerName.isEmpty()) {
        setToolTip(QString("Player #%1 - Click for details or drag to another position").arg(playerId));
    } else {
        setToolTip(QString("%1 - Click for details or drag to another position").arg(playerName));
    }
}

void PlayerPositionWidget::configureWithoutPlayer() {
    setStyleSheet("background-color: rgba(255, 255, 255, 0.85); "
                  "border-radius: 8px; "
                  "border: 1px dashed #ccc; "
                  "font-weight: bold; "
                  "font-size: 14px; "
                  "color: #333;");
    
    setToolTip(QString("Position: %1 - Drag a player here").arg(positionName));
}

void PlayerPositionWidget::clearPlayer() {
    playerId = -1;
    playerName = "";
    
    QLayout* existingLayout = layout();
    if (existingLayout) {
        QLayoutItem* item;
        while ((item = existingLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
        delete existingLayout;
    }
    
    setPixmap(QPixmap());
    setText(positionName);
    setCursor(Qt::ArrowCursor);
    
    configureWithoutPlayer();
}

void PlayerPositionWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton) || playerId <= 0)
        return;
        
    if ((event->pos() - dragStartPosition).manhattanLength() < 10)
        return;
    
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = setupDragMimeData();
    drag->setMimeData(mimeData);
    
    QPixmap dragPixmap = createDragPixmap();
    drag->setPixmap(dragPixmap);
    drag->setHotSpot(event->pos());
    
    emit playerDragged(playerId, positionName);
    
    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
}

QMimeData* PlayerPositionWidget::setupDragMimeData() {
    QMimeData* mimeData = new QMimeData;
    
    mimeData->setText(QString("%1|%2").arg(playerId).arg(positionName));
    
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << playerId;
    mimeData->setData("application/x-qabstractitemmodeldatalist", itemData);
    
    return mimeData;
}

QPixmap PlayerPositionWidget::createDragPixmap() {
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    
    painter.setClipPath(path);
    render(&painter);
    
    return pixmap;
}

void PlayerPositionWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && playerId > 0) {
        dragStartPosition = event->pos();
    }
    
    QLabel::mousePressEvent(event);
}

void PlayerPositionWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && playerId > 0) {
        if ((event->pos() - dragStartPosition).manhattanLength() < 10) {
            emit playerClicked(playerId);
        }
    }
    
    QLabel::mouseReleaseEvent(event);
}

void PlayerPositionWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void PlayerPositionWidget::dropEvent(QDropEvent* event) {
    if (!event->mimeData()->hasText())
        return;
    
    QString mimeText = event->mimeData()->text();
    int droppedPlayerId = -1;
    QString fromPosition;
    
    if (mimeText.contains("|")) {
        QStringList data = mimeText.split("|");
        droppedPlayerId = data[0].toInt();
        fromPosition = data[1];
    } else if (mimeText.toInt() > 0) {
        droppedPlayerId = mimeText.toInt();
        fromPosition = "UNKNOWN";
    } else {
        return;
    }
    
    emit playerDropped(droppedPlayerId, fromPosition, positionName);
    event->acceptProposedAction();
}

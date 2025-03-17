#include "gui/components/widgets/DraggableListWidget.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGroupBox>
#include <QTimer>
#include <QFile>
#include <set>

QMimeData* DraggableListWidget::mimeData(const QList<QListWidgetItem*> &items) const {
    QMimeData* mimeData = QListWidget::mimeData(items);
    if (items.count() == 1) {
        int playerId = items.first()->data(Qt::UserRole).toInt();
        mimeData->setText(QString("%1|%2").arg(playerId).arg(listType));
    }
    return mimeData;
}

void DraggableListWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
        dragStarted = false;
    }
    QListWidget::mousePressEvent(event);
}

void DraggableListWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton))
        return;
        
    if ((event->pos() - dragStartPosition).manhattanLength() >= QApplication::startDragDistance()) {
        dragStarted = true;
    }
    
    QListWidget::mouseMoveEvent(event);
}

void DraggableListWidget::mouseReleaseEvent(QMouseEvent* event) {
    dragStarted = false;
    QListWidget::mouseReleaseEvent(event);
}

void DraggableListWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText()) {
        QString text = event->mimeData()->text();

        if (text.contains("|")) {
            QStringList parts = text.split("|");
            int playerId = parts[0].toInt();
            QString source = parts[1];
            
            if (source != "BENCH" && source != "RESERVE") {
                event->setDropAction(Qt::MoveAction);
                event->accept();
                return;
            }
        }
        event->acceptProposedAction();
        return;
    }
    QListWidget::dragEnterEvent(event);
}

void DraggableListWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasText()) {
        QString text = event->mimeData()->text();
        
        if (text.contains("|")) {
            QStringList parts = text.split("|");
            QString source = parts[1];
            
            if (source != "BENCH" && source != "RESERVE") {
                event->setDropAction(Qt::MoveAction);
                event->accept();
                return;
            }
        }
        event->acceptProposedAction();
        return;
    }
    QListWidget::dragMoveEvent(event);
}

void DraggableListWidget::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasText()) {
        QString data = event->mimeData()->text();
        
        if (data.contains("|")) {
            QStringList parts = data.split("|");
            int playerId = parts[0].toInt();
            QString source = parts[1];
            
            if (source != "BENCH" && source != "RESERVE") {
                emit fieldPlayerDropped(playerId, source, listType);
                event->setDropAction(Qt::MoveAction);
                event->accept();
                return;
            }
            else if (source != listType) {
                event->acceptProposedAction();
                return;
            }
            else if (source == listType) {
                event->ignore();
                return;
            }
        }
    }
    QListWidget::dropEvent(event);
}

void DraggableListWidget::enterEvent(QEnterEvent* event) {
    if (count() > 0) {
        setCursor(Qt::PointingHandCursor);
    }
    QListWidget::enterEvent(event);
}

void DraggableListWidget::leaveEvent(QEvent* event) {
    setCursor(Qt::ArrowCursor);
    QListWidget::leaveEvent(event);
}

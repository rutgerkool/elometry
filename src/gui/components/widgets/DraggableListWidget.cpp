#include "gui/components/widgets/DraggableListWidget.h"
#include <QtCore/QMimeData>
#include <QtWidgets/QApplication>

DraggableListWidget::DraggableListWidget(QString listType, QWidget* parent)
    : QListWidget(parent)
    , m_listType(std::move(listType))
{
}

QMimeData* DraggableListWidget::mimeData(const QList<QListWidgetItem*>& items) const {
    QMimeData* mimeData = QListWidget::mimeData(items);
    
    if (items.size() == 1) {
        const int playerId = items.first()->data(Qt::UserRole).toInt();
        mimeData->setText(QString("%1|%2").arg(playerId).arg(m_listType));
    }
    
    return mimeData;
}

void DraggableListWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
        m_dragStarted = false;
    }
    
    QListWidget::mousePressEvent(event);
}

void DraggableListWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    
    if ((event->pos() - m_dragStartPosition).manhattanLength() >= QApplication::startDragDistance()) {
        m_dragStarted = true;
    }
    
    QListWidget::mouseMoveEvent(event);
}

void DraggableListWidget::mouseReleaseEvent(QMouseEvent* event) {
    m_dragStarted = false;
    QListWidget::mouseReleaseEvent(event);
}

void DraggableListWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (!hasDragData(event->mimeData())) {
        QListWidget::dragEnterEvent(event);
        return;
    }
    
    const QString source = extractSourceFromMimeData(event->mimeData());
    
    if (isFieldSource(source)) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else if (!isSameListType(source)) {
        event->acceptProposedAction();
    } else {
        QListWidget::dragEnterEvent(event);
    }
}

void DraggableListWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (!hasDragData(event->mimeData())) {
        QListWidget::dragMoveEvent(event);
        return;
    }
    
    const QString source = extractSourceFromMimeData(event->mimeData());
    
    if (isFieldSource(source)) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else if (!isSameListType(source)) {
        event->acceptProposedAction();
    } else {
        QListWidget::dragMoveEvent(event);
    }
}

void DraggableListWidget::dropEvent(QDropEvent* event) {
    if (!hasDragData(event->mimeData())) {
        QListWidget::dropEvent(event);
        return;
    }
    
    const QString source = extractSourceFromMimeData(event->mimeData());
    
    if (isFieldSource(source)) {
        handleDragFromField(event, source);
    } else if (!isSameListType(source)) {
        handleDragFromDifferentList(event);
    } else {
        QListWidget::dropEvent(event);
    }
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

bool DraggableListWidget::isFieldSource(const QString& source) const {
    return source != "BENCH" && source != "RESERVE";
}

bool DraggableListWidget::isSameListType(const QString& source) const {
    return source == m_listType;
}

bool DraggableListWidget::hasDragData(const QMimeData* mimeData) const {
    return mimeData && mimeData->hasText() && mimeData->text().contains('|');
}

QString DraggableListWidget::extractSourceFromMimeData(const QMimeData* mimeData) const {
    return mimeData->text().split('|').at(1);
}

void DraggableListWidget::handleDragFromField(QDropEvent* event, const QString& source) {
    const QStringList parts = event->mimeData()->text().split('|');
    const int playerId = parts.first().toInt();
    
    emit fieldPlayerDropped(playerId, source, m_listType);
    event->setDropAction(Qt::MoveAction);
    event->accept();
}

void DraggableListWidget::handleDragFromDifferentList(QDropEvent* event) {
    event->acceptProposedAction();
}

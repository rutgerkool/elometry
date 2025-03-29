#ifndef DRAGGABLELISTWIDGET_H
#define DRAGGABLELISTWIDGET_H

#include <QtWidgets/QListWidget>
#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtGui/QMouseEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QEnterEvent>

class DraggableListWidget final : public QListWidget {
    Q_OBJECT
    
    public:
        explicit DraggableListWidget(QString listType, QWidget* parent = nullptr);
        ~DraggableListWidget() override = default;
        
        DraggableListWidget(const DraggableListWidget&) = delete;
        DraggableListWidget& operator=(const DraggableListWidget&) = delete;
        DraggableListWidget(DraggableListWidget&&) = delete;
        DraggableListWidget& operator=(DraggableListWidget&&) = delete;
        
    signals:
        void fieldPlayerDropped(int playerId, const QString& fromPosition, const QString& toListType);
        
    protected:
        [[nodiscard]] QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragMoveEvent(QDragMoveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void enterEvent(QEnterEvent* event) override;
        void leaveEvent(QEvent* event) override;
        
    private:
        bool isFieldSource(const QString& source) const;
        bool isSameListType(const QString& source) const;
        bool hasDragData(const QMimeData* mimeData) const;
        QString extractSourceFromMimeData(const QMimeData* mimeData) const;
        void handleDragFromField(QDropEvent* event, const QString& source);
        void handleDragFromDifferentList(QDropEvent* event);
        
        const QString m_listType;
        QPoint m_dragStartPosition;
        bool m_dragStarted = false;
};

#endif

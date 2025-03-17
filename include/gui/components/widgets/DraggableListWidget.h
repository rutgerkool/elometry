#ifndef DRAGGABLELISTWIDGET_H
#define DRAGGABLELISTWIDGET_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtGui/QStandardItemModel>
#include <QtCore/QMap>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QApplication>
#include <QEnterEvent>
#include "services/TeamManager.h"
#include "gui/views/LineupPitchView.h"
#include "gui/components/dialogs/LineupCreationDialog.h"

class DraggableListWidget : public QListWidget {
    Q_OBJECT
    
    public:
        explicit DraggableListWidget(const QString& listType, QWidget* parent = nullptr)
            : QListWidget(parent), listType(listType), dragStarted(false) {}
        
    signals:
        void fieldPlayerDropped(int playerId, const QString& fromPosition, const QString& toListType);
        
    protected:
        virtual QMimeData* mimeData(const QList<QListWidgetItem*> &items) const override;
        virtual void mousePressEvent(QMouseEvent* event) override;
        virtual void mouseMoveEvent(QMouseEvent* event) override;
        virtual void mouseReleaseEvent(QMouseEvent* event) override;
        virtual void dragEnterEvent(QDragEnterEvent* event) override;
        virtual void dragMoveEvent(QDragMoveEvent* event) override;
        virtual void dropEvent(QDropEvent* event) override;
        void enterEvent(QEnterEvent* event) override;
        void leaveEvent(QEvent* event) override;
        
    private:
        QString listType;
        QPoint dragStartPosition;
        bool dragStarted;
};

#endif

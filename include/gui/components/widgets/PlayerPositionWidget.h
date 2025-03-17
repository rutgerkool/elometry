#ifndef PLAYERPOSITIONWIDGET_H
#define PLAYERPOSITIONWIDGET_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtCore/QMap>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QMimeData>
#include "services/TeamManager.h"

class PlayerPositionWidget : public QLabel {
    Q_OBJECT
    
    public:
        explicit PlayerPositionWidget(const QString& positionName, QWidget *parent = nullptr);
        
        void setPlayer(int playerId, const QString& playerName, const QPixmap& playerImage = QPixmap());
        void clearPlayer();
        
        bool hasPlayer() const { return playerId > 0; }
        int getPlayerId() const { return playerId; }
        QString getPositionName() const { return positionName; }
        
    signals:
        void playerDragged(int playerId, const QString& fromPosition);
        void playerDropped(int playerId, const QString& fromPosition, const QString& toPosition);
        void playerClicked(int playerId);
        
    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        
    private:
        void createPlayerLayout(const QPixmap& playerImage);
        void createPlayerWithImage(const QPixmap& playerImage);
        void createPlayerWithInitials();
        QString truncateNameForDisplay(const QString& name) const;
        QString generateInitials() const;
        void setupContentWidget(QVBoxLayout* contentLayout);
        void configureWithPlayer();
        void configureWithoutPlayer();
        QPixmap createCircularAvatar(const QPixmap& playerImage);
        QMimeData* setupDragMimeData();
        QPixmap createDragPixmap();
        
        QString positionName;
        int playerId;
        QString playerName;
        QPoint dragStartPosition;
};

#endif

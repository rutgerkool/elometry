#ifndef LINEUPPITCHVIEW_H
#define LINEUPPITCHVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtCore/QMap>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QMimeData>
#include "services/TeamManager.h"
#include "gui/components/widgets/PlayerPositionWidget.h"

class LineupPitchView : public QWidget {
    Q_OBJECT

    public:
        explicit LineupPitchView(QWidget *parent = nullptr);
        void setFormation(const QString& formation);
        void clearPosition(const QString& position);
        void clearPositions();
        void setPlayerAtPosition(int playerId, const QString& position);
        QStringList getPositionsForFormation(const QString& formation);
        QMap<QString, int> getPlayersPositions() const;
        void setPlayerAtPosition(int playerId, const QString& playerName, const QPixmap& playerImage, const QString& position);
        
    signals:
        void playerDragDropped(int playerId, const QString& fromPosition, const QString& toPosition);
        void playerClicked(int playerId);
        
    protected:
        void paintEvent(QPaintEvent* event) override;
        
    private:
        void setupFormationPositions(const QString& formation);
        void createPlayerWidgets();
        QPoint getPositionCoordinates(const QString& position);
        QMap<QString, QPoint> getFormationCoordinates(const QString& formation) const;
        QMap<QString, QPoint> getFormation433Coordinates() const;
        QMap<QString, QPoint> getFormation442Coordinates() const;
        QMap<QString, QPoint> getFormation532Coordinates() const;
        QMap<QString, QPoint> getFormation352Coordinates() const;
        QMap<QString, QPoint> getFormation4231Coordinates() const;
        
        void drawPitch(QPainter& painter);
        void drawCenterLine(QPainter& painter, const QRect& fieldRect);
        void drawCenterCircle(QPainter& painter, const QRect& fieldRect);
        void drawPenaltyAreas(QPainter& painter, const QRect& fieldRect);
        void drawGoals(QPainter& painter, const QRect& fieldRect);
        void drawGridLines(QPainter& painter, const QRect& fieldRect);
        
        void initializeFormations();
        QStringList getAllPositions() const;
        void setupPlayerPositionWidget(PlayerPositionWidget* posWidget);
        void clearPlayerFromOtherPositions(int playerId, const QString& currentPosition);
        
        QString currentFormation;
        QGridLayout* mainLayout;
        QMap<QString, PlayerPositionWidget*> positionWidgets;
        QMap<QString, QStringList> formationPositionsMap;
};

#endif

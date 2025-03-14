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
#include "gui/components/PlayerPositionWidget.h"

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
        void setupPositions();
        void setupFormationPositions(const QString& formation);
        void createPlayerWidgets();
        QPoint getPositionCoordinates(const QString& position);
        
        QString currentFormation;
        QGridLayout* mainLayout;
        QMap<QString, PlayerPositionWidget*> positionWidgets;
        QMap<QString, QStringList> formationPositionsMap;
};

#endif

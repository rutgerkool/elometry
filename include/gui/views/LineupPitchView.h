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
#include <memory>
#include <optional>
#include <span>
#include "services/TeamManager.h"
#include "gui/components/widgets/PlayerPositionWidget.h"

class LineupPitchView final : public QWidget {
    Q_OBJECT

    public:
        explicit LineupPitchView(QWidget* parent = nullptr);
        ~LineupPitchView() override;

        LineupPitchView(const LineupPitchView&) = delete;
        LineupPitchView& operator=(const LineupPitchView&) = delete;
        LineupPitchView(LineupPitchView&&) = delete;
        LineupPitchView& operator=(LineupPitchView&&) = delete;

        void setFormation(const QString& formation);
        void clearPosition(const QString& position);
        void clearPositions();
        void setPlayerAtPosition(int playerId, const QString& position);
        void setPlayerAtPosition(int playerId, const QString& playerName, const QPixmap& playerImage, const QString& position);
        
        [[nodiscard]] QStringList getPositionsForFormation(const QString& formation) const;
        [[nodiscard]] QMap<QString, int> getPlayersPositions() const;
        
    signals:
        void playerDragDropped(int playerId, const QString& fromPosition, const QString& toPosition);
        void playerClicked(int playerId);
        
    protected:
        void paintEvent(QPaintEvent* event) override;
        
    private:
        void setupFormationPositions(const QString& formation);
        void createPlayerWidgets();
        
        [[nodiscard]] QPoint getPositionCoordinates(const QString& position) const;
        [[nodiscard]] QMap<QString, QPoint> getFormationCoordinates(const QString& formation) const;
        [[nodiscard]] QMap<QString, QPoint> getFormation433Coordinates() const;
        [[nodiscard]] QMap<QString, QPoint> getFormation442Coordinates() const;
        [[nodiscard]] QMap<QString, QPoint> getFormation532Coordinates() const;
        [[nodiscard]] QMap<QString, QPoint> getFormation352Coordinates() const;
        [[nodiscard]] QMap<QString, QPoint> getFormation4231Coordinates() const;
        
        void drawPitch(QPainter& painter);
        void drawCenterLine(QPainter& painter, const QRect& fieldRect);
        void drawCenterCircle(QPainter& painter, const QRect& fieldRect);
        void drawPenaltyAreas(QPainter& painter, const QRect& fieldRect);
        void drawGoals(QPainter& painter, const QRect& fieldRect);
        void drawGridLines(QPainter& painter, const QRect& fieldRect);
        
        void initializeFormations();
        [[nodiscard]] QStringList getAllPositions() const;
        void setupPlayerPositionWidget(PlayerPositionWidget* posWidget);
        void clearPlayerFromOtherPositions(int playerId, const QString& currentPosition);
        
        QString m_currentFormation;
        QGridLayout* m_mainLayout;
        QMap<QString, PlayerPositionWidget*> m_positionWidgets;
        QMap<QString, QStringList> m_formationPositionsMap;
};

#endif

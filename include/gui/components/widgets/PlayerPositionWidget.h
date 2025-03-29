#ifndef PLAYERPOSITIONWIDGET_H
#define PLAYERPOSITIONWIDGET_H

#include <QtWidgets/QLabel>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QMimeData>

class QVBoxLayout;

#include <optional>
#include <string_view>

class PlayerPositionWidget final : public QLabel {
    Q_OBJECT
    
    public:
        explicit PlayerPositionWidget(QString positionName, QWidget* parent = nullptr);
        ~PlayerPositionWidget() override = default;
        
        PlayerPositionWidget(const PlayerPositionWidget&) = delete;
        PlayerPositionWidget& operator=(const PlayerPositionWidget&) = delete;
        PlayerPositionWidget(PlayerPositionWidget&&) = delete;
        PlayerPositionWidget& operator=(PlayerPositionWidget&&) = delete;
        
        void setPlayer(int playerId, const QString& playerName, const QPixmap& playerImage = {});
        void clearPlayer();
        
        [[nodiscard]] bool hasPlayer() const noexcept { return m_playerId > 0; }
        [[nodiscard]] int getPlayerId() const noexcept { return m_playerId; }
        [[nodiscard]] const QString& getPositionName() const noexcept { return m_positionName; }
        
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
        void setupInitialState();
        void createPlayerLayout(const QPixmap& playerImage);
        void createPlayerWithImage(const QPixmap& playerImage);
        void createPlayerWithInitials();
        void setupContentLayout(QVBoxLayout* contentLayout);
        
        void configureWithPlayer();
        void configureWithoutPlayer();
        
        [[nodiscard]] std::unique_ptr<QMimeData> createDragMimeData() const;
        [[nodiscard]] QPixmap createDragPixmap() const;
        
        [[nodiscard]] QString truncateNameForDisplay(std::string_view name) const;
        [[nodiscard]] QString generateInitials() const;
        [[nodiscard]] QPixmap createCircularAvatar(const QPixmap& playerImage) const;
        void clearLayout();
        
        QString m_positionName;
        int m_playerId = -1;
        QString m_playerName;
        QPoint m_dragStartPosition;
};

#endif

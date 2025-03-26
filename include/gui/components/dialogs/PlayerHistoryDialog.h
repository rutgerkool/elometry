#ifndef PLAYERHISTORYDIALOG_H
#define PLAYERHISTORYDIALOG_H

#include "services/RatingManager.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QChart>
#include <QScatterSeries>
#include <QStandardItemModel>

#include <memory>
#include <span>
#include <optional>

class PlayerHistoryDialog final : public QDialog {
    Q_OBJECT

    public:
        explicit PlayerHistoryDialog(const RatingManager& ratingManager, int playerId, QWidget* parent = nullptr);
        ~PlayerHistoryDialog() override = default;

        PlayerHistoryDialog(const PlayerHistoryDialog&) = delete;
        PlayerHistoryDialog& operator=(const PlayerHistoryDialog&) = delete;
        PlayerHistoryDialog(PlayerHistoryDialog&&) = delete;
        PlayerHistoryDialog& operator=(PlayerHistoryDialog&&) = delete;

    private slots:
        void animateContent();

    private:
        void initializeUi();
        void loadPlayerData();
        void setupChartView();
        void setupTableView();
        void setupAnimations();
        void centerOnParent();
        void setupChart();
        void setupTable();
        void configureChartAppearance();
        void createChartSeries();
        void createChartAxes();
        
        [[nodiscard]] std::pair<double, double> calculateRatingRange() const;
        [[nodiscard]] double calculatePadding(double minRating, double maxRating) const;
        
        void populateTableWithHistory();
        void configureTableColumns();

        const RatingManager& m_ratingManager;
        const int m_playerId;
        std::optional<Player> m_player;
        std::vector<RatingChange> m_playerHistory;
        std::vector<std::pair<int, double>> m_ratingProgression;

        QVBoxLayout* m_mainLayout{nullptr};
        QLabel* m_titleLabel{nullptr};
        QLabel* m_playerInfoLabel{nullptr};
        QTableView* m_historyTable{nullptr};
        QStandardItemModel* m_tableModel{nullptr};
        QChartView* m_chartView{nullptr};
        QChart* m_chart{nullptr};
        QPushButton* m_closeButton{nullptr};

        QGraphicsOpacityEffect* m_chartOpacityEffect{nullptr};
        QPropertyAnimation* m_chartAnimation{nullptr};
        QGraphicsOpacityEffect* m_tableOpacityEffect{nullptr};
        QPropertyAnimation* m_tableAnimation{nullptr};
        QParallelAnimationGroup* m_animationGroup{nullptr};
};

#endif

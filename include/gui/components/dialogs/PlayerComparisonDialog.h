#ifndef PLAYERCOMPARISONDIALOG_H
#define PLAYERCOMPARISONDIALOG_H

#include <QDialog>
#include <QColor>
#include <QString>
#include <QPointF>
#include <vector>
#include <span>
#include <optional>

#include "services/RatingManager.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QTableView;
class QChart;
class QChartView;
class QLineSeries;
class QValueAxis;
class QStandardItemModel;

class PlayerComparisonDialog final : public QDialog {
    Q_OBJECT

    public:
        PlayerComparisonDialog(RatingManager& ratingManager, int playerId1, int playerId2, QWidget* parent = nullptr);
        ~PlayerComparisonDialog() override = default;

        PlayerComparisonDialog(const PlayerComparisonDialog&) = delete;
        PlayerComparisonDialog& operator=(const PlayerComparisonDialog&) = delete;
        PlayerComparisonDialog(PlayerComparisonDialog&&) = delete;
        PlayerComparisonDialog& operator=(PlayerComparisonDialog&&) = delete;

    private:
        void initializeUI();
        void createPlayerInfoSection();
        void createChartSection();
        void createTableSection();
        void createButtonSection();
        void centerOnParent(QWidget* parent);

        void loadPlayerData();
        [[nodiscard]] bool validatePlayerData() const;
        
        void setupChart();
        void populateChart();
        void configureSeries(QLineSeries* series1, QLineSeries* series2);
        void configureAxes(QValueAxis* xAxis, QValueAxis* yAxis, double minRating, double maxRating);
        [[nodiscard]] double calculateRatingPadding(double minRating, double maxRating) const;
        
        void setupPlayerTable(QTableView* tableView, std::span<const RatingChange> playerHistory);
        [[nodiscard]] QStandardItemModel* createTableModel(std::span<const RatingChange> playerHistory) const;

        [[nodiscard]] QLabel* createPlayerNameLabel(const QString& text, const QColor& color) const;
        [[nodiscard]] QLabel* createPlayerInfoLabel(const QString& text, const QColor& color) const;
        [[nodiscard]] QHBoxLayout* createPlayerInfoLayout() const;
        
        void populateRatingData(QLineSeries* series, const std::vector<std::pair<int, double>>& progression, 
                            double& minRating, double& maxRating);
        std::pair<double, double> calculateRatingRange(QLineSeries* series1, QLineSeries* series2);

        const RatingManager& m_ratingManager;
        const int m_playerId1;
        const int m_playerId2;
        
        std::optional<Player> m_player1;
        std::optional<Player> m_player2;
        std::vector<RatingChange> m_player1History;
        std::vector<RatingChange> m_player2History;
        std::vector<std::pair<int, double>> m_rating1Progression;
        std::vector<std::pair<int, double>> m_rating2Progression;

        QVBoxLayout* m_mainLayout{nullptr};
        QChartView* m_chartView{nullptr};
        QChart* m_chart{nullptr};
        QPushButton* m_closeButton{nullptr};
        
        static constexpr QColor PLAYER1_COLOR{0x0c, 0x7b, 0xb3};
        static constexpr QColor PLAYER2_COLOR{0x2e, 0xa0, 0x43};
};

#endif

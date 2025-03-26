#ifndef PLAYERSELECTDIALOG_H
#define PLAYERSELECTDIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QTableView>
#include "gui/models/PlayerSelectModel.h"
#include "services/TeamManager.h"
#include <vector>
#include <memory>
#include <optional>
#include <set>

class Team;
class Player;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;
class QDialogButtonBox;
class QScrollBar;
class QVBoxLayout;
class QHBoxLayout;
class QItemSelection;

class PlayerSelectDialog final : public QDialog {
    Q_OBJECT

    public:
        explicit PlayerSelectDialog(TeamManager& teamManager, Team* currentTeam = nullptr, QWidget* parent = nullptr);
        ~PlayerSelectDialog() override = default;
        
        PlayerSelectDialog(const PlayerSelectDialog&) = delete;
        PlayerSelectDialog& operator=(const PlayerSelectDialog&) = delete;
        PlayerSelectDialog(PlayerSelectDialog&&) = delete;
        PlayerSelectDialog& operator=(PlayerSelectDialog&&) = delete;
        
        [[nodiscard]] std::vector<Player> getSelectedPlayers() const;

    private slots:
        void searchPlayers();
        void positionFilterChanged();
        void clearFilters();
        void toggleSelection();
        void updateSelectionInfo();
        void sortPlayerList(int column, Qt::SortOrder order);
        void checkScrollPosition();
        void loadMorePlayersIfNeeded();

    private:
        void setupUi();
        void setupTitleAndLabels();
        void setupSearchControls();
        void setupTableView();
        void setupButtons();
        void setupConnections();
        void setupScrollConnections();
        void setupTableConnections();
        void initializePositionFilter();
        void updatePlayerList();
        void configureTableColumns();
        void selectCurrentTeamPlayers();
        void createPlayerSelectModel();
        [[nodiscard]] QHBoxLayout* createSearchLayout();

        TeamManager& m_teamManager;
        Team* m_currentTeam;
        std::unique_ptr<PlayerSelectModel> m_playersModel;

        QTableView* m_playersTable{nullptr};
        QLineEdit* m_searchInput{nullptr};
        QComboBox* m_positionFilter{nullptr};
        QPushButton* m_clearButton{nullptr};
        QDialogButtonBox* m_buttonBox{nullptr};
        QLabel* m_selectionInfoLabel{nullptr};
        QLabel* m_titleLabel{nullptr};
        QLabel* m_instructionLabel{nullptr};
        QLabel* m_loadingIndicator{nullptr};

        int m_currentOffset{0};
        static constexpr int m_pageSize{20};
        bool m_isLoading{false};
};

#endif

#ifndef TEAMSELECTDIALOG_H
#define TEAMSELECTDIALOG_H

#include <QtWidgets/QDialog>
#include <set>
#include <span>
#include <vector>

class QListView;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QDialogButtonBox;
class TeamManager;
class TeamSelectModel;

class TeamSelectDialog final : public QDialog {
    Q_OBJECT

    public:
        explicit TeamSelectDialog(TeamManager& teamManager, int playerId, QWidget* parent = nullptr);
        ~TeamSelectDialog() override = default;

        TeamSelectDialog(const TeamSelectDialog&) = delete;
        TeamSelectDialog& operator=(const TeamSelectDialog&) = delete;
        TeamSelectDialog(TeamSelectDialog&&) = delete;
        TeamSelectDialog& operator=(TeamSelectDialog&&) = delete;

        [[nodiscard]] std::vector<int> getSelectedTeamIds() const;

    private slots:
        void filterTeams(const QString& filter);
        void clearFilter();
        void toggleSelection(const QModelIndex& index);
        void updateSelectionInfo();

    private:
        void setupUi();
        void setupConnections();
        void loadTeams();
        
        void createHeaderSection();
        void createSearchSection();
        void createTeamsListSection();
        void createButtonSection();
        
        [[nodiscard]] QString formatSelectionText(int selectedCount, int newCount, int removedCount) const;
        [[nodiscard]] std::pair<int, int> calculateTeamChanges() const;

        TeamManager& m_teamManager;
        const int m_playerId;
        TeamSelectModel* m_teamsModel{nullptr};
        std::set<int> m_initialTeamIds;

        QLabel* m_titleLabel{nullptr};
        QLabel* m_selectionInfoLabel{nullptr};
        QLabel* m_instructionLabel{nullptr};
        QLabel* m_noTeamsLabel{nullptr};
        
        QLineEdit* m_searchInput{nullptr};
        QPushButton* m_clearButton{nullptr};
        
        QListView* m_teamsList{nullptr};
        QDialogButtonBox* m_buttonBox{nullptr};
};

#endif

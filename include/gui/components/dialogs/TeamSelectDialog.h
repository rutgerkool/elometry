#ifndef TEAMSELECTDIALOG_H
#define TEAMSELECTDIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QListView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QScrollBar>
#include <QtCore/QSet>
#include "services/TeamManager.h"
#include "models/PlayerRating.h"

class TeamSelectModel;

class TeamSelectDialog : public QDialog {
    Q_OBJECT

    public:
        explicit TeamSelectDialog(TeamManager& teamManager, int playerId, QWidget *parent = nullptr);
        std::vector<int> getSelectedTeamIds() const;

    private slots:
        void searchTeams(const QString& text);
        void clearFilter();
        void toggleSelection(const QModelIndex &index);
        void updateSelectionInfo();
        QString buildSelectionInfoText(int count, int newCount, int removedCount);

    private:
        void setupUi();
        void setupTitleAndLabels();
        void setupSearchControls();
        void setupListView();
        void setupButtons();
        void setupConnections();
        void updateTeamList();
        void handleEmptyTeamList();
        void handleNonEmptyTeamList(const std::vector<Team>& allTeams);
        void updateInitialTeamIds();
        QHBoxLayout* createSearchLayout();

        TeamManager& teamManager;
        int playerId;
        TeamSelectModel* teamsModel;
        QSet<int> initialTeamIds;

        QListView* teamsList;
        QLineEdit* searchInput;
        QPushButton* clearButton;
        QDialogButtonBox* buttonBox;
        QLabel* selectionInfoLabel;
        QLabel* titleLabel;
        QLabel* instructionLabel;
        QLabel* noTeamsLabel;
};

#endif

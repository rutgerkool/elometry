#ifndef LINEUPCREATEDIALOG_H
#define LINEUPCREATEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include "services/TeamManager.h"

class LineupCreationDialog : public QDialog {
    Q_OBJECT

    public:
        explicit LineupCreationDialog(TeamManager& teamManager, QWidget *parent = nullptr);

        int getSelectedFormationId() const;
        QString getLineupName() const;

    private:
        void setupUi();
        void setupConnections();

        TeamManager& teamManager;
        QComboBox* formationComboBox;
        QLineEdit* lineupNameInput;
        QPushButton* createButton;
        QPushButton* cancelButton;
};

#endif

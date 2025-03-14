#ifndef LINEUPCREATEDIALOG_H
#define LINEUPCREATEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include "services/TeamManager.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class LineupCreationDialog : public QDialog {
    Q_OBJECT

    public:
        explicit LineupCreationDialog(TeamManager& teamManager, QWidget *parent = nullptr);
        ~LineupCreationDialog() override = default;

        int getSelectedFormationId() const;
        QString getLineupName() const;

    private:
        void setupUi();
        void setupFormationSection(QVBoxLayout* mainLayout);
        void setupNameSection(QVBoxLayout* mainLayout);
        void setupButtonSection(QVBoxLayout* mainLayout);
        void setupConnections();
        void populateFormations();
        bool validateInput() const;

        TeamManager& teamManager;
        
        QComboBox* formationComboBox{nullptr};
        QLineEdit* lineupNameInput{nullptr};
        QPushButton* createButton{nullptr};
        QPushButton* cancelButton{nullptr};
};

#endif

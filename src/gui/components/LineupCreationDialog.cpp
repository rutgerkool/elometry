#include "gui/components/LineupCreationDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

LineupCreationDialog::LineupCreationDialog(TeamManager& tm, QWidget *parent)
    : QDialog(parent)
    , teamManager(tm)
{
    setWindowTitle("Create New Lineup");
    setupUi();
    setupConnections();
    setModal(true);
}

void LineupCreationDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    QHBoxLayout* formationLayout = new QHBoxLayout();
    QLabel* formationLabel = new QLabel("Formation:", this);
    formationComboBox = new QComboBox(this);
    formationComboBox->setMinimumWidth(200);

    std::vector<Formation> formations = teamManager.getAllFormations();
    for (const auto& formation : formations) {
        formationComboBox->addItem(
            QString::fromStdString(formation.name), 
            formation.id
        );
    }

    formationLayout->addWidget(formationLabel);
    formationLayout->addWidget(formationComboBox);
    mainLayout->addLayout(formationLayout);

    QHBoxLayout* nameLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel("Lineup Name:", this);
    lineupNameInput = new QLineEdit(this);
    lineupNameInput->setPlaceholderText("Optional: Enter a name for the lineup");
    lineupNameInput->setMinimumWidth(200);

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(lineupNameInput);
    mainLayout->addLayout(nameLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    createButton = new QPushButton("Create", this);
    cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(createButton);

    mainLayout->addLayout(buttonLayout);
}

void LineupCreationDialog::setupConnections() {
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(createButton, &QPushButton::clicked, [this]() {
        int formationId = getSelectedFormationId();
        
        if (formationId <= 0) {
            QMessageBox::warning(this, "Error", "Please select a formation.");
            return;
        }
        
        accept();
    });
}

int LineupCreationDialog::getSelectedFormationId() const {
    int index = formationComboBox->currentIndex();
    return formationComboBox->itemData(index).toInt();
}

QString LineupCreationDialog::getLineupName() const {
    return lineupNameInput->text().trimmed();
}

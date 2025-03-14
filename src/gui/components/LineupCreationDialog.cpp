#include "gui/components/LineupCreationDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

LineupCreationDialog::LineupCreationDialog(TeamManager& tm, QWidget *parent)
    : QDialog(parent)
    , teamManager(tm)
{
    setWindowTitle(tr("Create New Lineup"));
    setupUi();
    populateFormations();
    setupConnections();
    setModal(true);
    resize(400, 150);
}

void LineupCreationDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    setupFormationSection(mainLayout);
    setupNameSection(mainLayout);
    setupButtonSection(mainLayout);
}

void LineupCreationDialog::setupFormationSection(QVBoxLayout* mainLayout) {
    auto* formationLayout = new QHBoxLayout();
    auto* formationLabel = new QLabel(tr("Formation:"), this);
    formationComboBox = new QComboBox(this);
    formationComboBox->setMinimumWidth(200);

    formationLayout->addWidget(formationLabel);
    formationLayout->addWidget(formationComboBox);
    mainLayout->addLayout(formationLayout);
}

void LineupCreationDialog::setupNameSection(QVBoxLayout* mainLayout) {
    auto* nameLayout = new QHBoxLayout();
    auto* nameLabel = new QLabel(tr("Lineup Name:"), this);
    lineupNameInput = new QLineEdit(this);
    lineupNameInput->setPlaceholderText(tr("Optional: Enter a name for the lineup"));
    lineupNameInput->setMinimumWidth(200);

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(lineupNameInput);
    mainLayout->addLayout(nameLayout);
}

void LineupCreationDialog::setupButtonSection(QVBoxLayout* mainLayout) {
    auto* buttonLayout = new QHBoxLayout();
    createButton = new QPushButton(tr("Create"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);
    
    createButton->setDefault(true);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(createButton);

    mainLayout->addLayout(buttonLayout);
}

void LineupCreationDialog::populateFormations() {
    if (!formationComboBox) {
        return;
    }
    
    formationComboBox->clear();
    
    const std::vector<Formation>& formations = teamManager.getAllFormations();
    for (const auto& formation : formations) {
        formationComboBox->addItem(
            QString::fromStdString(formation.name), 
            formation.id
        );
    }
}

void LineupCreationDialog::setupConnections() {
    if (!cancelButton || !createButton) {
        return;
    }
    
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(createButton, &QPushButton::clicked, [this]() {
        if (validateInput()) {
            accept();
        }
    });
    
    connect(lineupNameInput, &QLineEdit::returnPressed, createButton, &QPushButton::click);
}

bool LineupCreationDialog::validateInput() const {
    int formationId = getSelectedFormationId();
    
    if (formationId <= 0) {
        QMessageBox::warning(
            const_cast<LineupCreationDialog*>(this), 
            tr("Error"), 
            tr("Please select a formation.")
        );
        return false;
    }
    
    return true;
}

int LineupCreationDialog::getSelectedFormationId() const {
    if (!formationComboBox || formationComboBox->count() == 0) {
        return -1;
    }
    
    int index = formationComboBox->currentIndex();
    return formationComboBox->itemData(index).toInt();
}

QString LineupCreationDialog::getLineupName() const {
    return lineupNameInput ? lineupNameInput->text().trimmed() : QString();
}

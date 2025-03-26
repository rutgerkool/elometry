#include "gui/components/dialogs/LineupCreationDialog.h"
#include "services/TeamManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSpacerItem>

LineupCreationDialog::LineupCreationDialog(TeamManager& teamManager, QWidget* parent)
    : QDialog(parent)
    , m_teamManager(teamManager)
{
    setWindowTitle(tr("Create New Lineup"));
    setModal(true);
    resize(450, 180);
    
    setupUi();
    populateFormations();
    setupConnections();
}

int LineupCreationDialog::getSelectedFormationId() const {
    if (!m_formationComboBox || m_formationComboBox->count() == 0) {
        return -1;
    }
    
    const int index = m_formationComboBox->currentIndex();
    return m_formationComboBox->itemData(index).toInt();
}

QString LineupCreationDialog::getLineupName() const {
    return m_lineupNameInput ? m_lineupNameInput->text().trimmed() : QString();
}

void LineupCreationDialog::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(24, 24, 24, 24);
    m_mainLayout->setSpacing(16);

    setupFormationSection();
    setupNameSection();
    setupButtonSection();
}

void LineupCreationDialog::setupFormationSection() {
    auto* formationLayout = new QHBoxLayout();
    formationLayout->setSpacing(12);
    
    auto* formationLabel = new QLabel(tr("Formation:"), this);
    formationLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formationLabel->setMinimumWidth(100);
    
    m_formationComboBox = new QComboBox(this);
    m_formationComboBox->setMinimumWidth(250);
    
    formationLayout->addWidget(formationLabel);
    formationLayout->addWidget(m_formationComboBox, 1);
    
    m_mainLayout->addLayout(formationLayout);
}

void LineupCreationDialog::setupNameSection() {
    auto* nameLayout = new QHBoxLayout();
    nameLayout->setSpacing(12);
    
    auto* nameLabel = new QLabel(tr("Lineup Name:"), this);
    nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    nameLabel->setMinimumWidth(100);
    
    m_lineupNameInput = new QLineEdit(this);
    m_lineupNameInput->setPlaceholderText(tr("Optional: Enter a name for the lineup"));
    m_lineupNameInput->setMinimumWidth(250);
    
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_lineupNameInput, 1);
    
    m_mainLayout->addLayout(nameLayout);
}

void LineupCreationDialog::setupButtonSection() {
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    auto* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_createButton = new QPushButton(tr("Create"), this);
    m_createButton->setDefault(true);
    
    buttonLayout->addItem(spacer);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_createButton);
    
    m_mainLayout->addLayout(buttonLayout);
}

void LineupCreationDialog::populateFormations() {
    if (!m_formationComboBox) {
        return;
    }
    
    m_formationComboBox->clear();
    
    const auto& formations = m_teamManager.getAllFormations();
    for (const auto& formation : formations) {
        m_formationComboBox->addItem(
            QString::fromStdString(formation.name), 
            formation.id
        );
    }
}

void LineupCreationDialog::setupConnections() {
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_createButton, &QPushButton::clicked, this, &LineupCreationDialog::validateAndAccept);
    connect(m_lineupNameInput, &QLineEdit::returnPressed, m_createButton, &QPushButton::click);
}

bool LineupCreationDialog::isInputValid() const {
    const int formationId = getSelectedFormationId();
    
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

void LineupCreationDialog::validateAndAccept() {
    if (isInputValid()) {
        accept();
    }
}

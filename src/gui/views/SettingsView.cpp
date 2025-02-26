#include "gui/views/SettingsView.h"
#include <QtWidgets>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>

SettingsView::SettingsView(Database& db, QWidget *parent)
    : QWidget(parent)
    , database(db)
{
    setupUi();

    usernameLineEdit->setText(QString::fromStdString(database.getKaggleUsername()));
    keyLineEdit->setText(QString::fromStdString(database.getKaggleKey()));
}

SettingsView::~SettingsView() {
}

void SettingsView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    QWidget* formWidget = new QWidget(this);
    QFormLayout* formLayout = new QFormLayout(formWidget);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    formLayout->setHorizontalSpacing(10);
    formLayout->setVerticalSpacing(10);

    QLabel* explanationLabel = new QLabel(
        "To enable automatic updates of player data, please provide your Kaggle credentials. "
        "These credentials are only stored locally on your machine and are not shared with anyone.", this);
    explanationLabel->setWordWrap(true);
    explanationLabel->setAlignment(Qt::AlignJustify);
    explanationLabel->setMargin(10);
    formLayout->addRow(explanationLabel);

    usernameLineEdit = new QLineEdit(this);
    formLayout->addRow("Kaggle Username:", usernameLineEdit);

    keyLineEdit = new QLineEdit(this);
    keyLineEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("Kaggle Key:", keyLineEdit);

    saveButton = new QPushButton("Save", this);
    formLayout->addRow(saveButton);

    backButton = new QPushButton("Back", this);
    formLayout->addRow(backButton);

    mainLayout->addWidget(formWidget);

    connect(saveButton, &QPushButton::clicked, this, &SettingsView::saveSettings);
    connect(backButton, &QPushButton::clicked, this, &SettingsView::backToMain);
}

void SettingsView::saveSettings() {
    QString updatedUsername = usernameLineEdit->text();
    QString updatedKey = keyLineEdit->text();

    database.setKaggleCredentials(updatedUsername.toStdString(), updatedKey.toStdString());

    QMessageBox::information(this, "Settings Saved", "Kaggle credentials have been saved successfully.");
}

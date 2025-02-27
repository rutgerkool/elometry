#include "gui/views/SettingsView.h"
#include <QtWidgets>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>
#include <QEasingCurve>
#include <QParallelAnimationGroup>

SettingsView::SettingsView(Database& db, QWidget *parent)
    : QWidget(parent)
    , database(db)
{
    setupUi();
    setupAnimations();

    usernameLineEdit->setText(QString::fromStdString(database.getKaggleUsername()));
    keyLineEdit->setText(QString::fromStdString(database.getKaggleKey()));
    
    animateForm();
}

SettingsView::~SettingsView() {
}

void SettingsView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    backButton = new QPushButton("Back", this);
    mainLayout->addWidget(backButton, 0, Qt::AlignLeft);
    
    formWidget = new QWidget(this);
    formWidget->setMinimumWidth(500);
    formWidget->setMaximumWidth(600);
    
    QFormLayout* formLayout = new QFormLayout(formWidget);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(20);

    QLabel* headerLabel = new QLabel("Settings", formWidget);
    headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #0c7bb3; margin-bottom: 20px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    formLayout->addRow(headerLabel);

    QLabel* explanationLabel = new QLabel(
        "To enable automatic updates of player data, please provide your Kaggle credentials. "
        "These credentials are only stored locally on your machine and are not shared with anyone.", formWidget);
    explanationLabel->setWordWrap(true);
    explanationLabel->setAlignment(Qt::AlignJustify);
    explanationLabel->setStyleSheet("color: #a0a0a0; margin: 15px 0;");
    formLayout->addRow(explanationLabel);

    usernameLineEdit = new QLineEdit(formWidget);
    usernameLineEdit->setStyleSheet("padding: 8px; border-radius: 4px; background-color: #252525; color: #e0e0e0;");
    QLabel* usernameLabel = new QLabel("Kaggle Username:", formWidget);
    usernameLabel->setStyleSheet("font-weight: bold; color: #e0e0e0;");
    formLayout->addRow(usernameLabel, usernameLineEdit);

    keyLineEdit = new QLineEdit(formWidget);
    keyLineEdit->setEchoMode(QLineEdit::Password);
    keyLineEdit->setStyleSheet("padding: 8px; border-radius: 4px; background-color: #252525; color: #e0e0e0;");
    QLabel* keyLabel = new QLabel("Kaggle Key:", formWidget);
    keyLabel->setStyleSheet("font-weight: bold; color: #e0e0e0;");
    formLayout->addRow(keyLabel, keyLineEdit);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    saveButton = new QPushButton("Save Settings", formWidget);
    saveButton->setStyleSheet("padding: 10px 20px;");
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    formLayout->addRow(buttonLayout);

    mainLayout->addStretch();
    mainLayout->addWidget(formWidget);
    mainLayout->addStretch();

    connect(saveButton, &QPushButton::clicked, this, &SettingsView::saveSettings);
    connect(backButton, &QPushButton::clicked, this, &SettingsView::backToMain);
}

void SettingsView::setupAnimations() {
    formOpacityEffect = new QGraphicsOpacityEffect(formWidget);
    formWidget->setGraphicsEffect(formOpacityEffect);
    formOpacityEffect->setOpacity(0.0);
    
    formOpacityAnimation = new QPropertyAnimation(formOpacityEffect, "opacity");
    formOpacityAnimation->setDuration(500);
    formOpacityAnimation->setStartValue(0.0);
    formOpacityAnimation->setEndValue(1.0);
    formOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    formSlideAnimation = new QPropertyAnimation(formWidget, "pos");
    formSlideAnimation->setDuration(500);
    formSlideAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    formAnimationGroup = new QParallelAnimationGroup(this);
    formAnimationGroup->addAnimation(formOpacityAnimation);
    formAnimationGroup->addAnimation(formSlideAnimation);
}

void SettingsView::animateForm() {
    QPoint currentPos = formWidget->pos();
    QPoint startPos = currentPos + QPoint(0, 50);
    
    formSlideAnimation->setStartValue(startPos);
    formSlideAnimation->setEndValue(currentPos);
    
    formAnimationGroup->start();
}

void SettingsView::saveSettings() {
    QString updatedUsername = usernameLineEdit->text();
    QString updatedKey = keyLineEdit->text();

    database.setKaggleCredentials(updatedUsername.toStdString(), updatedKey.toStdString());

    QMessageBox msgBox;
    msgBox.setWindowTitle("Settings Saved");
    msgBox.setText("Kaggle credentials have been saved successfully.");
    msgBox.setIcon(QMessageBox::Information);

    msgBox.exec();
}

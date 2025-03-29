#include "gui/views/SettingsView.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScreen>
#include <QApplication>
#include <QCursor>
#include <QEasingCurve>

SettingsView::SettingsView(Database& database, QWidget* parent)
    : QWidget(parent)
    , m_database(database)
{
    setupUi();
    setupAnimations();
    setupConnections();

    m_usernameLineEdit->setText(QString::fromStdString(m_database.getKaggleUsername()));
    m_keyLineEdit->setText(QString::fromStdString(m_database.getKaggleKey()));
    
    animateForm();
}

void SettingsView::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    m_formWidget = new QWidget(this);
    m_formWidget->setMinimumWidth(500);
    m_formWidget->setMaximumWidth(600);
    
    createFormLayout();

    mainLayout->addStretch();
    mainLayout->addWidget(m_formWidget);
    mainLayout->addStretch();
}

void SettingsView::createFormLayout()
{
    auto* formLayout = new QFormLayout(m_formWidget);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(20);

    createFormHeader(formLayout);
    createFormFields(formLayout);
    createFormButtons(formLayout);
}

void SettingsView::createFormHeader(QLayout* layout)
{
    auto* formLayout = qobject_cast<QFormLayout*>(layout);
    if (!formLayout) return;

    auto* headerLabel = new QLabel("Settings", m_formWidget);
    headerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #0c7bb3; margin-bottom: 20px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    formLayout->addRow(headerLabel);

    auto* explanationLabel = new QLabel(
        "To enable automatic updates of player data, please provide your Kaggle credentials. "
        "These credentials are only stored locally on your machine and are not shared with anyone.", 
        m_formWidget);
    explanationLabel->setWordWrap(true);
    explanationLabel->setAlignment(Qt::AlignJustify);
    explanationLabel->setStyleSheet("color: #a0a0a0; margin: 15px 0;");
    formLayout->addRow(explanationLabel);
}

void SettingsView::createFormFields(QLayout* layout)
{
    auto* formLayout = qobject_cast<QFormLayout*>(layout);
    if (!formLayout) return;

    m_usernameLineEdit = new QLineEdit(m_formWidget);
    m_usernameLineEdit->setStyleSheet("padding: 8px; border-radius: 4px; background-color: #252525; color: #e0e0e0;");
    
    auto* usernameLabel = new QLabel("Kaggle Username:", m_formWidget);
    usernameLabel->setStyleSheet("font-weight: bold; color: #e0e0e0;");
    formLayout->addRow(usernameLabel, m_usernameLineEdit);

    m_keyLineEdit = new QLineEdit(m_formWidget);
    m_keyLineEdit->setEchoMode(QLineEdit::Password);
    m_keyLineEdit->setStyleSheet("padding: 8px; border-radius: 4px; background-color: #252525; color: #e0e0e0;");
    
    auto* keyLabel = new QLabel("Kaggle Key:", m_formWidget);
    keyLabel->setStyleSheet("font-weight: bold; color: #e0e0e0;");
    formLayout->addRow(keyLabel, m_keyLineEdit);
}

void SettingsView::createFormButtons(QLayout* layout)
{
    auto* formLayout = qobject_cast<QFormLayout*>(layout);
    if (!formLayout) return;

    auto* buttonLayout = new QHBoxLayout();
    m_backButton = new QPushButton("Back", this);
    m_saveButton = new QPushButton("Save Settings", m_formWidget);
    m_saveButton->setStyleSheet("padding: 10px 20px;");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_backButton);
    buttonLayout->addWidget(m_saveButton);
    
    formLayout->addRow(buttonLayout);
}

void SettingsView::setupConnections()
{
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsView::saveSettings);
    connect(m_backButton, &QPushButton::clicked, this, &SettingsView::backToMain);
}

void SettingsView::setupAnimations()
{
    m_formOpacityEffect = new QGraphicsOpacityEffect(m_formWidget);
    m_formWidget->setGraphicsEffect(m_formOpacityEffect);
    m_formOpacityEffect->setOpacity(0.0);
    
    m_formOpacityAnimation = new QPropertyAnimation(m_formOpacityEffect, "opacity", this);
    m_formOpacityAnimation->setDuration(250);
    m_formOpacityAnimation->setStartValue(0.0);
    m_formOpacityAnimation->setEndValue(1.0);
    m_formOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    m_formSlideAnimation = new QPropertyAnimation(m_formWidget, "pos", this);
    m_formSlideAnimation->setDuration(250);
    m_formSlideAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    m_formAnimationGroup = new QParallelAnimationGroup(this);
    m_formAnimationGroup->addAnimation(m_formOpacityAnimation);
    m_formAnimationGroup->addAnimation(m_formSlideAnimation);
}

void SettingsView::animateForm()
{
    QPoint currentPos = m_formWidget->pos();
    QPoint startPos = currentPos + QPoint(0, 50);
    
    m_formSlideAnimation->setStartValue(startPos);
    m_formSlideAnimation->setEndValue(currentPos);
    
    m_formAnimationGroup->start();
}

void SettingsView::saveSettings()
{
    QString updatedUsername = m_usernameLineEdit->text();
    QString updatedKey = m_keyLineEdit->text();

    m_database.setKaggleCredentials(updatedUsername.toStdString(), updatedKey.toStdString());
    showSuccessMessage();
}

void SettingsView::showSuccessMessage()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Settings Saved");
    msgBox.setText("Kaggle credentials have been saved successfully.");
    msgBox.setIcon(QMessageBox::Information);

    QScreen* screen = QApplication::screenAt(QCursor::pos());
    if (!screen) screen = QApplication::primaryScreen();
    
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        msgBox.adjustSize();
        msgBox.move(
            screenGeometry.x() + (screenGeometry.width() - msgBox.width()) / 2,
            screenGeometry.y() + (screenGeometry.height() - msgBox.height()) / 2
        );
    }
    
    msgBox.exec();
}

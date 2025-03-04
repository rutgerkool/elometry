#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QtWidgets/QFormLayout>
#include "utils/database/Database.h"

class SettingsView : public QWidget {
    Q_OBJECT

public:
    explicit SettingsView(Database& database, QWidget *parent = nullptr);
    ~SettingsView();

signals:
    void backToMain();

private slots:
    void saveSettings();

private:
    void setupUi();
    void createFormLayout();
    void createFormHeader(QFormLayout* formLayout);
    void createFormFields(QFormLayout* formLayout);
    void createFormButtons(QFormLayout* formLayout);
    void setupConnections();
    void setupAnimations();
    void setupFormAnimations();
    void animateForm();
    void showSuccessMessage();

    Database& database;
    QLineEdit* usernameLineEdit;
    QLineEdit* keyLineEdit;
    QPushButton* saveButton;
    QPushButton* backButton;
    
    QWidget* formWidget;
    QGraphicsOpacityEffect* formOpacityEffect;
    QPropertyAnimation* formOpacityAnimation;
    QPropertyAnimation* formSlideAnimation;
    QParallelAnimationGroup* formAnimationGroup;
};

#endif

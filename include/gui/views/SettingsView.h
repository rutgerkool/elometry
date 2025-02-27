#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
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
    
    void setupUi();
    void setupAnimations();
    void animateForm();
};

#endif

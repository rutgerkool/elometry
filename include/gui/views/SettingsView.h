#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include "utils/database/Database.h"

class SettingsView : public QWidget {
    Q_OBJECT

public:
    explicit SettingsView(Database& database, QWidget *parent = nullptr);
    ~SettingsView() override;

signals:
    void backToMain();

private:
    Database& database;
    QLineEdit* usernameLineEdit;
    QLineEdit* keyLineEdit;
    QPushButton* saveButton;
    QPushButton* backButton;

    void setupUi();

private slots:
    void saveSettings();
};

#endif

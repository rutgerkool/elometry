#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <string_view>
#include "utils/database/Database.h"

class SettingsView final : public QWidget {
    Q_OBJECT

    public:
        explicit SettingsView(Database& database, QWidget* parent = nullptr);
        ~SettingsView() override = default;

        SettingsView(const SettingsView&) = delete;
        SettingsView& operator=(const SettingsView&) = delete;
        SettingsView(SettingsView&&) = delete;
        SettingsView& operator=(SettingsView&&) = delete;

    signals:
        void backToMain();

    private slots:
        void saveSettings();
        void animateForm();
        void showSuccessMessage();

    private:
        void setupUi();
        void createFormLayout();
        void createFormHeader(QLayout* layout);
        void createFormFields(QLayout* layout);
        void createFormButtons(QLayout* layout);
        void setupConnections();
        void setupAnimations();
        void applyFormAnimations();

        Database& m_database;
        QLineEdit* m_usernameLineEdit{nullptr};
        QLineEdit* m_keyLineEdit{nullptr};
        QPushButton* m_saveButton{nullptr};
        QPushButton* m_backButton{nullptr};
        
        QWidget* m_formWidget{nullptr};
        QGraphicsOpacityEffect* m_formOpacityEffect{nullptr};
        QPropertyAnimation* m_formOpacityAnimation{nullptr};
        QPropertyAnimation* m_formSlideAnimation{nullptr};
        QParallelAnimationGroup* m_formAnimationGroup{nullptr};
};

#endif

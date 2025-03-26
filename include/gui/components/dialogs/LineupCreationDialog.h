#ifndef LINEUPCREATEDIALOG_H
#define LINEUPCREATEDIALOG_H

#include <QDialog>
#include <QString>
#include <memory>

class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;
class TeamManager;

class LineupCreationDialog final : public QDialog {
    Q_OBJECT

    public:
        explicit LineupCreationDialog(TeamManager& teamManager, QWidget* parent = nullptr);
        ~LineupCreationDialog() override = default;
        
        LineupCreationDialog(const LineupCreationDialog&) = delete;
        LineupCreationDialog& operator=(const LineupCreationDialog&) = delete;
        LineupCreationDialog(LineupCreationDialog&&) = delete;
        LineupCreationDialog& operator=(LineupCreationDialog&&) = delete;

        [[nodiscard]] int getSelectedFormationId() const;
        [[nodiscard]] QString getLineupName() const;

    private slots:
        void validateAndAccept();

    private:
        void setupUi();
        void setupFormationSection();
        void setupNameSection();
        void setupButtonSection();
        void setupConnections();
        void populateFormations();
        [[nodiscard]] bool isInputValid() const;

        TeamManager& m_teamManager;
        QVBoxLayout* m_mainLayout{nullptr};
        
        QComboBox* m_formationComboBox{nullptr};
        QLineEdit* m_lineupNameInput{nullptr};
        QPushButton* m_createButton{nullptr};
        QPushButton* m_cancelButton{nullptr};
};

#endif

#ifndef CLUBSELECTDIALOG_H
#define CLUBSELECTDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <vector>
#include <string>
#include <span>
#include <optional>

class QStandardItemModel;

class ClubSelectDialog : public QDialog {
    Q_OBJECT

    public:
        explicit ClubSelectDialog(std::span<const std::pair<int, std::string>> clubs, QWidget* parent = nullptr);
        ~ClubSelectDialog() override = default;
        
        [[nodiscard]] std::optional<int> getSelectedClubId() const;
        
    private slots:
        void filterClubs();
        void handleClubSelection();
        void handleDoubleClick(const QModelIndex& index);
        
    private:
        void setupUi();
        void setupConnections();
        void createModel();
        void updateModel(const QString& filter = {});
        [[nodiscard]] QStandardItemModel* createFilteredModel(const QString& filter) const;
        [[nodiscard]] int getCurrentClubId() const;
        
        QLineEdit* m_searchLineEdit{nullptr};
        QTableView* m_clubsTableView{nullptr};
        QPushButton* m_selectButton{nullptr};
        QPushButton* m_cancelButton{nullptr};
        QStandardItemModel* m_clubsModel{nullptr};
        
        std::vector<std::pair<int, std::string>> m_availableClubs;
        std::optional<int> m_selectedClubId;
};

#endif

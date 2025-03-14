#ifndef CLUBSELECTDIALOG_H
#define CLUBSELECTDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <vector>
#include <string>

class ClubSelectDialog : public QDialog {
    Q_OBJECT

    public:
        ClubSelectDialog(const std::vector<std::pair<int, std::string>>& clubs, QWidget* parent = nullptr);
        
        int getSelectedClubId() const;
        
    private slots:
        void filterClubs(const QString& text);
        void handleDoubleClick(const QModelIndex& index);
        
    private:
        QLineEdit* searchLineEdit;
        QTableView* clubsTableView;
        QPushButton* selectButton;
        QPushButton* cancelButton;
        
        std::vector<std::pair<int, std::string>> availableClubs;
        int selectedClubId = -1;
        
        void setupUi();
        void setupConnections();
        void updateClubsModel(const QString& filter = QString());
};

#endif 
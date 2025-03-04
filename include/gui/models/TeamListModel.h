#ifndef TEAMLISTMODEL_H
#define TEAMLISTMODEL_H

#include "models/PlayerRating.h"
#include "services/TeamManager.h"
#include <QtCore/QAbstractTableModel>
#include <QtCore/QAbstractListModel>
#include <QtCore/QString>
#include <vector>

class TeamListModel : public QAbstractListModel {
    Q_OBJECT

    public:
        explicit TeamListModel(TeamManager& teamManager, QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    public slots:
        void refresh();

    private:
        TeamManager& teamManager;
        std::vector<Team> teams;
};

#endif

#ifndef DATALOADER_H
#define DATALOADER_H

#include <QObject>
#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/Database.h"

class DataLoader : public QObject {
    Q_OBJECT

public:
    explicit DataLoader(RatingManager& rm, TeamManager& tm, Database& db, QObject* parent = nullptr);
    
public slots:
    void loadData();
    
signals:
    void progressUpdate(const QString& status, int progress);
    void loadingComplete();
    
private:
    RatingManager& ratingManager;
    TeamManager& teamManager;
    Database& database;
};

#endif

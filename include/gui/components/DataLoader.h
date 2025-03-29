#ifndef DATALOADER_H
#define DATALOADER_H

#include <QObject>
#include <QString>
#include <memory>
#include <filesystem>

class RatingManager;
class TeamManager;
class Database;

class DataLoader final : public QObject {
    Q_OBJECT

    public:
        explicit DataLoader(RatingManager& ratingManager, 
                        TeamManager& teamManager, 
                        Database& database, 
                        QObject* parent = nullptr);
        ~DataLoader() override = default;

        DataLoader(const DataLoader&) = delete;
        DataLoader& operator=(const DataLoader&) = delete;
        DataLoader(DataLoader&&) = delete;
        DataLoader& operator=(DataLoader&&) = delete;

    public slots:
        void loadData();
        
    signals:
        void progressUpdate(const QString& status, int progress);
        void loadingComplete();
            
    private:
        void initializeDatabase();
        void loadRatingData();
        void loadTeamData();
        void notifyProgress(const QString& status, int progress);

        RatingManager& m_ratingManager;
        TeamManager& m_teamManager;
        Database& m_database;
};

#endif

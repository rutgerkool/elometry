#ifndef KAGGLE_API_H
#define KAGGLE_API_H

#include <string>
#include <curl/curl.h>
#include <ctime>

class KaggleAPIClient {
    public:
        KaggleAPIClient();
        KaggleAPIClient(const std::string& username, const std::string& key);
        ~KaggleAPIClient();

        time_t getDatasetLastUpdated(const std::string& dataset);
        bool downloadDataset(const std::string& dataset, const std::string& outputPath);
        
        std::string getUsername() const { return username; }
        std::string getKey() const { return key; }

    private:
        std::string username;
        std::string key;
        std::string authHeader;

        std::string makeApiRequest(const std::string& endpoint, bool requireAuth = true);
        std::string extractDateFromJson(const std::string& jsonResponse);
        time_t convertIsoDateToTimestamp(const std::string& dateString);
        std::string base64Encode(const std::string& input);
        
        static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
        static size_t WriteDataCallback(void* ptr, size_t size, size_t nmemb, FILE* stream);
};

#endif

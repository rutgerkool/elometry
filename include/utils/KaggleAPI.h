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
        bool setupDownloadRequest(CURL* curl, FILE* fp, struct curl_slist** headers);
        bool initializeDownload(const std::string& dataset, CURL** curl, FILE** fp, const std::string& outputPath);
        
        std::string getUsername() const { return username; }
        std::string getKey() const { return key; }

    private:
        std::string username;
        std::string key;
        std::string authHeader;
        
        bool validateCredentials(bool requireAuth) const;
        std::string makeApiRequest(const std::string& endpoint, bool requireAuth = true);
        bool setupApiRequest(CURL* curl, struct curl_slist** headers, std::string& responseBuffer, bool requireAuth);
        bool executeRequest(CURL* curl, long& http_code);
        std::string extractDateFromJson(const std::string& jsonResponse);
        time_t convertIsoDateToTimestamp(const std::string& dateString);
        std::string base64Encode(const std::string& input);
        void processBase64Block(unsigned char* char_array_3, unsigned char* char_array_4, 
                               std::string& encoded, const std::string& base64_chars);
        void processBase64Remainder(int i, unsigned char* char_array_3, unsigned char* char_array_4, 
                                  std::string& encoded, const std::string& base64_chars);
        
        CURL* initCurl(const std::string& url);
        void cleanupCurl(CURL* curl, struct curl_slist* headers);
        
        static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
        static size_t WriteDataCallback(void* ptr, size_t size, size_t nmemb, FILE* stream);
};

#endif

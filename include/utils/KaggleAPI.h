#ifndef KAGGLE_API_H
#define KAGGLE_API_H

#include <string>
#include <string_view>
#include <optional>
#include <curl/curl.h>
#include <ctime>
#include <span>
#include <vector>
#include <memory>

class KaggleAPIClient {
public:
    KaggleAPIClient();
    KaggleAPIClient(std::string_view username, std::string_view key);
    ~KaggleAPIClient();

    KaggleAPIClient(const KaggleAPIClient&) = delete;
    KaggleAPIClient& operator=(const KaggleAPIClient&) = delete;
    KaggleAPIClient(KaggleAPIClient&&) noexcept;
    KaggleAPIClient& operator=(KaggleAPIClient&&) noexcept;

    [[nodiscard]] time_t getDatasetLastUpdated(std::string_view dataset) const;
    [[nodiscard]] bool downloadDataset(std::string_view dataset, std::string_view outputPath) const;
    
    [[nodiscard]] std::string_view getUsername() const noexcept { return m_username; }
    [[nodiscard]] std::string_view getKey() const noexcept { return m_key; }

private:
    struct CurlDeleter {
        void operator()(CURL* curl) const noexcept {
            if (curl) curl_easy_cleanup(curl);
        }
    };

    struct CurlHeadersDeleter {
        void operator()(curl_slist* headers) const noexcept {
            if (headers) curl_slist_free_all(headers);
        }
    };

    std::string m_username;
    std::string m_key;
    std::string m_authHeader;
    
    [[nodiscard]] bool validateCredentials(bool requireAuth) const noexcept;
    [[nodiscard]] std::string makeApiRequest(std::string_view endpoint, bool requireAuth = true) const;
    [[nodiscard]] std::string base64Encode(std::string_view input) const;
    
    [[nodiscard]] std::string extractDateFromJson(std::string_view jsonResponse) const;
    [[nodiscard]] time_t convertIsoDateToTimestamp(std::string_view dateString) const;
    
    [[nodiscard]] std::unique_ptr<CURL, CurlDeleter> initCurl(std::string_view url) const;
    [[nodiscard]] bool setupApiRequest(
        CURL* curl, 
        curl_slist** headers, 
        std::string& responseBuffer, 
        bool requireAuth
    ) const;
    
    [[nodiscard]] bool setupDownloadRequest(
        CURL* curl,
        FILE* fp,
        curl_slist** headers
    ) const;
    
    [[nodiscard]] bool executeRequest(
        CURL* curl, 
        long& httpCode
    ) const;
    
    [[nodiscard]] std::optional<FILE*> openOutputFile(std::string_view outputPath) const;
    
    static size_t writeMemoryCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    static size_t writeDataCallback(void* ptr, size_t size, size_t nmemb, FILE* stream);
};

#endif

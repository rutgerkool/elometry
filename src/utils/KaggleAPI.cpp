#include "utils/KaggleAPI.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <iomanip>
#include <array>
#include <algorithm>
#include <utility>

KaggleAPIClient::KaggleAPIClient() = default;

KaggleAPIClient::KaggleAPIClient(std::string_view username, std::string_view key) 
    : m_username(username)
    , m_key(key) 
{
    if (!m_username.empty() && !m_key.empty()) {
        m_authHeader = "Authorization: Basic " + base64Encode(m_username + ":" + m_key);
    }
}

KaggleAPIClient::~KaggleAPIClient() = default;

KaggleAPIClient::KaggleAPIClient(KaggleAPIClient&& other) noexcept
    : m_username(std::move(other.m_username))
    , m_key(std::move(other.m_key))
    , m_authHeader(std::move(other.m_authHeader))
{
}

KaggleAPIClient& KaggleAPIClient::operator=(KaggleAPIClient&& other) noexcept {
    if (this != &other) {
        m_username = std::move(other.m_username);
        m_key = std::move(other.m_key);
        m_authHeader = std::move(other.m_authHeader);
    }
    return *this;
}

bool KaggleAPIClient::validateCredentials(bool requireAuth) const noexcept {
    if (requireAuth && (m_username.empty() || m_key.empty())) {
        std::cerr << "Kaggle credentials not set for authenticated request" << std::endl;
        return false;
    }
    return true;
}

std::string KaggleAPIClient::base64Encode(std::string_view input) const {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
        
    std::string encoded;
    encoded.reserve((input.size() + 2) / 3 * 4);
    
    std::array<unsigned char, 3> char_array_3{};
    std::array<unsigned char, 4> char_array_4{};
    
    size_t i = 0;
    auto bytes_to_encode = reinterpret_cast<const unsigned char*>(input.data());
    auto in_len = input.size();
    
    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++) {
                encoded += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    
    if (i > 0) {
        for (size_t j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (size_t j = 0; j < i + 1; j++) {
            encoded += base64_chars[char_array_4[j]];
        }
        
        while ((i++ < 3)) {
            encoded += '=';
        }
    }
    
    return encoded;
}

size_t KaggleAPIClient::writeMemoryCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t realsize = size * nmemb;
    userp->append(static_cast<char*>(contents), realsize);
    return realsize;
}

size_t KaggleAPIClient::writeDataCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

std::unique_ptr<CURL, KaggleAPIClient::CurlDeleter> KaggleAPIClient::initCurl(std::string_view url) const {
    std::unique_ptr<CURL, CurlDeleter> curl(curl_easy_init());
    
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return nullptr;
    }
    
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.data());
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "Mozilla/5.0");
    
    return curl;
}

bool KaggleAPIClient::setupApiRequest(
    CURL* curl, 
    curl_slist** headers, 
    std::string& responseBuffer, 
    bool requireAuth
) const {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
    
    if (requireAuth && !m_username.empty() && !m_key.empty()) {
        *headers = curl_slist_append(*headers, m_authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, *headers);
    }
    
    return true;
}

bool KaggleAPIClient::executeRequest(CURL* curl, long& httpCode) const {
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    
    if (res != CURLE_OK || httpCode != 200) {
        std::cerr << "API request failed: " << curl_easy_strerror(res) 
                << ", HTTP code: " << httpCode << std::endl;
        return false;
    }
    
    return true;
}

std::string KaggleAPIClient::makeApiRequest(std::string_view endpoint, bool requireAuth) const {
    if (!validateCredentials(requireAuth)) {
        return "";
    }
    
    std::string responseBuffer;
    
    curl_global_init(CURL_GLOBAL_ALL);
    auto curl = initCurl(endpoint);

    if (!curl) {
        curl_global_cleanup();
        return "";
    }
    
    curl_slist* rawHeaders = nullptr;
    std::unique_ptr<curl_slist, CurlHeadersDeleter> headers(nullptr);
    if (!setupApiRequest(curl.get(), &rawHeaders, responseBuffer, requireAuth)) {
        curl_global_cleanup();
        return "";
    }
    headers.reset(rawHeaders);
    
    long httpCode = 0;
    bool success = executeRequest(curl.get(), httpCode);
    
    curl_global_cleanup();
    
    if (!success) {
        std::cerr << "API request failed for endpoint: " << endpoint << std::endl;
    }
    
    return success ? responseBuffer : "";
}

std::string KaggleAPIClient::extractDateFromJson(std::string_view jsonResponse) const {
    std::regex datePattern("\"lastUpdated\"\\s*:\\s*\"([^\"]+)\"");
    std::regex altDatePattern("\"lastUpdatedNullable\"\\s*:\\s*\"([^\"]+)\"");
    std::regex isoPattern("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}");
    std::smatch match;
    
    std::string jsonStr(jsonResponse);
    
    if (std::regex_search(jsonStr, match, datePattern) && match.size() > 1) {
        return match[1].str();
    } 
    
    if (std::regex_search(jsonStr, match, altDatePattern) && match.size() > 1) {
        return match[1].str();
    }
    
    if (std::regex_search(jsonStr, match, isoPattern)) {
        return match[0].str();
    }
    
    return "";
}

time_t KaggleAPIClient::convertIsoDateToTimestamp(std::string_view dateString) const {
    if (dateString.empty()) {
        return 0;
    }
    
    std::string isoDate(dateString);
    size_t dotPos = isoDate.find('.');

    if (dotPos != std::string::npos) {
        isoDate = isoDate.substr(0, dotPos);
    }
    
    std::tm tm = {};
    
    #ifdef _WIN32
        std::istringstream ss(isoDate);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (ss.fail()) {
            return 0;
        }
    #else
        if (strptime(isoDate.c_str(), "%Y-%m-%dT%H:%M:%S", &tm) == nullptr) {
            return 0;
        }
    #endif
    
    return mktime(&tm);
}

time_t KaggleAPIClient::getDatasetLastUpdated(std::string_view dataset) const {
    if (!validateCredentials(true)) {
        std::cerr << "Kaggle credentials required to check for updates" << std::endl;
        return 0;
    }
    
    std::string apiUrl = "https://www.kaggle.com/api/v1/datasets/list?search=" + std::string(dataset) + "&sortBy=updated";
    std::string response = makeApiRequest(apiUrl, true);
    
    if (response.empty()) {
        std::cerr << "Empty response from Kaggle API when checking dataset: " << dataset << std::endl;
        return 0;
    }
    
    std::string dateString = extractDateFromJson(response);
    if (dateString.empty()) {
        std::cerr << "Failed to extract date from Kaggle API response" << std::endl;
        return 0;
    }
    
    time_t timestamp = convertIsoDateToTimestamp(dateString);
    if (timestamp == 0) {
        std::cerr << "Failed to convert date string to timestamp: " << dateString << std::endl;
    }
    
    return timestamp;
}

bool KaggleAPIClient::setupDownloadRequest(
    CURL* curl,
    FILE* fp,
    curl_slist** headers
) const {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);
    
    if (!m_username.empty() && !m_key.empty()) {
        *headers = curl_slist_append(*headers, m_authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, *headers);
    }
    
    return true;
}

std::optional<FILE*> KaggleAPIClient::openOutputFile(std::string_view outputPath) const {
    FILE* fp = fopen(std::string(outputPath).c_str(), "wb");
    
    if (!fp) {
        std::cerr << "Failed to create file for writing: " << outputPath << std::endl;
        return std::nullopt;
    }
    
    return fp;
}

bool KaggleAPIClient::downloadDataset(std::string_view dataset, std::string_view outputPath) const {
    if (!validateCredentials(false)) {
        return false;
    }
    
    std::string apiUrl = "https://www.kaggle.com/api/v1/datasets/download/" + std::string(dataset);
    
    curl_global_init(CURL_GLOBAL_ALL);
    auto curl = initCurl(apiUrl);

    if (!curl) {
        curl_global_cleanup();
        return false;
    }
    
    auto fpOpt = openOutputFile(outputPath);
    if (!fpOpt) {
        curl_global_cleanup();
        return false;
    }
    
    FILE* fp = *fpOpt;
    curl_slist* rawHeaders = nullptr;
    std::unique_ptr<curl_slist, CurlHeadersDeleter> headers(nullptr);
    
    if (!setupDownloadRequest(curl.get(), fp, &rawHeaders)) {
        fclose(fp);
        curl_global_cleanup();
        return false;
    }
    headers.reset(rawHeaders);
    
    long httpCode = 0;
    bool success = executeRequest(curl.get(), httpCode);
    
    fclose(fp);
    curl_global_cleanup();
    
    if (!success) {
        std::cerr << "Failed to download dataset, HTTP code: " << httpCode << std::endl;
    }
    
    return success;
}

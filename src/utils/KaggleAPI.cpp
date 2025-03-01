#include "utils/KaggleAPI.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <iomanip>
#include <ctime>

KaggleAPIClient::KaggleAPIClient() : username(""), key("") {
}

KaggleAPIClient::KaggleAPIClient(const std::string& username, const std::string& key) 
    : username(username), key(key) {
    if (!username.empty() && !key.empty()) {
        std::string authStr = username + ":" + key;
        std::string encodedAuth = base64Encode(authStr);
        authHeader = "Authorization: Basic " + encodedAuth;
    }
}

KaggleAPIClient::~KaggleAPIClient() {}

std::string KaggleAPIClient::base64Encode(const std::string& input) {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
        
    std::string encoded;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    size_t in_len = input.size();
    const char* bytes_to_encode = input.c_str();
    
    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                encoded += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            encoded += base64_chars[char_array_4[j]];
        
        while((i++ < 3))
            encoded += '=';
    }
    
    return encoded;
}

size_t KaggleAPIClient::WriteMemoryCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t realsize = size * nmemb;
    userp->append(static_cast<char*>(contents), realsize);
    return realsize;
}

size_t KaggleAPIClient::WriteDataCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

std::string KaggleAPIClient::makeApiRequest(const std::string& endpoint, bool requireAuth) {
    if (requireAuth && (username.empty() || key.empty())) {
        std::cerr << "Kaggle credentials not set for authenticated request" << std::endl;
        return "";
    }
    
    std::string responseBuffer;
    
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();
    
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        curl_global_cleanup();
        return "";
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    
    struct curl_slist* headers = NULL;
    
    if (requireAuth && !username.empty() && !key.empty()) {
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    
    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (headers != NULL) {
        curl_slist_free_all(headers);
    }
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    if (res != CURLE_OK || http_code != 200) {
        std::cerr << "API request failed: " << curl_easy_strerror(res) 
                << ", HTTP code: " << http_code << std::endl;
        return "";
    }
    
    return responseBuffer;
}

std::string KaggleAPIClient::extractDateFromJson(const std::string& jsonResponse) {
    std::regex datePattern("\"lastUpdated\"\\s*:\\s*\"([^\"]+)\"");
    std::regex altDatePattern("\"lastUpdatedNullable\"\\s*:\\s*\"([^\"]+)\"");
    std::regex isoPattern("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}");
    std::smatch match;
    
    if (std::regex_search(jsonResponse, match, datePattern) && match.size() > 1) {
        return match[1].str();
    } 
    
    if (std::regex_search(jsonResponse, match, altDatePattern) && match.size() > 1) {
        return match[1].str();
    }
    
    if (std::regex_search(jsonResponse, match, isoPattern)) {
        return match[0].str();
    }
    
    return "";
}

time_t KaggleAPIClient::convertIsoDateToTimestamp(const std::string& dateString) {
    if (dateString.empty()) {
        return 0;
    }
    
    std::string isoDate = dateString;
    size_t dotPos = isoDate.find('.');
    if (dotPos != std::string::npos) {
        isoDate = isoDate.substr(0, dotPos);
    }
    
    struct tm tm = {};
    
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

time_t KaggleAPIClient::getDatasetLastUpdated(const std::string& dataset) {
    if (username.empty() || key.empty()) {
        std::cerr << "Kaggle credentials required for checking dataset updates" << std::endl;
        return 0;
    }
    
    std::string apiUrl = "https://www.kaggle.com/api/v1/datasets/list?search=" + dataset + "&sortBy=updated";
    std::string response = makeApiRequest(apiUrl, true);
    
    if (response.empty()) {
        return 0;
    }
    
    std::string dateString = extractDateFromJson(response);
    return convertIsoDateToTimestamp(dateString);
}

bool KaggleAPIClient::downloadDataset(const std::string& dataset, const std::string& outputPath) {
    std::string apiUrl = "https://www.kaggle.com/api/v1/datasets/download/" + dataset;
    
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();
    
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        curl_global_cleanup();
        return false;
    }
    
    FILE* fp = fopen(outputPath.c_str(), "wb");
    if (!fp) {
        std::cerr << "Failed to create file for writing: " << outputPath << std::endl;
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return false;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteDataCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    
    struct curl_slist* headers = NULL;
    if (!username.empty() && !key.empty()) {
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);
    
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    
    CURLcode res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    fclose(fp);
    
    if (headers != NULL) {
        curl_slist_free_all(headers);
    }
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    if (res != CURLE_OK || http_code != 200) {
        std::cerr << "Failed to download dataset: " << curl_easy_strerror(res) 
                  << ", HTTP code: " << http_code << std::endl;
        return false;
    }
    
    return true;
}

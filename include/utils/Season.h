#ifndef SEASONUTILS_H
#define SEASONUTILS_H

#include <ctime>

inline int getCurrentSeasonYear() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    
    int year = localTime->tm_year + 1900; 
    int month = localTime->tm_mon + 1; 

    return (month < 8) ? (year - 1) : year;
}

#endif 

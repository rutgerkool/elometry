#ifndef CSVREADER_H
#define CSVREADER_H

#include <string>
#include <vector>

class CSVReader {
    public:
        CSVReader(const std::string& filename, const std::string& delimiter = ",");
        std::vector<std::vector<std::string>> readData();

    private:
        std::string filename;
        std::string delimiter;
};

#endif

#include "CSVReader.h"
#include <iostream>

int main() {
    CSVReader reader("../data/club_games.csv");
    std::vector<std::vector<std::string>> data = reader.readData();

    for (int i = 0; i < 5 && i < data.size(); i++) {
        for (const auto& cell : data[i]) {
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
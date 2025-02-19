# Football ELO Valuator

This project analyzes football player data from the **Football Data from Transfermarkt dataset**, which includes over 60,000 games, 30,000+ players, 400,000+ player market valuations, and more. The project calculates adapted ELO ratings based on an algorithm [introduced by Wolff et al. in 2020](https://www.researchgate.net/publication/346383793_A_football_player_rating_system).

It is developed in C++ with CMake as the build system. 

## **Project Structure**

```
football-elo-valuator/
│
├── CMakeLists.txt                # CMake build configuration
├── README.md                     # Project README file
│
├── data/                          # Dataset CSV files (excluded from repo)
│   ├── appearances.csv
│   ├── club_games.csv
│   ├── clubs.csv
│   ├── competitions.csv
│   ├── game_events.csv
│   ├── game_lineups.csv
│   ├── games.csv
│   ├── player_ratings.csv
│   ├── player_valuations.csv
│   ├── players.csv
│   └── transfers.csv
│
├── include/                       # Header files
│   ├── models/
│   │   └── PlayerRating.h
│   └── utils/
│       └── database/
│           ├── Database.h
│           └── repositories/
│               ├── AppearanceRepository.h
│               └── GameRepository.h
│
├── src/                           # Source code files
│   ├── main.cpp                   # Entry point
│   ├── models/
│   │   └── PlayerRating.cpp
│   └── utils/
│       └── database/
│           ├── Database.cpp
│           └── repositories/
│               ├── AppearanceRepository.cpp
│               └── GameRepository.cpp
```

## **Dataset Information**
- **Source**: [Kaggle - Football Data from Transfermarkt](https://www.kaggle.com/datasets/davidcariboo/player-scores)
- **Update Frequency**: Weekly
- **Note**: The dataset is not included in this repository due to its size. Please download the dataset from the Kaggle page and place the CSV files in the `data/` directory.

## **Dependencies**

Before compiling the project, install the necessary tools and libraries:

### **G++, CMake & SQLite3**
```bash
sudo apt update
sudo apt install build-essential cmake libsqlite3-dev
```

## **Building the Project**

1. Clone the repository:
   ```bash
   git clone https://github.com/rutgerkool/football-elo-valuator.git
   cd football-elo-valuator
   ```

2. Download the dataset from [Kaggle](https://www.kaggle.com/datasets/davidcariboo/player-scores) and place all CSV files into the `data/` directory.

3. Create a `build` directory and navigate into it:
   ```bash
   mkdir build
   cd build
   ```

4. Run CMake to configure the project:
   ```bash
   cmake ..
   ```

5. Compile the project:
   ```bash
   make
   ```

## **Running the Project**

After compilation, you can run the executable from the `build` directory:
```bash
./FootballELOValuator
```

## **Performance Optimizations**
- Batch INSERT statements to optimize table initialization
- Indexed SQL queries for faster lookups
- Parallel processing for ELO calculations using OpenMP

# Elometry

This project introduces Elometry, an application that analyzes football player data from the **Football Data from Transfermarkt dataset**, which includes over 60,000 games, 30,000+ players, 400,000+ player market valuations, and more. The project calculates adapted ELO ratings based on an algorithm [introduced by Wolff et al. in 2020](https://www.researchgate.net/publication/346383793_A_football_player_rating_system) and provides optimal team selection based on these ratings using Integer Linear Programming (ILP).

It is developed in C++ with CMake as the build system. 

## **Features**
- Player ELO rating calculation based on match performances
- Team management of new/existing teams including:
   - Automatic analysis of missing positions
   - Smart team completion within budget constraints
- Optimal player selection using Integer Linear Programming (ILP)
  - Select best players for specific positions within a budget constraint
  - Optimization based on player ratings and historical market values
- Database management of player and match data
- Performance-optimized data processing

## **Project Structure**

```
elometry/
│
├── CMakeLists.txt                # CMake build configuration
├── README.md                     # Project README file
│
├── cmake/                        # CMake modules
│   └── FindGLPK.cmake           # GLPK finder module
│
├── data/                         # Dataset CSV files (excluded from repo)
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
├── include/                      # Header files
│   ├── models/
│   │   ├── PlayerRating.h
│   │   └── ILPSelector.h        # Team selection algorithm
│   ├── services/
│   │   ├── RatingManager.h      # Rating management service
│   │   └── TeamManager.h        # Team management service
│   └── utils/
│       └── database/
│           ├── Database.h
│           ├── PlayerMapper.h
│           └── repositories/
│               ├── AppearanceRepository.h
│               ├── GameRepository.h
│               ├── PlayerRepository.h
│               └── TeamRepository.h
│
└── src/                         # Source code files
    ├── main.cpp                 # Entry point
    ├── models/
    │   ├── PlayerRating.cpp
    │   └── ILPSelector.cpp
    ├── services/
    │   ├── RatingManager.cpp
    │   └── TeamManager.cpp
    └── utils/
        └── database/
            ├── Database.cpp
            ├── PlayerMapper.cpp
            └── repositories/
                ├── AppearanceRepository.cpp
                ├── GameRepository.cpp
                ├── PlayerRepository.cpp
                └── TeamRepository.cpp
```

## **Dependencies**

Before compiling the project, install the necessary tools and libraries:

### **Required Tools**
- C++ Compiler (GCC/Clang/MSVC)
- CMake (3.15 or higher)
- SQLite3
- GLPK (GNU Linear Programming Kit)

### **Platform-Specific Installation**

#### **Linux (Ubuntu/Debian)**
```bash
sudo apt update
sudo apt install build-essential cmake libsqlite3-dev libglpk-dev
```

#### **macOS**
```bash
brew install cmake sqlite3 glpk
```

#### **Windows**
1. Install Visual Studio or MinGW
2. Install CMake from https://cmake.org/download/
3. Download SQLite3 development files
4. Download GLPK from https://sourceforge.net/projects/winglpk/
   - Extract to a desired location
   - Set environment variable GLPK_ROOT to the installation directory
   - Add DLL directory to PATH:
     ```batch
     set GLPK_ROOT=C:\path\to\glpk-4.65
     set PATH=%PATH%;%GLPK_ROOT%\w64
     ```

## **Building the Project**

1. Clone the repository:
   ```bash
   git clone https://github.com/rutgerkool/elometry.git
   cd elometry
   ```

2. Download the dataset from [Kaggle](https://www.kaggle.com/datasets/davidcariboo/player-scores) and place all CSV files into the `data/` directory.

3. Create and navigate to build directory:
   ```bash
   mkdir build && cd build
   ```

4. Configure with CMake:
   ```bash
   cmake ..
   ```

5. Build the project:
   ```bash
   cmake --build .
   ```

## **Usage**

### **Running the Program**
```bash
./Elometry
```

### **Team Management Example**
```cpp
Database database("test.db");
RatingManager ratingManager(database);
TeamRepository teamRepository(database);
TeamManager teamManager(teamRepository, ratingManager);

ratingManager.loadAndProcessRatings();

Team& team = teamManager.createTeam("My Team FC");

teamManager.addPlayerToTeam(team.teamId, {1, 1, "Player 1", "My Club", "Centre-Back", "Defender", "2026-06-30", 7000000, 10000000});
teamManager.addPlayerToTeam(team.teamId, {2, 1, "Player 2", "My Club", "Centre-Forward", "Attack", "2027-01-30", 4000000, 8000000});

teamManager.autoFillTeam(team, 20000000);  
```

## **Performance Optimizations**
- Batch INSERT statements for table initialization
- Indexed SQL queries for data retrieval
- Parallel processing for ELO calculations using OpenMP
- Integer Linear Programming for optimal team selection

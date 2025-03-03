# Elometry

This project introduces Elometry, an application that analyzes football player data from the **Football Data from Transfermarkt dataset**, which includes over 60,000 games, 30,000+ players, 400,000+ player market valuations, and more. The project calculates adapted ELO ratings based on an algorithm [introduced by Wolff et al. in 2020](https://www.researchgate.net/publication/346383793_A_football_player_rating_system) and provides optimal team selection based on these ratings using Integer Linear Programming (ILP).

It is developed in C++ with CMake as the build system. 

## **Features**

### Player ELO Rating Overview
- Calculate player ratings based on match performances
- Adaptive rating system
- Interactive rating history dialog with performance stats

![Player Ratings Demo](img/player-list.gif)

### Optimal Team Composition
- Integer Linear Programming for team selection
- Optimize players within budget constraints
- Balance performance and market value

![Team Optimization Demo](img/team-manager-view-new.gif)

### Team Management from Existing Clubs
- Create and manage custom teams based on real data
- Budget-constrained team building based on adapted ELO ratings
- Access detailed player rating history from team manager

![Team Management Demo](img/team-manager-view-existing.gif)

## **Project Structure**
```
elometry/
├── CMakeLists.txt
├── cmake/                    # CMake modules
├── db/                       # Database scripts
├── include/                  # Header files
│   ├── gui/                  # User interface headers
│   ├── models/               # Data models
│   ├── services/             # Business logic
│   └── utils/                # Utility classes
├── src/                      # Source code
│   ├── gui/                  # User interface implementation
│   │   ├── components/
│   │   ├── models/
│   │   ├── resources/
│   │   ├── styles/
│   │   └── views/
│   ├── models/               # Implementation of data models
│   ├── services/             # Service implementations
│   └── utils/                # Utility implementations
└── static/                   # Static resources
```

## **Technology Stack**
- C++20
- Qt6 for GUI
- SQLite for data management
- GLPK for Integer Linear Programming
- OpenMP for parallel processing

## **Prerequisites**

### **Required Tools**
- C++ Compiler (GCC 10+/Clang 10+/MSVC 2019+)
- CMake (3.15 or higher)
- Qt6
- SQLite3
- GLPK (GNU Linear Programming Kit)
- CURL for API requests

## **Installation**
### **Linux (Ubuntu/Debian)**
```bash
sudo apt install build-essential cmake qt6-base-dev libglpk-dev libsqlite3-dev libcurl4-openssl-dev libqt6charts6-dev
git clone https://github.com/rutgerkool/elometry.git
cd elometry
```

### **macOS**
```bash
brew install cmake qt@6 glpk sqlite curl
git clone https://github.com/rutgerkool/elometry.git
cd elometry
```

### **Windows**
Windows setup requires several components to be installed separately:

1. **Install Qt6 (MSVC 2022)**
   - Download and install Qt from the [Qt Online Installer](https://www.qt.io/download-qt-installer)
   - Select Qt 6.10.0 for MSVC 2022 64-bit during installation (Custom Installation)
      - Ensure you also select "Qt Charts" during the installation under 'Additional Libraries'
   - Note the installation path (typically `C:\Qt\6.10.0\msvc2022_64`)

2. **Install GLPK**
   - Download GLPK 4.65 from [SourceForge](https://sourceforge.net/projects/winglpk/)
   - Extract the archive to `C:\glpk-4.65`

3. **Install vcpkg and dependencies**
   ```powershell
   cd C:\
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg install sqlite3:x64-windows curl:x64-windows
   ```

4. **Install Visual Studio with C++ Desktop Development workload**
   - Download from [Visual Studio](https://visualstudio.microsoft.com/downloads/)
   - Ensure you select "Desktop development with C++" during installation

5. **Install CMake**
   - Download from [CMake website](https://cmake.org/download/)
   - Add CMake to your system PATH during installation

6. **Clone the repository**
   ```powershell
   git clone https://github.com/rutgerkool/elometry.git
   cd elometry
   ```

## **Building the project**

### **Linux/macOS**
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### **Windows**
```powershell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DQt6_DIR="C:\Qt\6.10.0\msvc2022_64\lib\cmake\Qt6"
cmake --build . --config Release
```

## **Running the project**

### **Linux/macOS (in `/build`)**
```bash
./Elometry
```

### **Windows (in `/build`)**
```powershell
# Copy necessary DLLs
copy C:\glpk-4.65\w64\glpk_4_65.dll .\Release\
copy C:\vcpkg\installed\x64-windows\bin\sqlite3.dll .\Release\

# Deploy Qt dependencies
C:\Qt\6.10.0\msvc2022_64\bin\windeployqt.exe .\Release\Elometry.exe

# Run the application
.\Release\Elometry.exe
```

## **Configuration**

### **Kaggle Dataset**
To allow for automated updates, Kaggle API credentials must be submitted in the settings when running the program. 
- Open 'Settings' when running the application
- Enter Kaggle API credentials
  - Kaggle Username
  - Kaggle API Key

*Note: Generate API key from [Kaggle Account Settings](https://www.kaggle.com/)*

## **Performance Optimizations**
- Batch SQL operations
- Indexed queries
- Parallel ELO calculations
- Integer Linear Programming optimization

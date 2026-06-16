# JM GUI C++ Apps

This repo contains small native Windows C++ desktop applications built with CMake and Raylib.

## Apps

- `TaskListApp`: enter tasks, add them to a polished Raylib list view, then sort that list alphabetically.
- `CGPACalculator`: enter a student's courses, credits, and grades in a Raylib dashboard to calculate total credits, total grade points, semester GPA, CGPA, and the student's overall course grade.
- `SnakeGame`: play a Raylib snake game with a neon orchard color theme, keyboard controls, score tracking, pause, and restart.
- `BreakBricksGame`: play a colorful one-player brick breaker with power bricks for ball speed, paddle size, double points, and extra balls.

## Build

From a Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Raylib is downloaded into the build tree during CMake configuration. To build a single target:

```powershell
cmake -S . -B build
cmake --build build --config Release --target TaskListApp
cmake --build build --config Release --target CGPACalculator
cmake --build build --config Release --target SnakeGame
cmake --build build --config Release --target BreakBricksGame
```

The executables will be created under:

```text
build\Release\TaskListApp.exe
build\Release\CGPACalculator.exe
build\Release\SnakeGame.exe
build\Release\BreakBricksGame.exe
```

## Run An App

Run the app you want by launching its executable:

```powershell
.\build\Release\TaskListApp.exe
.\build\Release\CGPACalculator.exe
.\build\Release\SnakeGame.exe
.\build\Release\BreakBricksGame.exe
```

You can also build and run a single app target:

```powershell
cmake --build build --config Release --target TaskListApp
.\build\Release\TaskListApp.exe

cmake --build build --config Release --target CGPACalculator
.\build\Release\CGPACalculator.exe

cmake --build build --config Release --target SnakeGame
.\build\Release\SnakeGame.exe

cmake --build build --config Release --target BreakBricksGame
.\build\Release\BreakBricksGame.exe
```

## Task List App

Type a task in the input field, then click `Add` or press `Enter`. Each task is added as a new row in the list. Click `Sort A-Z` / `Sort Z-A` to toggle alphabetical order.

## CGPA Calculator

Enter the student's name, then add each course with its credit count and earned grade. The course table shows each individual course grade and grade points. The summary panel updates the number of courses, total credits, total grade points, semester GPA, CGPA, and overall course grade.

## Snake Game

Use the arrow keys or `WASD` to steer the snake toward the citrus food. Press `Space` to pause and `R` to restart after a collision. The game uses a neon orchard theme with ink backgrounds, mint snake segments, citrus food, and coral game-over accents.

## Break Bricks Game

Move the paddle with the mouse or `A` / `D` and the arrow keys. Break every brick while keeping at least one ball alive. Colored attribute bricks can speed up or slow down the balls, make the paddle larger or smaller, enable double points, or add an extra ball. Press `Space` to restart after clearing the board or losing all lives.

## Tests

The CGPA calculation logic has a small test executable registered with CTest:

```powershell
ctest --test-dir build -C Release
```

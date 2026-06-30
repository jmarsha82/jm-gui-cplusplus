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

Unit tests live in one unified location: `tests/`. The current CTest target covers the shared CGPA calculation core in `src/cgpa/cgpa_core.hpp`, including grade-point mapping, overall grade boundaries, GPA summary math, empty input, and negative-credit handling.

Build and run the unit tests:

```powershell
cmake -S . -B build
cmake --build build --config Release --target CgpaCoreTests
ctest --test-dir build -C Release --output-on-failure
```

The CI coverage job uses `gcovr` on Ubuntu with GCC coverage flags and enforces at least 90% line coverage for the unit-tested `src/cgpa` core:

```bash
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage -O0 -g" -DCMAKE_EXE_LINKER_FLAGS="--coverage"
cmake --build build-coverage --target CgpaCoreTests
ctest --test-dir build-coverage --output-on-failure
gcovr --root . --filter "src/cgpa" --exclude "tests" --txt --fail-under-line 90
```

## CI Pipeline

The GitHub Actions pipeline is defined in `.github/workflows/ci.yml` and runs on pushes to `main` / `master`, pull requests, and manual dispatches.

- `Unit Tests`: configures CMake, builds the `CgpaCoreTests` executable, and runs the registered CTest suite.
- `Unit Test Coverage`: runs the same unit target with GCC coverage instrumentation and fails if line coverage for `src/cgpa` drops below 90%.
- `Code Scanning / Quality`: runs GitHub CodeQL with the `security-and-quality` query suite so maintainability and correctness findings are reported in GitHub code scanning.
- `Code Scanning / Security`: runs GitHub Dependency Review on pull requests and GitHub CodeQL with the `security-extended` query suite so security-oriented findings stay separate from quality findings.

CodeQL code scanning and Dependency Review are available at no cost for public repositories. Private repositories may require GitHub Advanced Security to publish code scanning and dependency alerts, depending on the account and repository settings.

The main test command used by the pipeline is:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

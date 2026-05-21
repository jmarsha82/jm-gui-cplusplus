# JM GUI C++ Apps

This repo contains small native Windows C++ desktop applications built with CMake and the Windows API.

## Apps

- `TaskListApp`: enter tasks, add them to a visible list, and sort that list alphabetically by clicking the `Name` header.
- `CGPACalculator`: enter a student's courses, credits, and grades to calculate total credits, total grade points, semester GPA, CGPA, and the student's overall course grade.

## Build

From a Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The executables will be created under:

```text
build\Release\TaskListApp.exe
build\Release\CGPACalculator.exe
```

## Run An App

Run the app you want by launching its executable:

```powershell
.\build\Release\TaskListApp.exe
.\build\Release\CGPACalculator.exe
```

## Task List App

Type a task in the input field, then click `Add` or press `Enter`. Each task is added as a new row in the `Name` list. Click the `Name` header to sort tasks alphabetically.

## CGPA Calculator

Enter the student's name, then add each course with its credit count and earned grade. The course table shows each individual course grade and grade points. The summary panel updates the number of courses, total credits, total grade points, semester GPA, CGPA, and overall course grade.

## Tests

The CGPA calculation logic has a small test executable registered with CTest:

```powershell
ctest --test-dir build -C Release
```

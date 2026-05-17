# Task List App

A small native Windows C++ desktop application for entering tasks, adding them to a visible list, and sorting that list alphabetically by clicking the `Name` header.

## Build

This project uses CMake and the Windows API. From a Visual Studio Developer PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The executable will be created under:

```text
build\Release\TaskListApp.exe
```

## Use

Type a task in the input field, then click `Add` or press `Enter`. Each task is added as a new row in the `Name` list. Click the `Name` header to sort tasks alphabetically.

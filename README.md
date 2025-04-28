C++ Dynamic Asynchronous Task Scheduler with C Integration
---

This project implements a **real-time dynamic asynchronous task scheduler** in modern C++20, fully integrated with C libraries via `void*` handlers.

* Scheduler (dynamic, real-time, priority-based)  
* C Library Integration (pure C `void*` handler)  
* Catch2 Unit Tests (header-only)  
* Visual Studio Code Ready  
* CLion Ready  
* Docker Ready  
* GitHub Actions CI/CD Ready

---

## Features

### Scheduler Core

- Dynamic task add/remove at runtime
- Task handler (`void*`) mandatory — C compatible
- Priority-based scheduling
- Interval-based execution (e.g., every X ms)
- Delayed task start
- Automatic timeout detection
- State machine per task:
  - `Ready`, `Waiting`, `Running`, `Done`, `Failed`, `Timeout`, `Paused`
- Retry failed tasks N times automatically
- Pause and resume tasks dynamically
- Task dependency management (run after another task finishes)
- Efficient event-driven loop (`std::condition_variable`)
- Multiple independent Scheduler instances
- Fast task lookup by handler

### IDE and Tools Support

- **Visual Studio Code**: Build, debug, IntelliSense ready
- **CLion**: Fully supported
- **Docker**: Build and run in container
- **GitHub Actions CI**: Automatic build, test, validate

---

## Project Structure

```plaintext
/ProjectRoot
 ├── CMakeLists.txt           # CMake build script
 ├── Dockerfile               # Build project in Docker
 ├── README.md                 # This documentation
 ├── scheduler/                # Task scheduler source (C++)
 ├── c_library/                # C library code (C)
 ├── src/                      # Main application
 ├── tests/                    # Catch2 unit tests
 └── .vscode/                  # VSCode config for CMake Tools
    └── settings.json, launch.json, c_cpp_properties.json
 └── .github/
    └── workflows/
        └── ci.yml             # GitHub Actions CI/CD pipeline
```

---

## How to Build and Run

### Local Build (Linux, Mac, Windows)

```bash
mkdir build
cd build
cmake ..
make
./main
```

### Run Unit Tests

```bash
mkdir build
cd build
cmake ..
make
ctest --verbose
```

### Build and Run with Docker

```bash
docker build -t scheduler_project .
docker run --rm scheduler_project
```

### VSCode Usage

1. Install extensions:
   - CMake Tools
   - C++ Extension (ms-vscode.cpptools)
2. Open Folder.
3. Press "CMake: Build" ➔ Press F5 to run/debug.

---

## How to Create and Run a Task

### 1. Write a Task Function

```cpp
TaskState myTaskFunction(void* handler) {
    MyCStruct* ctx = static_cast<MyCStruct*>(handler);
    ctx->value++;
    return (ctx->value >= 5) ? TaskState::Done : TaskState::Running;
}
```

### 2. Add a Task to Scheduler

```cpp
Scheduler scheduler("MainScheduler");
scheduler.start();

MyCStruct ctx = {42, "TaskA"};
scheduler.addTask(myTaskFunction, &ctx, 500, 0, 5000, 5, 1);

scheduler.stop();
```

---

## Dependencies

- CMake ≥ 3.15
- C++17 compiler
- spdlog (for logging)

(Install with `sudo apt install libspdlog-dev` or vcpkg.)

---

## GitHub Actions (CI/CD)

- Every Push / Pull Request:
  - Auto Build via CMake
  - Auto Run Unit Tests (Catch2)
  - Validate before merge

Workflow file: `.github/workflows/ci.yml`

---

## Future Upgrades

- Code Coverage reporting
- Auto Retry with backoff
- Task progress reporting (percent complete)
- Event queue for task communications
- Group task execution pipelines
- Full documentation website

---

## Authors

Alexander Sacharov and 

---

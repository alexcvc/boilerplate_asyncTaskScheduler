# C++20 Real-Time Dynamic Event Scheduler

This project delivers a modern, extensible **asynchronous event scheduler** in C++20, designed for real-time systems and
seamless C/C++ interoperability.

- Dynamic event scheduling and management at runtime
- C and C++ integration via `void*` handlers and interfaces
- interval, and delayed event execution
- Comprehensive state machine for event lifecycle
- Automatic timeout, retry
- Multiple independent scheduler instances
- Fast event lookup and management
- Extensive unit tests with Catch2
- Ready for CLion, VSCode, Docker, and CI/CD pipelines

---

## Features

### Scheduler Core

- Add/remove events dynamically at runtime
- C/C++ compatible event handlers (`void*` and interfaces)
- Priority, interval, and delayed event execution
- State machine per event:
    - `Ready`, `Waiting`, `Running`, `Done`, `Failed`, `Timeout`, `Paused`
- Automatic timeout, retry, and dependency management
- Pause/resume and abort events dynamically
- Multiple independent scheduler instances
- Fast event lookup and management

### IDE and Tools Support

- **CLion**: Fully supported
- **Visual Studio Code**: Build, debug, IntelliSense ready
- **Docker**: Build and run in container
- **GitHub Actions CI**: Automatic build, test, validate

---

## Project Structure

```plaintext
/ProjectRoot
 ├── CMakeLists.txt           # CMake build script
 ├── Dockerfile               # Build project in Docker
 ├── README.md                # This documentation
 ├── include/                 # C++ event scheduler headers
 ├── src/                     # Main application
 ├── test/                    # Catch2 unit tests
 └── .github/
    └── workflows/
        └── ci.yml            # GitHub Actions CI/CD pipeline
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

### CLion & VSCode Usage

- Open the project folder in your IDE.
- Configure CMake profile if needed.
- Build and run/debug as usual.

---

## How to Create and Run an Event

### 1. Implement Controller and User Data

```cpp
class MyController : public IController {
  void handleEvent() override { /* ... */ }
  void stopEvent() override { /* ... */ }
  void abortEvent() override { /* ... */ }
};

class MyUserData : public IUserData {
  uint32_t getEventType() const override { return 1; }
  std::any getUserParameter() const override { return std::string("param"); }
};
```

### 2. Add an Event to Scheduler

```cpp
Scheduler scheduler;
auto controller = std::make_shared<MyController>();
auto userData = std::make_shared<MyUserData>();

EventConfig config{/* delay, serve, life, callbacks... */};
auto event = scheduler.pushEvent(controller, userData, config);

scheduler.start();
scheduler.service();
scheduler.terminate();
```

---

## Dependencies

- CMake ≥ 3.15
- C++20 compiler
- spdlog (for logging)
- Catch2 (for unit testing)

---

## GitHub Actions (CI/CD)

- Every push/pull request:
    - Auto build via CMake
    - Auto run unit tests (Catch2)
    - Validate before merge

Workflow file: `.github/workflows/ci.yml`

---

## Future Upgrades

- Code coverage reporting
- Auto retry with backoff
- Event progress reporting
- Event queue for inter-event communication
- Group event execution pipelines
- Full documentation website

---

## Authors

Alexander Sacharov and contributors

---
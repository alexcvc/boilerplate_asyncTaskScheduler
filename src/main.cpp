#include <any>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "scheduler.hpp"

// Assuming relevant classes/interfaces are in namespace tev
using namespace tev;
using namespace std::chrono_literals;

// Custom controller implementing IController
class MyController : public IController {
 public:
  void handleEvent() {
    std::cout << "Event handled.\n";
  }
  void stopEvent() {
    std::cout << "Event stopped.\n";
  }
  void abortEvent() {
    std::cout << "Event aborted.\n";
  }
};

// Custom user data implementing IUserData
class MyUserData : public IUserData {
 public:
  [[nodiscard]] uint32_t getEventType() const {
    return 1;  // Sample event type
  }
  [[nodiscard]] std::any getUserParameter() const {
    return std::string("Sample parameter");
  }
};

int main() {
  // Instantiate scheduler
  Scheduler scheduler;

  // Create controller and user data instances
  auto controller = std::make_shared<MyController>();
  auto userData = std::make_shared<MyUserData>();

  // Configure event callbacks
  EventConfig config{2000ms,  // delayMs
                     1000ms,  // serveMs
                     8000ms,  // lifeMs
                     // startCallback
                     []([[maybe_unused]] EventPtr event) -> DurationUnit {
                       std::cout << "*** Event started\n";
                       return 1000ms;
                     },
                     // eventCallback
                     []([[maybe_unused]] EventPtr event) -> DurationUnit {
                       std::cout << "*** Event running\n";
                       return 1000ms;
                     },
                     // abortCallback
                     []([[maybe_unused]] EventPtr event) -> DurationUnit {
                       std::cout << "*** Event aborted\n";
                       return 0ms;
                     },
                     // completeCallback
                     []([[maybe_unused]] EventPtr event) -> DurationUnit {
                       std::cout << "*** Event completed\n";
                       return 0ms;
                     },
                     // timeoutCallback
                     []([[maybe_unused]] EventPtr event) -> DurationUnit {
                       std::cout << "*** Event timed out\n";
                       return 0ms;
                     }};

  // Push event to scheduler
  auto event = scheduler.pushEvent(controller, userData, config);

  // Start scheduler (usually runs in its own thread)
  if (!scheduler.start()) {
    std::cerr << "Failed to start scheduler\n";
    return 1;
  }

  // Simulate servicing events for a while
  for (int i = 0; i < 100; i++) {
    auto timeout = 5000ms;
    auto sooner = scheduler.service();
    if (timeout > sooner) {
      timeout = sooner;
    }
    std::cout << ">>> Next event in: " << timeout.count() << " ms\n";
    std::this_thread::sleep_for(timeout);
  }

  // Terminate scheduler after done
  scheduler.terminate();

  return 0;
}

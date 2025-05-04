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
    std::cout << "Hi from controller *** event served\n";
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
  // Create an array of 5 EventConfig objects with staggered delays
  EventConfig configs[5] = {
      {0ms, 1100ms, 6666ms, /* start */
       [](EventPtr e) {
         std::cout << "Event 0 started. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       /* run */
       [](EventPtr e) {
         std::cout << "Event 0 running. Processing intervall " << e->getServeInterval().count() << " ms\n";
         std::shared_ptr<MyController> myController = std::dynamic_pointer_cast<MyController>(e->getController());
         if (myController) {
           // successful cast, safe to use myController
           myController->handleEvent();
         } else {
           // cast failed, handle error
         }
       },
       /* abort */
       [](EventPtr e) {
         std::cout << "Event 0 aborted. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       /* complete */
       [](EventPtr e) {
         std::cout << "Event 0 completed. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       /* timeout */
       [](EventPtr e) {
         std::cout << "Event 0 timed out. Processing intervall " << e->getServeInterval().count() << " ms\n";
       }},
      {200ms, 888ms, 7050ms,
       [](EventPtr e) {
         std::cout << "Event 1 started. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 1 running. Processing intervall " << e->getServeInterval().count() << " ms\n";
         std::shared_ptr<MyController> myController = std::dynamic_pointer_cast<MyController>(e->getController());
         if (myController) {
           // successful cast, safe to use myController
           myController->handleEvent();
         } else {
           // cast failed, handle error
         }
       },
       [](EventPtr e) {
         std::cout << "Event 1 aborted. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 1 completed. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 1 timed out. Processing intervall " << e->getServeInterval().count() << " ms\n";
       }},
      {500ms, 1225ms, 7777ms,
       [](EventPtr e) {
         std::cout << "Event 2 started. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 2 running. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 2 aborted. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 2 completed. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 2 timed out. Processing intervall " << e->getServeInterval().count() << " ms\n";
       }},
      {700ms, 912ms, 8765ms,
       [](EventPtr e) {
         std::cout << "Event 3 started. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 3 running. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 3 aborted. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 3 completed. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 3 timed out. Processing intervall " << e->getServeInterval().count() << " ms\n";
       }},
      {100ms, 902ms, 9876ms,
       [](EventPtr e) {
         std::cout << "Event 4 started. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 4 running. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 4 aborted. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 4 completed. Processing intervall " << e->getServeInterval().count() << " ms\n";
       },
       [](EventPtr e) {
         std::cout << "Event 4 timed out. Processing intervall " << e->getServeInterval().count() << " ms\n";
       }},
  };

  // Start scheduler (usually runs in its own thread)
  if (!scheduler.start()) {
    std::cerr << "Failed to start scheduler\n";
    return 1;
  }

  // Push 5 events to the scheduler
  for (int i = 0; i < 5; ++i) {
    scheduler.pushEvent(controller, userData, configs[i]);
    std::this_thread::sleep_for(1000ms);  // Stagger event creation
  }

  while (scheduler.getEventsCount() > 0) {
    // Process events
  }

  std::this_thread::sleep_for(3s);

  // Terminate scheduler after done
  scheduler.terminate();

  return 0;
}

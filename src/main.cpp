#include <any>
#include <chrono>
#include <cmath>
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
  void startEvent() {
    std::cout << "Hi from controller *** process event started\n";
  }
};

// Custom user data implementing IUserData
struct MyUserData : public IUserData {
  uint32_t counter{0};  // Sample event type
};

// Function to calculate the statistics of jitter
void calculateJitterStatistics(const std::vector<float>& jitterValues) {
  if (jitterValues.empty()) {
    std::cout << "No jitter data collected.\n";
    return;
  }

  // Calculate min, max, and average jitter
  float minJitter = *std::min_element(jitterValues.begin(), jitterValues.end());
  float maxJitter = *std::max_element(jitterValues.begin(), jitterValues.end());
  float sumJitter = 0.0f;
  for (float jitter : jitterValues) {
    sumJitter += jitter;
  }
  float averageJitter = jitterValues.empty() ? 0.0f : sumJitter / static_cast<float>(jitterValues.size());

  // Calculate standard deviation of jitter
  float squaredSum = 0.0f;
  for (float jitter : jitterValues) {
    squaredSum += (jitter - averageJitter) * (jitter - averageJitter);
  }
  float standardDeviation =
      jitterValues.empty() ? 0.0f : std::sqrt(squaredSum / static_cast<float>(jitterValues.size()));

  // Print results
  std::cout << "===================================\n";
  std::cout << "Jitter Statistics for Total Test:\n";
  std::cout << "  Min Jitter: " << minJitter << " ms\n";
  std::cout << "  Max Jitter: " << maxJitter << " ms\n";
  std::cout << "  Average Jitter: " << averageJitter << " ms\n";
  std::cout << "  Standard Deviation: " << standardDeviation << " ms\n";
}

int main() {
  // Instantiate scheduler
  Scheduler scheduler;
  // Vector to store jitter values for each event
  std::vector<float> jitterValues;

  // Create controller and user data instances
  auto controller = std::make_shared<MyController>();
  auto userData = std::make_shared<MyUserData>();

  auto process_print = [&]([[maybe_unused]] EventPtr e) {
    // Get the current time
    auto now = std::chrono::steady_clock::now();
    // Calculate the actual elapsed time since the last processing
    auto actualInterval = now - e->lastProcTimePoint();

    // Calculate jitter: the difference between scheduled and actual time as float milliseconds
    auto jitterDuration =
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(actualInterval - e->getServeInterval());

    // Output the jitter as float ms
    // std::cout << "   > intervall " << e->getServeInterval().count() << " ms. Jitter: " << jitterDuration.count()
    //           << " ms\n";
    // Store jitter in the vector
    jitterValues.push_back(jitterDuration.count());

    std::shared_ptr<MyController> myController = std::dynamic_pointer_cast<MyController>(e->getController());
    if (myController) {
      // successful cast, safe to use myController
      myController->handleEvent();
    } else {
      // cast failed, handle error
    }
    std::shared_ptr<MyUserData> myUserData = std::dynamic_pointer_cast<MyUserData>(e->getUserData());
    if (myUserData) {
      // successful cast, safe to use myController
      myUserData->counter++;
    }
  };

  auto start_print = [=]([[maybe_unused]] EventPtr e) {
    // Get the current time
    auto now = std::chrono::steady_clock::now();
    // Calculate the actual elapsed time since the last processing
    auto actualInterval = now - e->lastProcTimePoint();

    // Calculate jitter: the difference between scheduled and actual time as float milliseconds
    auto jitterDuration =
        std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(e->getStartDelay() - actualInterval);

    // Output the jitter as float ms
    // std::cout << "   > intervall " << e->getStartDelay().count() << " ms. Jitter: " << jitterDuration.count()
    //           << " ms\n";

    std::shared_ptr<MyController> myController = std::dynamic_pointer_cast<MyController>(e->getController());
    if (myController) {
      // successful cast, safe to use myController
      myController->startEvent();
    } else {
      // cast failed, handle error
    }
  };

  auto timeout_print = [&]([[maybe_unused]] EventPtr e) {
    std::shared_ptr<MyUserData> myUserData = std::dynamic_pointer_cast<MyUserData>(e->getUserData());
    if (myUserData) {
      // print
      std::cout << "   > timeout with counter " << myUserData->counter << "\n";
    }
  };

  // Use std::array to store the EventConfig objects for 10 events
  std::array<EventConfig, 10> configs = {{
      {0ms, 1100ms, 6666ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 0 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 0 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 0 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 0 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 0 time out\n";
         timeout_print(e);
       }},
      {200ms, 888ms, 7050ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 1 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 1 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 1 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 1 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 1 time out\n";
         timeout_print(e);
       }},
      {400ms, 1000ms, 7111ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 2 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 2 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 2 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 2 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 2 time out\n";
         timeout_print(e);
       }},
      {600ms, 975ms, 8200ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 3 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 3 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 3 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 3 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 3 time out\n";
         timeout_print(e);
       }},
      {800ms, 950ms, 9000ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 4 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 4 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 4 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 4 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 4 time out\n";
         timeout_print(e);
       }},
      {1000ms, 925ms, 9100ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 5 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 5 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 5 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 5 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 5 time out\n";
         timeout_print(e);
       }},
      {1100ms, 900ms, 9500ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 6 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 6 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 6 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 6 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 6 time out\n";
         timeout_print(e);
       }},
      {1200ms, 875ms, 8050ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 7 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 7 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 7 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 7 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 7 time out\n";
         timeout_print(e);
       }},
      {1300ms, 850ms, 8111ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 8 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 8 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 8 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 8 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 8 time out\n";
         timeout_print(e);
       }},
      {1400ms, 825ms, 7777ms,
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 9 started\n";
         start_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 9 running\n";
         process_print(e);
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 9 aborted\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 9 completed\n";
       },
       [=]([[maybe_unused]] EventPtr e) {
         std::cout << "Event 9 time out\n";
         timeout_print(e);
       }},
  }};

  // Start scheduler (usually runs in its own thread)
  if (!scheduler.start()) {
    std::cerr << "Failed to start scheduler\n";
    return 1;
  }

  // Push 10 events to the scheduler using std::array
  for (size_t i = 0; i < configs.size(); ++i) {
    std::cout << "Pushing event " << i << "\n";
    auto e = scheduler.pushEvent(controller, userData, configs[i]);
    std::this_thread::sleep_for(500ms);  // Stagger event creation
  }

  std::cout << "Waiting for events to be processed\n";
  while (scheduler.getEventsCount() > 0) {
    // Process events
  }

  // Terminate scheduler after done
  std::cout << "Terminating scheduler\n";
  scheduler.terminate();

  // Compute and display jitter statistics for the total test
  calculateJitterStatistics(jitterValues);

  return 0;
}

/*************************************************************************/ /**
 * \file
 * \brief  contains class declarations for delayed events scheduling.
 * \ingroup Scheduled Events
 *****************************************************************************/

#pragma once

//-----------------------------------------------------------------------------
// includes <...>
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// includes "..."
//-----------------------------------------------------------------------------
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include "event.hpp"
#include "iController.hpp"
#include "iUserData.hpp"

namespace tev {

/**
 * @brief Scheduler class for managing timed events.
 */
class Event;
using EventPtr = std::shared_ptr<Event>;
using DurationUnit = std::chrono::milliseconds;
using namespace std::chrono_literals;

/**
 * @brief Manages and schedules tasks or events to be executed at specified times.
 *
 * The Scheduler class allows the registration, rescheduling, and cancellation
 * of tasks. It is designed to handle concurrency and ensure that tasks
 * are executed at the appropriate time intervals or deadlines. It supports
 * delayed execution, periodic tasks.
 */
class Scheduler {
 public:
  static constexpr DurationUnit kMaxDelayIntervalMs{10000ms};

  Scheduler() = default;
  virtual ~Scheduler() = default;
  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;

  [[nodiscard]] DurationUnit service();

  void pushEvent(std::shared_ptr<Event> event);
  [[nodiscard]] std::shared_ptr<Event> pushEvent(std::shared_ptr<IController> controller,
                                                 std::shared_ptr<IUserData> userData, const EventConfig& config);

  void eraseEvent(std::shared_ptr<Event> event);
  void eraseEvent(std::shared_ptr<IUserData> userData);

  bool start();

  void wakeUp() {
    std::lock_guard lock(m_CondMutex);
    m_CondEvent.notify_one();
  }

  void terminate();

  [[nodiscard]] DurationUnit getMaxIdleTimeout() const noexcept {
    return m_MaxDelayIntervalMs;
  }

  void setMaxIdleTimeout(DurationUnit maxIdleTimeout) {
    m_MaxDelayIntervalMs = maxIdleTimeout;
  }

 private:
  std::mutex m_Mutex;                                      ///< Protects access to shared resources
  std::mutex m_CondMutex;                                  ///< Guards condition variable synchronization
  std::condition_variable m_CondEvent;                     ///< Notifies scheduler thread of events or termination
  std::list<std::shared_ptr<Event>> m_scheduledEvents;     ///< Stores scheduled events
  std::jthread m_Thread;                                   ///< Runs the event scheduler's service loop
  DurationUnit m_MaxDelayIntervalMs{kMaxDelayIntervalMs};  ///< Max idle timeout interval
};

}  // end of namespace tev

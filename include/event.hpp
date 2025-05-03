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
#include <functional>
#include <stopTimer.hpp>

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

/// Callback function signature for timed events related to controllers.
/// Parameters:
/// - controller: a shared pointer to the controller object.
/// - event: a shared pointer to the timed event object.
/// Returns the duration of the next timed action.
using ControllerEventCallback = std::function<DurationUnit(EventPtr event)>;

/**
 * @brief Configuration structure for defining parameters of a timed event.
 *
 * The EventConfig structure is used to initialize and specify behavior for a timed event,
 * including delay, duration, lifecycle, and associated callback functions.
 */
struct EventConfig {
  DurationUnit delayMs;
  DurationUnit serveMs;
  DurationUnit lifeMs;
  ControllerEventCallback startCallback;
  ControllerEventCallback eventCallback;
  ControllerEventCallback abortCallback;
  ControllerEventCallback completeCallback;
  ControllerEventCallback timeoutCallback;

  EventConfig(DurationUnit delayMs, DurationUnit serveMs, DurationUnit lifeMs,
              const ControllerEventCallback& startCallback, const ControllerEventCallback& eventCallback,
              const ControllerEventCallback& abortCallback, const ControllerEventCallback& completeCallback,
              const ControllerEventCallback& timeoutCallback)
      : delayMs(delayMs),
        serveMs(serveMs),
        lifeMs(lifeMs),
        startCallback(startCallback),
        eventCallback(eventCallback),
        abortCallback(abortCallback),
        completeCallback(completeCallback),
        timeoutCallback(timeoutCallback) {}
};

/**
 * @brief A class representing a scheduled event with customizable lifecycle and behavior.
 *
 * The Event class provides functionality to manage timed events, including their start delay,
 * execution interval, maximum lifespan, and associated callbacks for different stages of
 * event execution. It supports user-defined data and integrates with a controlling entity.
 */
class Event : public std::enable_shared_from_this<Event> {
 public:
  using EventClock = tkcppsl::time::StopTimer<>;
  static constexpr DurationUnit kDefaultIntervalMs{1000ms};
  static constexpr DurationUnit kDefaultLifeMs{60000ms};
  static constexpr DurationUnit kDefaultDelayDuration{std::chrono::milliseconds::min()};
  static constexpr DurationUnit kDefaultEndlessLifeMs{std::chrono::milliseconds::max()};

  // Status Enum
  enum class Status { Pending, Running, Completed, Aborted, Timeouted };

  // Constructors
  Event() = default;
  Event(const Event& other) = default;

  Event(const std::shared_ptr<IController>& controller, const std::shared_ptr<IUserData>& userData,
        const EventConfig& config)
      : m_Controller(controller),
        m_UserData(userData),
        m_StartDelay(config.delayMs),
        m_ServeInterval(config.serveMs),
        m_MaxLifeDuration(config.lifeMs) {
    // set func
    m_StartFunc = config.startCallback;
    m_EventFunc = config.eventCallback;
    m_AbortFunc = config.abortCallback;
    m_CompleteFunc = config.completeCallback;
    m_TimeoutFunc = config.timeoutCallback;
    // set timeout
    m_EventClock.SetTimeout(m_StartDelay.count() != 0 ? m_StartDelay : m_ServeInterval);
    m_LifeClock.SetTimeout(m_MaxLifeDuration);
  }

  Event& operator=(const Event& other) = default;

  virtual ~Event() = default;

  [[nodiscard]] std::shared_ptr<IController> getController() {
    return m_Controller;
  }
  [[nodiscard]] Status getStatus() {
    return m_Status;
  }
  [[nodiscard]] DurationUnit& getStartDelay() {
    return m_StartDelay;
  }
  void setStartDelay(const DurationUnit& startDelay) {
    m_StartDelay = startDelay;
  }
  [[nodiscard]] DurationUnit& getServeInterval() {
    return m_ServeInterval;
  }
  void setServeInterval(const DurationUnit& serveInterval) {
    m_ServeInterval = serveInterval;
  }
  [[nodiscard]] DurationUnit& getLifeDuration() {
    return m_MaxLifeDuration;
  }
  [[nodiscard]] EventClock& getEventClock() {
    return m_EventClock;
  }
  [[nodiscard]] EventClock& getLifeClock() {
    return m_LifeClock;
  }
  [[nodiscard]] std::shared_ptr<IUserData>& getUserData() {
    return m_UserData;
  }
  void setStatus(Status status) {
    m_Status = status;
  }
  void setStartDelay1(const DurationUnit& startDelay) {
    m_StartDelay = startDelay;
  }
  void setServeInterval1(const DurationUnit& serveInterval) {
    m_ServeInterval = serveInterval;
  }
  void setMaxLifeDuration(const DurationUnit& maxLifeDuration) {
    m_MaxLifeDuration = maxLifeDuration;
  }
  [[nodiscard]] ControllerEventCallback& getStartFunc() {
    return m_StartFunc;
  }
  void setStartFunc(const ControllerEventCallback& func) {
    m_StartFunc = func;
  }
  [[nodiscard]] ControllerEventCallback& getEventFunc() {
    return m_EventFunc;
  }
  void setEventFunc(const ControllerEventCallback& func) {
    m_EventFunc = func;
  }
  [[nodiscard]] ControllerEventCallback& getAbortFunc() {
    return m_AbortFunc;
  }
  void setAbortFunc(const ControllerEventCallback& func) {
    m_AbortFunc = func;
  }
  [[nodiscard]] ControllerEventCallback& getCompleteFunc() {
    return m_CompleteFunc;
  }
  void setCompleteFunc(const ControllerEventCallback& func) {
    m_CompleteFunc = func;
  }
  [[nodiscard]] ControllerEventCallback& getTimeoutFunc() {
    return m_TimeoutFunc;
  }
  void setTimeoutFunc(const ControllerEventCallback& func) {
    m_TimeoutFunc = func;
  }

 private:
  std::shared_ptr<IController> m_Controller;              ///< Associated controller
  std::shared_ptr<IUserData> m_UserData;                  ///< Encapsulated user-defined event data
  Status m_Status{Status::Pending};                       ///< Status of the event
  DurationUnit m_StartDelay{kDefaultDelayDuration};       ///< Optional delay before the event starts
  DurationUnit m_ServeInterval{kDefaultIntervalMs};       ///< Interval for serving this event
  DurationUnit m_MaxLifeDuration{kDefaultEndlessLifeMs};  ///< Maximum lifespan of the event
  EventClock m_EventClock;                                ///< Clock object for timing the event
  EventClock m_LifeClock;                                 ///< Clock object for lifetime tracking
  ControllerEventCallback m_StartFunc{nullptr};           ///< Function pointer for handling start event execution
  ControllerEventCallback m_EventFunc{nullptr};           ///< Function pointer for handling event execution
  ControllerEventCallback m_AbortFunc{nullptr};           ///< Function pointer for stopping the event
  ControllerEventCallback m_CompleteFunc{nullptr};        ///< Callback function executed on event completion
  ControllerEventCallback m_TimeoutFunc{nullptr};         ///< Callback executed on timeout
};

}  // end of namespace tev

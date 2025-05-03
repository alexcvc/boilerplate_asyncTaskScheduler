#pragma once

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include <chrono>
#include <optional>

//----------------------------------------------------------------------------
// Public Prototypes
//----------------------------------------------------------------------------

namespace tkcppsl::time {

/**
* @brief This is a easy stop timer in C++11 class.
* Timer allows to set timeout and to check elapsed of timer
*
*/
template <class TDuration = std::chrono::milliseconds>
class StopTimer {
 public:
  /** types */
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<StopTimer::Clock, TDuration>;

  /**
   * @brief constructors
   */
  StopTimer() = default;
  StopTimer(TDuration timeout) : m_timeout_duration(timeout) {}

  /**
   * @brief get timeout
   * @tparam TUnit - to-duration unit type
   * @return duration in to-duration unit type
   */
  template <typename TUnit = TDuration>
  [[nodiscard]] TUnit Timeout() const noexcept {
    return std::chrono::duration_cast<TUnit>(m_timeout_duration);
  }

  /**
   * @brief set timeout in a duration unit
   * @tparam TUnit - from-duration unit type
   * @param timeout
   */
  template <typename TUnit = TDuration>
  void SetTimeout(const TUnit& timeout) noexcept {
    m_timeout_duration = std::chrono::duration_cast<TDuration>(timeout);
  }

  /**
   * @brief is timer has been started (pushed)
   * @return true if it is running, otherwise - false
   */
  [[nodiscard]] bool IsRunning() const noexcept {
    return m_is_running;
  }

  /**
   * @brief stop running
   * @details reset a running flag and reset start point in timer
   */
  void Reset() noexcept {
    m_is_running = false;
    m_start_point = {};
  }

  /**
   * @brief stop running
   * @details stop running only without a reset start point
   */
  void Stop() noexcept {
    m_is_running = false;
  }

  /**
   * @brief restart timer
   * @details restart the timer with setup start point to now time point
   * @return start time point
   */
  TimePoint Start() noexcept {
    m_is_running = true;
    m_start_point = StopTimer::CurrentTime();
    return m_start_point;
  }

  /**
   * @brief start timer with setup timeout
   * @details start timer with reset start point to now time point and new timeout
   * @tparam TUnit - duration unit
   * @param new_timeout - new timeout in duration unit
   * @return start time point
   */
  template <typename TUnit = TDuration>
  TimePoint Start(TUnit new_timeout) noexcept {
    SetTimeout<TUnit>(new_timeout);
    m_is_running = true;
    m_start_point = StopTimer::CurrentTime();
    return m_start_point;
  }

  /**
   * @brief is a timeout interval elapsed
   * @brief check that a defined timeout interval was elapsed
   * @return optional boolean
   *        1. is not running - has no value - std::nullopt
   *        2. running but timeout is equal zero - true
   *        3. running with timeout > zero: true if timeout elapsed, otherwise - false
   */
  [[nodiscard]] std::optional<bool> IsElapsed() noexcept {
    if (!m_is_running) {
      // timer is not running
      return std::nullopt;
    } else if (m_timeout_duration.count() == 0) {
      // is running with timeout 0
      return true;
    } else {
      return (ElapsedTime() > m_timeout_duration);
    }
  }

  /**
   * @brief Checks if the timer is running and has elapsed time.
   * @return true if the timer was started and the time is elapsed.
   */
  [[nodiscard]] bool IsRunningAndElapsed() noexcept {
    auto timeoutStatus = this->IsElapsed();
    return timeoutStatus.has_value() && timeoutStatus.value();
  }

  /**
   * @brief elapsed timeout
   * @details elapsed timeout since start
   * @tparam TUnit - duration unit
   * @return elapsed time since start point
   */
  template <typename TUnit = TDuration>
  [[nodiscard]] TUnit ElapsedTime() noexcept {
    if (IsRunning()) {
      return std::chrono::duration_cast<TUnit>(StopTimer::CurrentTime() - m_start_point);
    } else {
      return TUnit{};
    }
  }

  /**
   * @brief left time
   * @details left time up to timeout point
   * @tparam TUnit - duration unit
   * @return left time
   */
  template <typename TUnit = TDuration>
  [[nodiscard]] TUnit LeftTime() noexcept {
    if (IsRunning()) {
      return std::chrono::duration_cast<TUnit>(m_timeout_duration - ElapsedTime());
    } else {
      return TUnit{};
    }
  }

  /**
   * @brief current time for used clock from chrono
   * @details static function used many times in class
   * @return timepoint
   */
  [[nodiscard]] static inline TimePoint CurrentTime() noexcept {
    return std::chrono::time_point_cast<TDuration>(Clock::now());
  }

 private:
  StopTimer::TimePoint m_start_point{};  ///< start time point
  TDuration m_timeout_duration{};        ///< timeout
  bool m_is_running{false};              ///< is running
};

/**
 * @brief useful types
 * @details useful types for replace template type like standard library std::string
 * TimerSec timerSec;
 * TimerMs timerMs;
 * TimerUs timerUs;
 */
using TimerSec = tkcppsl::time::StopTimer<std::chrono::seconds>;
using TimerMs = tkcppsl::time::StopTimer<std::chrono::milliseconds>;
using TimerUs = tkcppsl::time::StopTimer<std::chrono::microseconds>;

}  // namespace tkcppsl::time

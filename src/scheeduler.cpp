/* SPDX-License-Identifier: A-EBERLE */
/****************************************************************************\
**                   _        _____ _               _                      **
**                  / \      | ____| |__   ___ _ __| | ___                 **
**                 / _ \     |  _| | '_ \ / _ \ '__| |/ _ \                **
**                / ___ \ _  | |___| |_) |  __/ |  | |  __/                **
**               /_/   \_(_) |_____|_.__/ \___|_|  |_|\___|                **
**                                                                         **
*****************************************************************************
** Copyright (c) 2010 - 2024 A. Eberle GmbH & Co. KG. All rights reserved. **
\****************************************************************************/

/*************************************************************************/ /**
 * \file
 * \brief  contains class implementations timed events.
 *****************************************************************************/

//-----------------------------------------------------------------------------
// includes <...>
//-----------------------------------------------------------------------------
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>

#include "scheduler.hpp"

using namespace std::chrono_literals;

//----------------------------------------------------------------------------
// Private Defines and Macros
//----------------------------------------------------------------------------
namespace tev {

bool Scheduler::start() {
  if (m_Thread.get_id() != std::jthread::id{}) {
    return false;  // Scheduler is already running
  }
  m_Thread = std::jthread([this](const std::stop_token& stop_token) {
    // Register a stop callback
    std::stop_callback stopCb(stop_token, [&]() {
      // Wake thread on stop request
      m_CondEvent.notify_all();
    });

    while (true) {
      // serve events
      DurationUnit waitTime = processEvents(m_MaxInterval);
      // wait next serve
      {
        std::unique_lock lock(m_CondMutex);
        m_CondEvent.wait_for(lock, waitTime);
      }
      //Stop if requested to stop
      if (stop_token.stop_requested()) {
        break;
      }
    }
  });

  return true;
}

void Scheduler::terminate() {
  if (m_Thread.get_id() != std::jthread::id{}) {
    // Request the thread to stop
    m_Thread.request_stop();
  }
}

void Scheduler::pushEvent(std::shared_ptr<Event> event) {
  const std::lock_guard lg(m_Mutex);

  // set now
  event->setLastProcTimePoint(std::chrono::steady_clock::now());
  // set life clock
  if (event->getEventClock().Timeout<>() != std::chrono::milliseconds::min()) {
    event->getEventClock().Start();
  }
  if (event->getLifeClock().Timeout<>() != std::chrono::milliseconds::min()) {
    event->getLifeClock().Start();
  }
  // set start delay
  if (event->getStartDelay() != std::chrono::milliseconds::min()) {
    event->setStatus(Event::Status::Pending);
  } else {
    event->setStatus(Event::Status::Running);
  }
  // add event to the list
  m_scheduledEvents.push_back(event);
  // notify the scheduler thread
  wakeUp();
}

std::shared_ptr<Event> Scheduler::pushEvent(std::shared_ptr<IController> controller,
                                            std::shared_ptr<IUserData> userData, const EventConfig& config) {
  auto newEvent = std::make_shared<Event>(controller, userData, config);

  pushEvent(newEvent);
  return newEvent;
}

void Scheduler::eraseEvent(std::shared_ptr<Event> event) {
  const std::lock_guard lg(m_Mutex);
  std::erase_if(m_scheduledEvents, [&](const std::shared_ptr<Event>& event_item) {
    if (event_item == event) {
      return true;  // Remove event from a list
    }
    return false;
  });
}

void Scheduler::eraseEvent(std::shared_ptr<IUserData> userData) {
  if (userData) {
    const std::lock_guard lg(m_Mutex);
    std::erase_if(m_scheduledEvents, [&](const std::shared_ptr<Event>& event_item) {
      if (userData == event_item->getUserData()) {
        return true;  // Remove event from a list
      }
      return false;
    });
  }
}

/**
 * @brief Service function to process timed events.
 * @return Minimum delay for the next event.
 */
std::chrono::milliseconds Scheduler::processEvents(std::chrono::milliseconds processingTime) {
  // invoke start function
  [[maybe_unused]] auto invokeStartFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getStartFunc()) {
      it->getStartFunc()(it);
    }
    it->setLastProcTimePoint(std::chrono::steady_clock::now());
  };
  // invoke event function
  [[maybe_unused]] auto invokeEventFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getEventFunc()) {
      it->getEventFunc()(it);
    }
    it->setLastProcTimePoint(std::chrono::steady_clock::now());
  };
  // invoke timeout function
  [[maybe_unused]] auto invokeTimeoutFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getTimeoutFunc()) {
      it->getTimeoutFunc()(it);
    }
    it->setLastProcTimePoint(std::chrono::steady_clock::now());
  };
  // invoke complete function
  [[maybe_unused]] auto invokeCompleteFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getCompleteFunc()) {
      it->getCompleteFunc()(it);
    }
    it->setLastProcTimePoint(std::chrono::steady_clock::now());
  };
  // invoke abort function
  [[maybe_unused]] auto invokeAbortFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getAbortFunc()) {
      it->getAbortFunc()(it);
    }
    it->setLastProcTimePoint(std::chrono::steady_clock::now());
  };

  const std::lock_guard lg(m_Mutex);

  for (auto it = m_scheduledEvents.begin(); !m_scheduledEvents.empty() && it != m_scheduledEvents.end();) {
    auto event = *it;
    switch (event->getStatus()) {
      case Event::Status::Pending:
        if (event->getStartDelay() > std::chrono::milliseconds::min()) {
          if (!event->getEventClock().IsRunning()) {
            // Start the event clock to delay the event
            event->getEventClock().Start(event->getStartDelay());
            break;
          }
          if (event->getEventClock().IsElapsed()) {
            // start
            invokeStartFunction(event);
            event->setStatus(Event::Status::Running);
            event->getEventClock().Start(event->getServeInterval());
            break;
          }
        } else {
          // start immediately
          invokeStartFunction(event);
          event->setStatus(Event::Status::Running);
          event->getEventClock().Start(event->getServeInterval());
        }
        break;
      case Event::Status::Running:
        if (!event->getEventClock().IsRunning()) {
          // start timer
          event->getEventClock().Start(event->getServeInterval());
          invokeStartFunction(event);
        } else if (event->getEventClock().IsElapsed().value()) {
          invokeEventFunction(event);
          event->getEventClock().Start(event->getServeInterval());
        }
        break;
      case Event::Status::Completed:
        invokeCompleteFunction(event);
        it = m_scheduledEvents.erase(it);
        continue;
      case Event::Status::Aborted:
        invokeAbortFunction(event);
        it = m_scheduledEvents.erase(it);
        continue;
      case Event::Status::Timeouted:
        invokeTimeoutFunction(event);
        it = m_scheduledEvents.erase(it);
        continue;
      default:
        it = m_scheduledEvents.erase(it);
        continue;
    }

    auto remainingTimeMs = event->getEventClock().LeftTime().count();
    if (remainingTimeMs < 0) {
      remainingTimeMs = 0;
    }
    // check timeout
    if (event->getLifeDuration().count() > 0) {
      if (event->getLifeClock().IsRunning() && event->getLifeClock().IsElapsed().value()) {
        event->setStatus(Event::Status::Timeouted);
        remainingTimeMs = 0;
      } else {
        auto remainingLifeMs = event->getLifeClock().LeftTime().count();
        if (remainingLifeMs < 0) {
          remainingLifeMs = 0;
        }
        if (remainingTimeMs > remainingLifeMs) {
          remainingTimeMs = remainingLifeMs;
        }
      }
    }

    if (remainingTimeMs < processingTime.count()) {
      processingTime = std::chrono::milliseconds(remainingTimeMs);
    }

    ++it;
  }
  // Ensure wait time is at least minInterval
  if (processingTime.count() <= 0)
    processingTime = 1ms;

  return processingTime;
}

}  // namespace tev

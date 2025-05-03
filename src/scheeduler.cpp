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
      DurationUnit waitTime = service();
      std::unique_lock lock(m_CondMutex);
      m_CondEvent.wait_for(lock, waitTime);
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

  if (event->getEventClock().Timeout<>() != std::chrono::milliseconds::min()) {
    event->getEventClock().Start();
  }
  if (event->getLifeClock().Timeout<>() != std::chrono::milliseconds::min()) {
    event->getLifeClock().Start();
  }
  if (event->getStartDelay() != std::chrono::milliseconds::min()) {
    event->setStatus(Event::Status::Pending);
  } else {
    event->setStatus(Event::Status::Running);
  }
  // add event to the list
  m_scheduledEvents.push_back(event);
  // notify the scheduler thread
  m_CondEvent.notify_one();
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
std::chrono::milliseconds Scheduler::service() {
  std::chrono::milliseconds nMinDelay = m_scheduledEvents.empty() ? m_MaxDelayIntervalMs : 1000ms;
  // invoke start function
  [[maybe_unused]] auto invokeStartFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getStartFunc()) {
      it->getStartFunc()(it);
    }
  };
  // invoke event function
  [[maybe_unused]] auto invokeEventFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getEventFunc()) {
      it->getEventFunc()(it);
    }
  };
  // invoke timeout function
  [[maybe_unused]] auto invokeTimeoutFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getTimeoutFunc()) {
      it->getTimeoutFunc()(it);
    }
  };
  // invoke complete function
  [[maybe_unused]] auto invokeCompleteFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getCompleteFunc()) {
      it->getCompleteFunc()(it);
    }
  };
  // invoke abort function
  [[maybe_unused]] auto invokeAbortFunction = [this](const std::shared_ptr<Event>& it) {
    if (it->getAbortFunc()) {
      it->getAbortFunc()(it);
    }
  };

  const std::lock_guard lg(m_Mutex);

  for (auto it = m_scheduledEvents.begin(); !m_scheduledEvents.empty() && it != m_scheduledEvents.end();) {
    std::cout << ">> event check --------------------" << std::endl;
    auto event = *it;
    switch (event->getStatus()) {
      case Event::Status::Pending:
        std::cout << "pending" << std::endl;
        if (event->getStartDelay() != std::chrono::milliseconds::min()) {
          if (!event->getEventClock().IsRunning()) {
            // Start the event clock to delay the event
            std::cout << "start delay" << std::endl;
            event->getEventClock().Start(event->getStartDelay());
            break;
          }
          if (event->getEventClock().IsElapsed()) {
            // start
            std::cout << "start" << std::endl;
            event->getEventClock().SetTimeout(event->getServeInterval());
            event->setStatus(Event::Status::Running);
            invokeStartFunction(event);
            break;
          }
        } else {
          // start immediately
          std::cout << "start immediately" << std::endl;
          event->getEventClock().SetTimeout(event->getServeInterval());
          event->setStatus(Event::Status::Running);
          invokeStartFunction(event);
        }
        break;
      case Event::Status::Running:
        std::cout << "running" << std::endl;
        if (!event->getEventClock().IsRunning()) {
          // start timer
          std::cout << "start timer" << std::endl;
          event->getEventClock().Start(event->getServeInterval());
          invokeStartFunction(event);
        } else if (event->getEventClock().IsElapsed().value()) {
          std::cout << "elapsed" << std::endl;
          event->getEventClock().SetTimeout(event->getServeInterval());
          invokeEventFunction(event);
        }
        break;
      case Event::Status::Completed:
        std::cout << "completed" << std::endl;
        invokeCompleteFunction(event);
        it = m_scheduledEvents.erase(it);
        continue;
      case Event::Status::Aborted:
        std::cout << "aborted" << std::endl;
        invokeAbortFunction(event);
        it = m_scheduledEvents.erase(it);
        continue;
      case Event::Status::Timeouted:
        std::cout << "timeouted" << std::endl;
        invokeTimeoutFunction(event);
        it = m_scheduledEvents.erase(it);
        continue;
      default:
        std::cout << "default" << std::endl;
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

    if (remainingTimeMs < nMinDelay.count()) {
      nMinDelay = std::chrono::milliseconds(remainingTimeMs);
    }

    // next
    it++;
  }
  return nMinDelay;
}
}  // namespace tev

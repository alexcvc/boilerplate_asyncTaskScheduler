#include "asyncTaskScheduler.hpp"

#include <algorithm>

#include <spdlog/spdlog.h>

AsyncTaskScheduler::AsyncTaskScheduler(const std::string& name) : m_IsRunning(false), m_SchedulerName(name) {}

AsyncTaskScheduler::~AsyncTaskScheduler() {
  stop();
}

int AsyncTaskScheduler::addTask(std::function<TaskState(void*)> callback, void* handler, int intervalMs,
                                int delayStartMs, int timeoutMs, int priority, int retries, int groupId,
                                std::vector<int> dependencies) {
  std::lock_guard lock(m_TasksMutex);

  Task task;
  task.id = m_NextTaskId++;
  task.callback = callback;
  task.handler = handler;
  task.interval = std::chrono::milliseconds(intervalMs);
  task.delayStart = std::chrono::milliseconds(delayStartMs);
  task.timeoutLimit = std::chrono::milliseconds(timeoutMs);
  task.createdAt = std::chrono::steady_clock::now();
  task.lastRun = task.createdAt;
  task.priority = priority;
  task.retryCount = retries;
  task.groupId = groupId;
  task.dependencies = dependencies;

  m_Tasks.push_back(task);

  if (task.handler == nullptr) {
    m_Tasks.back().handler = &m_Tasks.back();
    spdlog::info("[{}] Handler auto-assigned to task {}", m_SchedulerName, task.id);
  }

  spdlog::info("[{}] Added Task {}", m_SchedulerName, task.id);
  m_Condition.notify_one();
  return task.id;
}

void AsyncTaskScheduler::pauseTask(int taskId) {
  std::lock_guard lock(m_TasksMutex);
  for (auto& task : m_Tasks) {
    if (task.id == taskId) {
      task.paused = true;
      task.state = TaskState::Paused;
      spdlog::info("[{}] Paused Task {}", m_SchedulerName, taskId);
      m_Condition.notify_one();
      return;
    }
  }
}

void AsyncTaskScheduler::resumeTask(int taskId) {
  std::lock_guard lock(m_TasksMutex);
  for (auto& task : m_Tasks) {
    if (task.id == taskId) {
      task.paused = false;
      task.state = TaskState::Waiting;
      spdlog::info("[{}] Resumed Task {}", m_SchedulerName, taskId);
      m_Condition.notify_one();
      return;
    }
  }
}

void AsyncTaskScheduler::start() {
  m_IsRunning = true;
  m_SchedulerThread = std::thread(&AsyncTaskScheduler::runLoop, this);
}

void AsyncTaskScheduler::stop() {
  m_IsRunning = false;
  m_Condition.notify_all();
  if (m_SchedulerThread.joinable()) {
    m_SchedulerThread.join();
  }
}

bool AsyncTaskScheduler::dependenciesSatisfied(const Task& task) {
  for (int const depId : task.dependencies) {
    auto it = std::ranges::find_if(m_Tasks, [depId](const Task& t) {
      return t.id == depId;
    });
    if (it != m_Tasks.end() && it->state != TaskState::Done) {
      return false;
    }
  }
  return true;
}

Task* AsyncTaskScheduler::findTaskByHandler(void* handler) {
  std::lock_guard const lock(m_TasksMutex);
  for (auto& task : m_Tasks) {
    if (task.handler == handler) {
      return &task;
    }
  }
  return nullptr;
}

void AsyncTaskScheduler::runLoop() {
  while (m_IsRunning) {
    auto now = std::chrono::steady_clock::now();
    std::chrono::milliseconds minSleepTime(200);

    {
      std::lock_guard const lock(m_TasksMutex);

      if (!m_Tasks.empty()) {
        std::ranges::sort(m_Tasks, [](const Task& a, const Task& b) {
          return a.priority > b.priority;
        });

        for (auto it = m_Tasks.begin(); it != m_Tasks.end();) {
          if (it->state == TaskState::Done || it->state == TaskState::Failed || it->state == TaskState::Timeout) {
            spdlog::info("[{}] Removing finished Task {}", m_SchedulerName, it->id);
            it = m_Tasks.erase(it);
            continue;
          }

          if (it->paused || !dependenciesSatisfied(*it)) {
            ++it;
            continue;
          }

          auto timeSinceCreation = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->createdAt);
          if (timeSinceCreation < it->delayStart) {
            auto waitTime = it->delayStart - timeSinceCreation;
            if (waitTime < minSleepTime) {
              minSleepTime = waitTime;
            }
            ++it;
            continue;
          }

          if (it->timeoutLimit.count() > 0 && timeSinceCreation > it->timeoutLimit) {
            spdlog::error("[{}] Task {} timeout triggered.", m_SchedulerName, it->id);
            it->state = TaskState::Timeout;
            ++it;
            continue;
          }

          auto timeSinceLastRun = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->lastRun);
          if (timeSinceLastRun >= it->interval) {
            it->state = it->callback(it->handler);
            it->lastRun = now;

            if (it->state == TaskState::Failed && it->currentRetries < it->retryCount) {
              it->currentRetries++;
              it->state = TaskState::Waiting;
              spdlog::warn("[{}] Task {} failed, retrying ({}/{})", m_SchedulerName, it->id, it->currentRetries,
                           it->retryCount);
            } else if (it->state == TaskState::Failed) {
              spdlog::error("[{}] Task {} permanently failed.", m_SchedulerName, it->id);
            } else if (it->state == TaskState::Done) {
              spdlog::info("[{}] Task {} finished successfully.", m_SchedulerName, it->id);
            }
          } else {
            auto waitTime = it->interval - timeSinceLastRun;
            if (waitTime < minSleepTime) {
              minSleepTime = waitTime;
            }
          }

          ++it;
        }
      }
    }

    std::unique_lock<std::mutex> lock(m_ConditionMutex);
    m_Condition.wait_for(lock, minSleepTime, [this]() {
      return !m_IsRunning;
    });
  }

  spdlog::info("[{}] Scheduler stopped.", m_SchedulerName);
}
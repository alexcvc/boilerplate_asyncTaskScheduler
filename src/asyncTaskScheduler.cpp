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

  AsyncTask task;
  task.id = m_NextTaskId++;
  task.m_callback = callback;
  task.m_handler = handler;
  task.m_interval = std::chrono::milliseconds(intervalMs);
  task.m_delayStart = std::chrono::milliseconds(delayStartMs);
  task.m_timeoutLimit = std::chrono::milliseconds(timeoutMs);
  task.m_addedAt = std::chrono::steady_clock::now();
  task.m_lastRun = task.m_addedAt;
  task.priority = priority;
  task.retryCount = retries;
  task.groupId = groupId;
  task.dependencies = dependencies;

  m_Tasks.push_back(task);

  if (task.m_handler == nullptr) {
    m_Tasks.back().m_handler = &m_Tasks.back();
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
      task.m_isRunning = true;
      task.m_taskState = TaskState::Paused;
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
      task.m_isRunning = false;
      task.m_taskState = TaskState::Waiting;
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

bool AsyncTaskScheduler::dependenciesSatisfied(const AsyncTask& task) {
  for (int const depId : task.dependencies) {
    auto it = std::ranges::find_if(m_Tasks, [depId](const AsyncTask& t) {
      return t.id == depId;
    });
    if (it != m_Tasks.end() && it->m_taskState != TaskState::Done) {
      return false;
    }
  }
  return true;
}

AsyncTask* AsyncTaskScheduler::findTaskByHandler(void* handler) {
  std::lock_guard const lock(m_TasksMutex);
  for (auto& task : m_Tasks) {
    if (task.m_handler == handler) {
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
        std::ranges::sort(m_Tasks, [](const AsyncTask& a, const AsyncTask& b) {
          return a.priority > b.priority;
        });

        for (auto it = m_Tasks.begin(); it != m_Tasks.end();) {
          if (it->m_taskState == TaskState::Done || it->m_taskState == TaskState::Failed || it->m_taskState == TaskState::Timeout) {
            spdlog::info("[{}] Removing finished Task {}", m_SchedulerName, it->id);
            it = m_Tasks.erase(it);
            continue;
          }

          if (!it->m_isRunning || !dependenciesSatisfied(*it)) {
            ++it;
            continue;
          }

          auto timeSinceCreation = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->m_addedAt);
          if (timeSinceCreation < it->m_delayStart) {
            auto waitTime = it->m_delayStart - timeSinceCreation;
            if (waitTime < minSleepTime) {
              minSleepTime = waitTime;
            }
            ++it;
            continue;
          }

          if (it->m_timeoutLimit.count() > 0 && timeSinceCreation > it->m_timeoutLimit) {
            spdlog::error("[{}] Task {} timeout triggered.", m_SchedulerName, it->id);
            it->m_taskState = TaskState::Timeout;
            ++it;
            continue;
          }

          auto timeSinceLastRun = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->m_lastRun);
          if (timeSinceLastRun >= it->m_interval) {
            it->m_taskState = it->m_callback(it->m_handler);
            it->m_lastRun = now;

            if (it->m_taskState == TaskState::Failed && it->currentRetries < it->retryCount) {
              it->currentRetries++;
              it->m_taskState = TaskState::Waiting;
              spdlog::warn("[{}] Task {} failed, retrying ({}/{})", m_SchedulerName, it->id, it->currentRetries,
                           it->retryCount);
            } else if (it->m_taskState == TaskState::Failed) {
              spdlog::error("[{}] Task {} permanently failed.", m_SchedulerName, it->id);
            } else if (it->m_taskState == TaskState::Done) {
              spdlog::info("[{}] Task {} finished successfully.", m_SchedulerName, it->id);
            }
          } else {
            auto waitTime = it->m_interval - timeSinceLastRun;
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
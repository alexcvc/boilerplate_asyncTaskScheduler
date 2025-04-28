#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "asyncTask.hpp"

/**
 * @class AsyncTaskScheduler
 * @brief A class responsible for scheduling and managing asynchronous tasks.
 *
 * This class allows asynchronous tasks to be queued, executed, and managed
 * in an efficient and organized manner. It provides functionality for
 * scheduling tasks, handling dependencies, and monitoring task completion.
 *
 * The `AsyncTaskScheduler` is designed for use in scenarios where multiple
 * asynchronous operations need to be managed concurrently, minimizing
 * overhead and optimizing system performance.
 */
class AsyncTaskScheduler {
 public:
  AsyncTaskScheduler(const std::string& name = "Scheduler");
  ~AsyncTaskScheduler();

  int addTask(std::function<TaskState(void*)> callback, void* handler, int intervalMs, int delayStartMs = 0,
              int timeoutMs = 0, int priority = 0, int retries = 0, int groupId = -1,
              std::vector<int> dependencies = {});

  void pauseTask(int taskId);
  void resumeTask(int taskId);
  void start();
  void stop();

  AsyncTask* findTaskByHandler(void* handler);

 private:
  void runLoop();
  bool dependenciesSatisfied(const AsyncTask& task);

  std::vector<AsyncTask> m_Tasks;
  std::mutex m_TasksMutex;
  std::atomic<bool> m_IsRunning;
  std::thread m_SchedulerThread;
  int m_NextTaskId = 0;
  std::string m_SchedulerName;
  std::condition_variable m_Condition;
  std::mutex m_ConditionMutex;
};
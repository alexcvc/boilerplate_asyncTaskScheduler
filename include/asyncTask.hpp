#pragma once

#include <chrono>
#include <functional>
#include <vector>

enum class TaskState { Ready, Waiting, Running, Done, Failed, Timeout, Paused };

struct AsyncTask {
  int id;
  std::function<TaskState(void*)> m_callback;
  void* m_handler = nullptr;

  std::chrono::milliseconds m_interval;
  std::chrono::milliseconds m_delayStart;
  std::chrono::milliseconds m_timeoutLimit;
  std::chrono::steady_clock::time_point m_addedAt;
  std::chrono::steady_clock::time_point m_lastRun;
  TaskState m_taskState = TaskState::Waiting;
  bool m_isRunning = false;
};

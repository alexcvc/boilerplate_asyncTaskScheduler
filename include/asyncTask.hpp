#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <vector>

enum class TaskState { Ready, Waiting, Running, Done, Failed, Timeout, Paused };

struct Task {
  int id;
  std::function<TaskState(void*)> callback;
  void* handler = nullptr;

  std::chrono::milliseconds interval;
  std::chrono::milliseconds delayStart;
  std::chrono::milliseconds timeoutLimit;
  std::chrono::steady_clock::time_point createdAt;
  std::chrono::steady_clock::time_point lastRun;
  TaskState state = TaskState::Waiting;
  int priority;
  int retryCount = 0;
  int currentRetries = 0;
  bool paused = false;
  int groupId = -1;
  std::vector<int> dependencies;
};

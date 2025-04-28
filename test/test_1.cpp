#define CATCH_CONFIG_MAIN
#include <cstring>

#include <catch2/catch_all.hpp>

#include "asyncTaskScheduler.hpp"
#include "cstack.h"

TEST_CASE("Simple Task Runs", "[task]") {
  AsyncTaskScheduler scheduler("TestScheduler");
  scheduler.start();

  TaskStackObject ctx;
  ctx.value = 0;
  strcpy(ctx.name, "TestTask");

  int taskId = scheduler.addTask(
      [](void* handler) -> TaskState {
        TaskStackObject* ctx = static_cast<TaskStackObject*>(handler);
        ctx->value++;
        return (ctx->value >= 3) ? TaskState::Done : TaskState::Running;
      },
      &ctx, 100, 0, 1000);

  REQUIRE(taskId >= 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  scheduler.stop();

  REQUIRE(ctx.value >= 3);
}
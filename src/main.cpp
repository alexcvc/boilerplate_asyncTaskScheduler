#include <thread>

#include <spdlog/spdlog.h>

#include "my_c_library.h"
#include "scheduler.h"

TaskState myTaskFunction(void* handler) {
  auto* ctx = static_cast<MyCStruct*>(handler);
  do_work(ctx);

  static int counter = 0;
  counter++;
  spdlog::info("Task [{}] running, value = {}", ctx->name, ctx->value);

  if (counter >= 5) {
    return TaskState::Done;
  }
  return TaskState::Running;
}

int main() {
  auto counter{10};
  std::string taskName;
  spdlog::set_level(spdlog::level::debug);

  Scheduler scheduler("MainScheduler");
  scheduler.start();

  MyCStruct ctx;
  ctx.value = 0;
  taskName = "Task_" + std::to_string(counter);
  strcpy(ctx.name, taskName.c_str());

  scheduler.addTask(myTaskFunction, &ctx, 500, 0, 10000, 5, 1);

  while (--counter > 0) {
    std::shared_ptr<MyCStruct> ctxPtr = std::make_shared<MyCStruct>();
    ctxPtr->value = 10 * counter;
    taskName = "Task_" + std::to_string(counter);
    strcpy(ctxPtr->name, taskName.c_str());
    scheduler.addTask(myTaskFunction, ctxPtr.get(), 500, 0, 10000, 5, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  scheduler.stop();

  spdlog::info("Main finished.");
}
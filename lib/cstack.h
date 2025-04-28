#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int value;
  char name[32];
} TaskStackObject;

/**
   *
   * @param obj task stack C-object
   */
void ProcessTaskStackObject(TaskStackObject* object);

#ifdef __cplusplus
}
#endif

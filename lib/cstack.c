#include "cstack.h"

#include <stdio.h>
#include <string.h>

void ProcessTaskStackObject(TaskStackObject* object) {
  printf("[C-Stack] Doing work: name=%s, value=%d\n", object->name, object->value);
  object->value += 1;
}
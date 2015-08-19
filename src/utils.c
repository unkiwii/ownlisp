#include "utils.h"
#include <string.h>

int is(char* a, char* b)
{
  if (!a) { return 0; }
  if (!b) { return 0; }
  return strcmp(a, b) == 0;
}

int has(char* a, char* b)
{
  if (!a) { return 0; }
  if (!b) { return 0; }
  return strstr(a, b) != 0;
}

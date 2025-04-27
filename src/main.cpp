#include "rawMode.h"
#include "sUtils.h"

#include <unistd.h>

int main()
{
  RawMode::enableRawMode();
  Init();
  while (true)
  {
    Draw();
    ProcessKey();
  }
  return 0;
}

#include "rawMode.h"
#include "sUtils.h"

#include <unistd.h>

int main()
{
  RawMode::enableRawMode();
  while (true)
  {
    Init();
    while (!Restart)
    {
      Draw();
      ProcessKey();
    }
  }
  return 0;
}

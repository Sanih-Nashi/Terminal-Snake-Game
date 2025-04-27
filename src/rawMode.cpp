#include "rawMode.h"
#include "sUtils.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void RawMode::disableRawMode()
{
  write(STDOUT_FILENO, "\033[40m\033[0m\033[2J\033[H", 16);
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH ,&orig_setting) == -1)
    die("tcsetattr");
}


void RawMode::enableRawMode() {

  write(STDOUT_FILENO, "\033[2J", 4);

  if (tcgetattr(STDIN_FILENO, &orig_setting) == -1) 
    die("tcgetattr");
  atexit(disableRawMode);
  struct termios raw = orig_setting;

  raw.c_lflag &= ~(ECHO | ISIG | IEXTEN | ICANON);
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}



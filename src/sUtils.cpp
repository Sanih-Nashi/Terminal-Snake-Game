#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <algorithm>
#include <random>
#include <cmath>
#include <iostream>

#include "sUtils.h"
#include "rawMode.h"

char ReadKey()
{
//   int read_num;
//   char c;
//   while ( (read_num = read(STDIN_FILENO, &c, 1)) != 1) 
//   {
//     if (read_num == -1 && errno != EAGAIN)
//       die("read");
//   }
//   return c;
  fd_set set;
  struct timeval timeout;
  int rv;

  FD_ZERO(&set);
  FD_SET(STDIN_FILENO, &set);

  timeout.tv_sec = 0;      
  timeout.tv_usec = 500000;

  rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
  if (rv <= 0) {
      // Timeout or error
      return 0;
  }

  char c;
  ssize_t n = read(STDIN_FILENO, &c, 1);
  if (n == 1) {
      return c;
  } else {
      // Read error or EOF
      return 0;
  }
}

void die(const char* errormsg)
{
  RawMode::disableRawMode();
  write(STDOUT_FILENO, "\033[2J", 4);
  perror(errormsg);
}

bool GetTerminalWin(int& column, int& row)
{
  struct winsize window_size;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1 || window_size.ws_col == 0)
    return false;
  else{
    column = window_size.ws_col;
    row = window_size.ws_row;
    return true;
  }

}

Vector2i GetRandomPos()
{
Restart:
  // gets random value near mean in a deviation
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> distribx(Snake[0].x, Deviation);
  std::normal_distribution<> distriby(Snake[0].y, Deviation);

  Vector2i pos;
  double value;
  while (true){

    value = distribx(gen);
    pos.x = std::clamp(static_cast<int>(std::round(value)), 1, TerminalSize.x - 1);

    value = distriby(gen);
    pos.y = std::clamp(static_cast<int>(std::round(value)), 1, TerminalSize.y - 1);

    // checks if the number is inside the vector Snake
    for (const Vector2i& val : Snake){
      if (val == pos)
        goto Restart;
    }
    return pos;

  }
  

}

void Init()
{
  if (!GetTerminalWin(TerminalSize.x, TerminalSize.y))
    die("cannot get the terminal size");

  Direction = Dir::UP;
  PreviousDirection = Dir::NIL;
  
  Snake = std::vector{
    Vector2i{TerminalSize.x / 2, (TerminalSize.y / 2)},
    Vector2i{TerminalSize.x / 2, (TerminalSize.y / 2) + 1},
    Vector2i{TerminalSize.x / 2, (TerminalSize.y / 2) + 2},
    Vector2i{TerminalSize.x / 2, (TerminalSize.y / 2) + 3},
  };

  Fruits = std::vector{
    GetRandomPos(),
    GetRandomPos(),
    GetRandomPos()
  };

}

void Draw()
{

  write(STDOUT_FILENO, "\033[H\033[2J", 7);
  char str[22];
  int len;

  for (int i = 0; i < TerminalSize.x; i++)
    write(STDOUT_FILENO, "#", 1);

  len = snprintf(str, sizeof(str), "\r\033[%dB", TerminalSize.y - 1);
  write(STDOUT_FILENO, str, len);

  for (int i = 0; i < TerminalSize.x; i++)
    write(STDOUT_FILENO, "#", 1);

  write(STDOUT_FILENO, "\033[H", 3);  
 
  write(STDOUT_FILENO, "#", 1);
  for (int i = 0; i < TerminalSize.y - 1; i++)
    write(STDOUT_FILENO, "\n\b#", 4);

  len = snprintf(str, sizeof(str), "\033[H\033[%dC", TerminalSize.x);
  write(STDOUT_FILENO, str, len);  

  write(STDOUT_FILENO, "#", 1);
  for (int i = 0; i < TerminalSize.y - 1; i++)
    write(STDOUT_FILENO, "\n#", 3);


  for (int i = 0; i < Fruits.size(); i++)
  {
    len = snprintf(str, sizeof(str), "\033[%d;%dH", Fruits[i].y + 1, Fruits[i].x + 1);
    write(STDOUT_FILENO, str, len);
    write(STDOUT_FILENO, "\033[31m@\033[0m", 10);
  }
  

  len = snprintf(str, sizeof(str), "\033[%d;%dH", Snake[0].y + 1, Snake[0].x + 1);
  write(STDOUT_FILENO, str, len);
  write(STDOUT_FILENO, "\033[32mO\033[0m", 10);
  for (int i = 1; i < Snake.size(); i++)
  {
    len = snprintf(str, sizeof(str), "\033[%d;%dH", Snake[i].y + 1, Snake[i].x + 1);
    write(STDOUT_FILENO, str, len);
    write(STDOUT_FILENO, "\033[32m#\033[0m", 10);
  }


}

void Move()
{
  switch (Direction)
  {
  case Dir::UP:
  {
    if (PreviousDirection == Dir::DOWN)
    {
      Direction = PreviousDirection;
      Move();
      break;
    }
      PreviousDirection = Dir::UP;
    for (int i = Snake.size() - 1; i > 0; i--)
    {
      Snake[i] = Snake[i - 1];
    }
    Snake[0].y--;
    break;
  }

  case Dir::DOWN:
  {

    if (PreviousDirection == Dir::UP)
    {
      Direction = PreviousDirection;
      Move();
      break;
    }
      PreviousDirection = Dir::DOWN;
    for (int i = Snake.size() - 1; i > 0; i--)
    {
      Snake[i] = Snake[i - 1];
    }
    Snake[0].y++;
    break;
  }
  

  case Dir::LEFT:
  {

    if (PreviousDirection == Dir::RIGHT)
    {
      Direction = PreviousDirection;
      Move();
      break;
    }
      PreviousDirection = Dir::LEFT;
    for (int i = Snake.size() - 1; i > 0; i--)
    {
      Snake[i] = Snake[i - 1];
    }
    Snake[0].x--;
    break;
  }
  

  case Dir::RIGHT:
  {

    if (PreviousDirection == Dir::LEFT)
    {
      Direction = PreviousDirection;
      Move();
      break;
    }
      PreviousDirection = Dir::RIGHT;
    for (int i = Snake.size() - 1; i > 0; i--)
    {
      Snake[i] = Snake[i - 1];
    }
    Snake[0].x++;
    break;
  }
  

  default:
    break;
  }
}

void ProcessKey()
{
  char c = ReadKey();

  switch (c)
  {
  case 'q':
  {
    exit(0);
  }
  case 0:
  {
    Move();
    break;
  }
  case 'w':
  {
    Direction = Dir::UP;
    Move();
    break;
  }  
  case 's':
  {
    Direction = Dir::DOWN;
    Move();
    break;
  }  
  case 'a':
  {
    Direction = Dir::LEFT;
    Move();
    break;
  }  
  case 'd':
  {
    Direction = Dir::RIGHT;
    Move();
    break;
  }  
  
  
  default:
    break;
  }
}
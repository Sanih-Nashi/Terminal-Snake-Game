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

char ReadDirectKey()
{
  int read_num;
  char c;
  while ( (read_num = read(STDIN_FILENO, &c, 1)) != 1) 
  {
    if (read_num == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}

char ReadKey()
{
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

  write(STDOUT_FILENO, "\033[H\033[2J\033[35m", 12);
  char str[32];
  int len;

  for (int i = 0; i < TerminalSize.x; i++)
    write(STDOUT_FILENO, "\u2500", 3);

  len = snprintf(str, sizeof(str), "\r\033[%dB", TerminalSize.y - 1);
  write(STDOUT_FILENO, str, len);

  for (int i = 0; i < TerminalSize.x; i++)
    write(STDOUT_FILENO, "\u2500", 3);

  write(STDOUT_FILENO, "\033[H", 3);  
 
  write(STDOUT_FILENO, "\u250C", 3);
  for (int i = 0; i < TerminalSize.y - 2; i++)
    write(STDOUT_FILENO, "\n\b\u2502", 5);

  write(STDOUT_FILENO, "\n\b\u2514", 5);

  len = snprintf(str, sizeof(str), "\033[H\033[%dC", TerminalSize.x);
  write(STDOUT_FILENO, str, len);  

  write(STDOUT_FILENO, "\u2510", 3);
  for (int i = 0; i < TerminalSize.y - 2; i++)
    write(STDOUT_FILENO, "\n\u2502", 4);

  write(STDOUT_FILENO, "\n\u2518", 4);

  write(STDOUT_FILENO, "\033[H\n\u251C", 7);
  len = snprintf(str, sizeof(str), "\033[32mScore: %d\033[35m", Score);

  for (int i = 1; i < TerminalSize.x/2 - len/2; i++) // subtraction to centre things so that the text looks good
    write(STDOUT_FILENO, "\u2500", 3);

  write(STDOUT_FILENO, str, len);
  for (int i = TerminalSize.x/2 + len/2; i < TerminalSize.x - 1; i++)
    write(STDOUT_FILENO, "\u2500", 3);

  write(STDOUT_FILENO, "\u2524", 3);


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
      return;
    }
      PreviousDirection = Dir::UP;
    for (int i = Snake.size() - 1; i > 0; i--)
    {
      Snake[i] = Snake[i - 1];
    }
    Snake[0].y--;
    Draw();
    break;
  }

  case Dir::DOWN:
  {

    if (PreviousDirection == Dir::UP)
    {
      Direction = PreviousDirection;
      return;
    }
      PreviousDirection = Dir::DOWN;
    for (int i = Snake.size() - 1; i > 0; i--)
    {
      Snake[i] = Snake[i - 1];
    }
    Snake[0].y++;
    Draw();
    break;
  }
  

  case Dir::LEFT:
  {

    if (PreviousDirection == Dir::RIGHT)
    {
      Direction = PreviousDirection;
      return;
    }
      PreviousDirection = Dir::LEFT;
    for (int i = Snake.size() - 1; i > 0; i--)
    {
      Snake[i] = Snake[i - 1];
    }
    Snake[0].x--;
    Draw();
    break;
  }
  

  case Dir::RIGHT:
  {

    if (PreviousDirection == Dir::LEFT)
    {
      Direction = PreviousDirection;
      return;
    }
      PreviousDirection = Dir::RIGHT;
    for (int i = Snake.size() - 1; i > 0; i--)
    {
      Snake[i] = Snake[i - 1];
    }
    Snake[0].x++;
    Draw();
    break;
  }
  

  default:
    break;
  }

  for (int i = 0; i < Fruits.size(); i++)
  {
    if (Snake[0] == Fruits[i])
    {
      // checks whether the x axises of the last elements same so the new block
      // and if yes allocates the new block at the end which is equal to the final block
      // except with a change in the y axis one unit
      // so the new block would be in a straight line to the other ones

      if (Snake[Snake.size() - 1].x == Snake[Snake.size() - 2].x)
        Snake.push_back(Vector2i{
          Snake[Snake.size() - 1].x,
          Snake[Snake.size() - 2].y + (Snake[Snake.size() - 1].y - Snake[Snake.size() - 2].y)
        });

      else 
        Snake.push_back(Vector2i{
          Snake[Snake.size() - 2].x + (Snake[Snake.size() - 1].x - Snake[Snake.size() - 2].x),
          Snake[Snake.size() - 1].y
        });
      // if not that means the y axis are the same so the exact opposite


      // so that after 5 fruits eaten the deviation would be farther from the snake
      Deviation += 1/5; 
      Score += 5;
      Fruits[i] = GetRandomPos();
      return;
    }
  }

  if (Snake[0].x <= 1 || Snake[0].y <= 1 || Snake[0].x >= TerminalSize.x - 1 || Snake[0].y >= TerminalSize.y - 1)
  {
    write(STDOUT_FILENO, "\033[H\033[2J\e]50;*Monospace-20\a\033[31m", 31);
    std::cout <<"\033[" <<TerminalSize.y/2 <<";" <<TerminalSize.x/2 - 4 <<"H" <<std::flush;
    write(STDOUT_FILENO, "YOU LOST", 8);
    std::cout <<"\033[" <<TerminalSize.y/2 + 1 <<";" <<TerminalSize.x/2 - 8 <<"H" <<std::flush;
    write(STDOUT_FILENO, "PRESS 'q' TO EXIT\033[0m", 21);
    while(true)
    {
      char c = ReadDirectKey();
      if (c == 'q')
        exit(0);
    }  
  }

  for (int i = 1; i < Snake.size(); i++)
  {
    if (Snake[0] == Snake[i]){
      write(STDOUT_FILENO, "\033[H\033[2J\033[31m\033]50;72\007", 20);
      std::cout <<"\033[" <<TerminalSize.y/2 <<";" <<TerminalSize.x/2 - 4 <<"H" <<std::flush;
      write(STDOUT_FILENO, "YOU LOST", 8);
      std::cout <<"\033[" <<TerminalSize.y/2 + 1 <<";" <<TerminalSize.x/2 - 8 <<"H" <<std::flush;
      write(STDOUT_FILENO, "PRESS 'q' TO EXIT\033[0m\033]50;12\007", 29);
      while(true)
      {
       char c = ReadDirectKey();
        if (c == 'q')
        exit(0);
      }  
    }           
  }
  // \e]50;*Monospace-20\a

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

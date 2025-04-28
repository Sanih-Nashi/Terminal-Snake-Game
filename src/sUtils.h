#include <termio.h>
#include <vector>

struct Vector2i
{
  int x, y;

  bool operator==(const Vector2i& other)
  const 
  {
    return x == other.x && y == other.y;
  }
};

enum class Dir{
  UP,
  DOWN, 
  LEFT,
  RIGHT,
  NIL
};

inline Dir Direction;
inline Dir PreviousDirection;
inline struct termios orig_setting;
inline Vector2i TerminalSize;
inline int Deviation = 3;
inline int Score = 0;
inline Vector2i PreviousScorePrint;
inline std::vector<Vector2i> Snake;
inline std::vector<Vector2i> PreviousSnake;
inline std::vector<Vector2i> Fruits;
inline std::vector<Vector2i> PreviousFruits;

char ReadKey();
void Move();
bool GetTerminalWin(int& column, int& row);
Vector2i GetRandomPos();
void Init();
void Draw();
void ProcessKey();
void die(const char* errormsg);
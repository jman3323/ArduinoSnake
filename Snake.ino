//**************************************
//Jack Dates, Tom Saad
//ECSE 1010 Final Project
//Snake game with an LED matrix
//**************************************


//**************
//LED SETUP STUFF
//**************

//row[i] is the Arduino pin for row i's ground
const int rowMap[8] = {
  45,44,43,42,38,39,40,41
};

//colMap[i] is the Arduino pins for col i's {red, green}
const int colMap[8][2] = {
  {29,28},{37,36},{27,26},{35,34},{25,32},{33,24},{23,30},{31,22}
};

//Pins where buttons are connected
const int LEFT = 51, RIGHT = 50, UP = 52, DOWN = 53;

//board[r][c] is the LED color
int board[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};

const int RED = -1, GREEN = 1, OFF = 0, GROUNDED = OUTPUT, OPEN = INPUT;

unsigned long prevTickTime = 0, prevRefreshTime = 0, restartTime = 0;
const unsigned long NORMALTICK = 250, HARDTICK = 175, EXPERTTICK = 117;
unsigned long gameTick = NORMALTICK; // game logic proceeds, 1/4 of a second
const unsigned long refreshTick = 3; //rate at which it flashes to the next row
int curPaintRow = 0;
int chaosTicks = 0;

//**************
//GAME LOGIC STUFF
//**************

int head[2] = {0,0};
int dir = RIGHT;
int nextDir = RIGHT;
int tail[2] = {0,0};
int prevDirs[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};
int chaos = 0;
int gameOver = 0;

void paint()
{
  if (millis() - prevRefreshTime >= refreshTick)
  {
    pinMode(rowMap[curPaintRow], OPEN);
    for (int next = (curPaintRow + 1) % 8, found = 0; next != curPaintRow && !found; next = (next + 1) % 8)
      for (int c = 0; c < 8; ++c)
        if (board[next][c] != OFF)
        {
          curPaintRow = next;
          found = 1;
          break;
        }
    pinMode(rowMap[curPaintRow], GROUNDED);
    prevRefreshTime = millis();
  }
  for (int c = 0; c < 8; ++c)
    if (board[curPaintRow][c] == OFF)
    {
      digitalWrite(colMap[c][0], LOW);
      digitalWrite(colMap[c][1], LOW);
    }
    else if (board[curPaintRow][c] == RED)
    {
      digitalWrite(colMap[c][0], HIGH);
      digitalWrite(colMap[c][1], LOW);
    }
    else if (board[curPaintRow][c] == GREEN)
    {
      digitalWrite(colMap[c][0], LOW);
      digitalWrite(colMap[c][1], HIGH);
    }
}

void oldPaint()
{
  for (int r = 0; r < 8; ++r)
  {
    pinMode(rowMap[r], OPEN);
    for (int c = 0; c < 8; ++c)
      if (board[r][c] == OFF)
      {
        digitalWrite(colMap[c][0], LOW);
        digitalWrite(colMap[c][1], LOW);
      }
      else if (board[r][c] == RED)
      {
        digitalWrite(colMap[c][0], HIGH);
        digitalWrite(colMap[c][1], LOW);
        pinMode(rowMap[r], GROUNDED);
      }
      else if (board[r][c] == GREEN)
      {
        digitalWrite(colMap[c][0], LOW);
        digitalWrite(colMap[c][1], HIGH);
        pinMode(rowMap[r], GROUNDED);
      }
  }
}

void clearBoard()
{
  for (int r = 0; r < 8; ++r)
    for (int c = 0; c < 8; ++c)
    {
      board[r][c] = OFF;
      prevDirs[r][c] = 0;
    }
}

void newGame()
{
  for (int p = 0; p < 8; ++p)
  {
    pinMode(rowMap[p], OPEN);
    pinMode(colMap[p][0], OUTPUT);
    pinMode(colMap[p][1], OUTPUT);
  }
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  Serial.begin(9600);
  prevTickTime = millis();
  prevRefreshTime = millis();
  curPaintRow = 0;

  clearBoard();
  board[1][5] = board[1][6] = board[1][7] = board[2][5] = GREEN;
  board[1][1] = board[1][2] = board[1][3] = board[2][1] = board[2][3] = GREEN;
  int curDiff = 0;
  board[5][6] = GREEN;
  if (gameTick == HARDTICK)
  {
    board[4][6] = board[5][6] = board[6][6] = GREEN;
    curDiff = 1;
  }
  else if (gameTick == EXPERTTICK)
  {
    board[4][6] = board[5][6] = board[6][6] = board[5][5] = RED;
    curDiff = 2;
  }
  unsigned long prevDiffPressTime = 0;
  board[5][2] = GREEN;
  if (chaos)
    board[4][2] = board[5][2] = board[6][2] = board[5][1] = RED;
  unsigned long prevChaosPressTime = 0;
  unsigned long pressWaitTime = 340;
  while (true)
  {
    if (digitalRead(LEFT) == LOW && digitalRead(RIGHT) == LOW && millis() - restartTime >= pressWaitTime)
      break;
    if (digitalRead(UP) == LOW && millis() - prevDiffPressTime >= pressWaitTime)
    {
      curDiff = (curDiff + 1) % 3;
      if (curDiff == 0)
      {
        board[5][6] = GREEN;
        board[4][6] = board[6][6] = board[5][5] = OFF;
      }
      else if (curDiff == 1)
      {
        board[4][6] = board[5][6] = board[6][6] = GREEN;
        board[5][5] = OFF;
      }
      else if (curDiff == 2)
        board[4][6] = board[5][6] = board[6][6] = board[5][5] = RED;
      prevDiffPressTime = millis();
    }
    if (digitalRead(DOWN) == LOW && millis() - prevChaosPressTime >= pressWaitTime)
    {
      chaos ^= 1;
      if (chaos)
        board[4][2] = board[5][2] = board[6][2] = board[5][1] = RED;
      else
      {
        board[5][2] = GREEN;
        board[4][2] = board[6][2] = board[5][1] = OFF;
      }
      prevChaosPressTime = millis();
    }
    paint();
  }
  if (curDiff == 0)
    gameTick = NORMALTICK;
  else if (curDiff == 1)
    gameTick = HARDTICK;
  else if (curDiff == 2)
    gameTick = EXPERTTICK;
  chaosTicks = 0;

  clearBoard();
  head[0] = 0;
  head[1] = 0;
  dir = RIGHT;
  nextDir = RIGHT;
  tail[0] = 0;
  tail[1] = 0;
  gameOver = 0;

  randomSeed(analogRead(0));
  spawnApple();
}

void setup()
{
  newGame();
}

void spawnApple()
{
  int x = random(0,8), y = random(0,8);
  while (board[x][y] != OFF)
  {
    x = random(0,8);
    y = random(0,8);
  }
  board[x][y] = RED;
}

void getMove(int dir, int* dr, int* dc)
{
  //NOTE that dr/dc are flipped since screen is flipped
  *dr = 0;
  *dc = 0;
  if (dir == LEFT)
    *dr = -1;
  else if (dir == RIGHT)
    *dr = 1;
  else if (dir == UP)
    *dc = 1;
  else if (dir == DOWN)
    *dc = -1;
}

void deleteTail()
{
  int dtr = 0, dtc = 0;
  getMove(prevDirs[tail[0]][tail[1]], &dtr, &dtc);
  board[tail[0]][tail[1]] = OFF;
  tail[0] = (tail[0]+dtr+8)%8;
  tail[1] = (tail[1]+dtc+8)%8;
}

void updateGameState()
{
  if (millis() - prevTickTime >= gameTick)
  {
    //This prevents you from killing yourself
    if (nextDir == LEFT && dir != RIGHT)
      dir = nextDir;
    else if (nextDir == RIGHT && dir != LEFT)
      dir = nextDir;
    else if (nextDir == UP && dir != DOWN)
      dir = nextDir;
    else if (nextDir == DOWN && dir != UP)
      dir = nextDir;
    
    int dr = 0, dc = 0;
    getMove(dir, &dr, &dc);
    int tr = (head[0]+dr+8)%8, tc = (head[1]+dc+8)%8;
    int appleEaten = 0;
    if (board[tr][tc] == RED)
      appleEaten = 1;
    Serial.print(head[0]);Serial.print(head[1]);Serial.print(tr);Serial.print(tc);Serial.println(appleEaten);
    if (head[0] == tail[0] && head[1] == tail[1])
      prevDirs[tail[0]][tail[1]] = dir;
    if (chaos && ++chaosTicks == 2)
    {
      chaosTicks = 0;
      if (!(head[0] == tail[0] && head[1] == tail[1]))
        deleteTail();
    }
    if (!appleEaten && !chaos || appleEaten && chaos) //delete tail if an apple wasn't eaten
    {
      if (chaos && !(head[0] == tail[0] && head[1] == tail[1]))
        deleteTail();
      deleteTail();
    }
    if (board[tr][tc] == GREEN)
    {
      gameOver = 1;
      for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
          if (board[r][c] == GREEN)
            board[r][c] = RED;
          else
            board[r][c] = OFF;
    }
    else
    {
      prevDirs[head[0]][head[1]] = dir;
      board[tr][tc] = GREEN;
      head[0] = tr;
      head[1] = tc;
    }
    if (appleEaten)
      spawnApple();
    prevTickTime = millis();
  }
}

void loop()
{
  //Test to loop over all spots
//  if (millis() - prevTickTime > 150)
//  {
//    board[r][c] = OFF;
//    if (++c == 8)
//    {
//      if (++r == 8)
//        r = 0;
//      c = 0;
//    }
//    board[r][c] = GREEN;
//    prevTickTime = millis();
//  }

  if (gameOver)
  {
    paint();
    //Click both left and right to restart game
    if (digitalRead(LEFT) == LOW && digitalRead(RIGHT) == LOW)
    {
      restartTime = millis();
      newGame();
    }
    return;
  }
  if (digitalRead(LEFT) == LOW)
    nextDir = LEFT;
  else if (digitalRead(RIGHT) == LOW)
    nextDir = RIGHT;
  else if (digitalRead(UP) == LOW)
    nextDir = UP;
  else if (digitalRead(DOWN) == LOW)
    nextDir = DOWN;
  updateGameState();
  paint();
}

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

unsigned long prevTickTime = 0, prevRefreshTime = 0;
const unsigned long gameTick = 250; // game logic proceeds, 1/4 of a second
const unsigned long refreshTick = 5; //rate at which it flashes to the next row
int curPaintRow = 0;

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

void setup()
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
    
    //NOTE that dr/dc are flipped since screen is flipped
    int dr = 0, dc = 0;
    getMove(dir, &dr, &dc);
    int tr = (head[0]+dr+8)%8, tc = (head[1]+dc+8)%8;
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
      if (board[tr][tc] != RED) //delete tail if an apple wasn't eaten
      {
        int dtr = 0, dtc = 0;
        getMove(prevDirs[tail[0]][tail[1]], &dtr, &dtc);
        board[tail[0]][tail[1]] = OFF;
        tail[0] = (tail[0]+dtr+8)%8;
        tail[1] = (tail[1]+dtc+8)%8;
      }
      else
      {
        board[tr][tc] = GREEN;
        spawnApple();
      }
      board[tr][tc] = GREEN;
      head[0] = tr;
      head[1] = tc;
    }
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
      setup();
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

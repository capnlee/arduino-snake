#include <Gamer.h>
#include <SoftwareSerial.h>

Gamer gamer;

#define gmMenu 0
#define gmPlay 1
#define gmScore 2
#define gmSelect 3

int gameMode = gmMenu;

#define dirUp 0
#define dirRight 1
#define dirDown 2
#define dirLeft 3

int gameUpdateRate = 10; //updates per second (defined later in beginGame())

int headX = 0;
int headY = 0;
int dir = dirRight;
int snakeLen = 3;

int fruitX = 0;
int fruitY = 0;

int field[8][8];
unsigned score = 0;
boolean gameOver = false;
boolean invertDisplay = false;
unsigned long lastUpdate = 0;

byte scoreFrames[5][8];
int scoreLength = 0;

byte scoreFont[11][8] = {
               {B00000000,
                B00011000,
                B00100100,
                B00101100,    // 0
                B00110100,
                B00100100,
                B00011000,
                B00000000},
                 
               {B00000000,
                B00001000,
                B00011000,
                B00001000,    // 1
                B00001000,
                B00001000,
                B00001000,
                B00000000},
                 
               {B00000000,
                B00011000,
                B00100100,
                B00000100,    // 2
                B00011000,
                B00100000,
                B00111100,
                B00000000},
              
               {B00000000,
                B00011000,
                B00100100,
                B00001000,    // 3
                B00000100,
                B00100100,
                B00011000,
                B00000000},
                 
               {B00000000,
                B00100100,
                B00100100,
                B00100100,    // 4
                B00111100,
                B00000100,
                B00000100,
                B00000000},   
                 
               {B00000000,
                B00111100,
                B00100000,
                B00111000,    // 5
                B00000100,
                B00100100,
                B00011000,
                B00000000},      
                 
               {B00000000,
                B00011000,
                B00100000,
                B00111000,    // 6
                B00100100,
                B00100100,
                B00011000,
                B00000000},
                 
               {B00000000,
                B00111100,
                B00000100,
                B00001000,    // 7
                B00001000,
                B00010000,
                B00010000,
                B00000000},
                 
               {B00000000,
                B00011000,
                B00100100,
                B00011000,    // 8
                B00100100,
                B00100100,
                B00011000,
                B00000000},
                 
               {B00000000,
                B00011000,
                B00100100,
                B00100100,    // 9
                B00011100,
                B00100100,
                B00011000,
                B00000000},   
                 
               {B00000000,
                B00000000,
                B00000000,
                B00000000,    // {BLANK}
                B00000000,
                B00000000,
                B00000000,
                B00000000}
};
  
   
byte menuFrames[5][8] = {
	       {B00000000,
		B01111110,
		B01000000,
		B00111100,    // S
		B00000010,
		B00000010,
		B01111110,
		B00000000},

  	       {B00000000,
		B01100010,
		B01010010,
		B01010010,    // N
		B01001010,
		B01001010,
		B01000110,
		B00000000},

	       {B00000000,
		B00100100,
		B00000000,
		B00111100,    // A
		B01000010,
		B01111110,
		B01000010,
		B00000000},
	       
               {B00000000,
		B01000010,
		B01000100,
		B01111000,    // K
		B01000100,
		B01000010,
		B01000010,
		B00000000},

	       {B00000000,
		B00000000,
		B00000000,
		B00000000,    // {BLANK}
		B00000000,
		B00000000,
		B00000000,
		B00000000}};


int menuTimer = 0;
int menuLogoNumber = 0;
int scoreCycleNumber = 0;

int difficultyLevel = 1;

void setup() {
  Serial.begin(9600);  
  gamer.begin();
}

void loop() {
  int timeSinceLastUpdate = millis() - lastUpdate;
  if (timeSinceLastUpdate < (1000 / gameUpdateRate)) {
    if ((gameMode == gmPlay) && (invertDisplay == true)) {
      renderPlay(invertDisplay);
    }
    invertDisplay = false;
    delay((1000 / gameUpdateRate)-timeSinceLastUpdate);
  }
  lastUpdate = millis();
  if (gameMode == gmMenu) {
    menuTimer += (1000 / gameUpdateRate);
    if (menuTimer > 350) {
      menuLogoNumber ++;
      menuTimer -= 350;
      if (menuLogoNumber > 4) menuLogoNumber = 0;
    }
    if (gamer.isPressed(START)) {
      beginGame();
    }
    if (gamer.isPressed(UP) || gamer.isPressed(DOWN) || gamer.isPressed(LEFT) || gamer.isPressed(RIGHT)) {
      toSelect();
    }
    gamer.printImage(menuFrames[menuLogoNumber]);
  } else if (gameMode == gmPlay) {
    dirCheck();
    moveAndCollision();
    shrinkField();
    renderPlay(invertDisplay);
    if (field[headX][headY] == 0 && gameOver){ // the snake has shrunk to nothing
      toScore();
    }
  } else if (gameMode == gmScore) {
    menuTimer += (1000 / gameUpdateRate);
    if (menuTimer > 350) {
      scoreCycleNumber ++;
      menuTimer -= 350;
      if (scoreCycleNumber > 4) scoreCycleNumber = 4-scoreLength;
    }
    if (gamer.isPressed(START)) {
      toMenu();
    }
    gamer.printImage(scoreFrames[scoreCycleNumber]);    
  } else if (gameMode == gmSelect) {
    selectDifficulty();
    if (gamer.isPressed(START)) {
      beginGame();
    }
    gamer.printImage(scoreFont[difficultyLevel]);
  }
}  

void selectDifficulty() {
  if ((gamer.isPressed(UP) || gamer.isPressed(RIGHT)) && difficultyLevel < 5) difficultyLevel++;
  if ((gamer.isPressed(DOWN) || gamer.isPressed(LEFT)) && difficultyLevel > 1) difficultyLevel--;
}  

void dirCheck() {
  if (gamer.isPressed(UP) && dir != dirDown) dir = dirUp; //check the snake is not travelling the opposite direction (would instakill)
  if (gamer.isPressed(RIGHT) && dir != dirLeft) dir = dirRight;
  if (gamer.isPressed(DOWN) && dir != dirUp) dir = dirDown;
  if (gamer.isPressed(LEFT) && dir != dirRight) dir = dirLeft;
}

void moveAndCollision() {
  int newHeadY = headY;
  int newHeadX = headX;
  if (!gameOver) {
    if (dir == dirUp) {
      newHeadY--;
    } else if (dir == dirRight) {
      newHeadX++;
    } else if (dir == dirDown) {
      newHeadY++;
    } else if (dir == dirLeft) {
      newHeadX--;
    }
    // wrap
    if (newHeadX < 0) newHeadX = 7;
    if (newHeadY < 0) newHeadY = 7;
    if (newHeadX > 7) newHeadX = 0;
    if (newHeadY > 7) newHeadY = 0;
  
    if (field[newHeadX][newHeadY] > 1) { //greater than 1 because we're about to shrink the field
      gameOver = true;
      invertDisplay = true;
    }
    if ((newHeadX == fruitX) && (newHeadY == fruitY)) {
      generateFruit();
      snakeLen ++;
      score += difficultyLevel;
    }
    headX = newHeadX;
    headY = newHeadY;
    field[headX][headY] = (snakeLen+1); //snakeLength+1 because we haven't yet shrunk each field square
  }
}

void generateFruit() {
  int x = random(8);
  int y = random(8);
  int i = 0;
  while ((field[x][y] > 0) || (i < 128)) {
    x = random(8);
    y = random(8);
    i++;
  }
  fruitX = x;
  fruitY = y;
  invertDisplay = true;
}

void shrinkField() {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (field[x][y] > 0) {
        field[x][y]--;
      }
    }
  }
}

void renderPlay(boolean invert) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (field[x][y] > 0) {
        if (invert == true) {
          gamer.display[x][y] = LOW;
        } else {
          gamer.display[x][y] = HIGH;
        }
      } else {
        if (invert == true) {
          gamer.display[x][y] = HIGH;
        } else {
          gamer.display[x][y] = LOW;      
        }
      }
    }
  }
  if (invert == true) {
    gamer.display[fruitX][fruitY] = LOW;
  } else {
    gamer.display[fruitX][fruitY] = HIGH;      
  }
  gamer.updateDisplay();
}

void beginGame() {
  randomSeed(millis()); 
  gameOver = false;
  gameMode = gmPlay;
  generateFruit();
  score = 0;
  headX = 3;
  headY = 3;
  dir = dirRight;
  snakeLen = 3;
  invertDisplay = true;  
  setUpdateRate();
}

void setUpdateRate() {
  if (difficultyLevel == 1) gameUpdateRate = 4;
  if (difficultyLevel == 2) gameUpdateRate = 7;
  if (difficultyLevel == 3) gameUpdateRate = 10;
  if (difficultyLevel == 4) gameUpdateRate = 13;
  if (difficultyLevel == 5) gameUpdateRate = 16;
}

void generateScore() {
  scoreCycleNumber = 0;  
  scoreLength = 0;
  int char1 = 0;
  int char2 = 0;
  int char3 = 0;
  int char4 = 0;
  if (score >= 1000){
    char1 = (score / 1000);
    score -= (1000 * char1);
    if (scoreLength == 0) scoreLength = 4;
  }
  if (score >= 100){
    char2 = (score / 100);
    score -= (100 * char2);
    if (scoreLength == 0) scoreLength = 3;
  }
  if (score >= 10){
    char3 = (score / 10);
    score -= (10 * char3);
    if (scoreLength == 0) scoreLength = 2;
  }
  char4 = score;
  if (scoreLength == 0) scoreLength = 1;
  for (int i = 0; i < 8; i++){
    scoreFrames[0][i] = scoreFont[char1][i];
  }
  for (int i = 0; i < 8; i++){
    scoreFrames[1][i] = scoreFont[char2][i];
  }  
  for (int i = 0; i < 8; i++){
    scoreFrames[2][i] = scoreFont[char3][i];
  }  
  for (int i = 0; i < 8; i++){
    scoreFrames[3][i] = scoreFont[char4][i];
  }    
  for (int i = 0; i < 8; i++){
    scoreFrames[4][i] = scoreFont[10][i];
  }
  scoreCycleNumber = 4-scoreLength;
}

void toMenu() {
  gameMode = gmMenu;
  menuLogoNumber = 0;
  menuTimer = 0;
  score = 0;
}

void toScore() {
  generateScore();
  gameMode = gmScore;
}

void toSelect() {
  gameMode = gmSelect;
}

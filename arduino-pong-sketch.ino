#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Define Pins
#define OLED_RESET -1
#define BEEPER A3
#define CONTROL_A A0
#define CONTROL_B A1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//Define Visuals
#define FONT_SIZE 2
#define PADDLE_WIDTH 4
#define PADDLE_HEIGHT 10
#define PADDLE_PADDING 10
#define BALL_SIZE 3
#define SCORE_PADDING 10
#define EFFECT_SPEED 0.5
#define MIN_Y_SPEED 0.5
#define MAX_Y_SPEED 2
//Define Variables

int game = -1; //0: pong-1vs1; 1: pongCPU; 2: scketch
int menuIndex = 0;
const int menuItems = 3;
String menu[menuItems] = {"PONG-1:1", "PONG-CPU", "SKETCH"};

int paddleLocationA = 0;
int paddleLocationB = 0;
float ballX = SCREEN_WIDTH/2;
float ballY = SCREEN_HEIGHT/2;
float ballSpeedX = 2;
float ballSpeedY = 1;
int lastPaddleLocationA = 0;
int lastPaddleLocationB = 0;
int scoreA = 0;
int scoreB = 0;


//Setup
void setup()
{
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  splash();
  display.setTextColor(WHITE);
  display.setTextSize(FONT_SIZE);
  display.clearDisplay();

  randomSeed(analogRead(0));
  //int controlA = analogRead(CONTROL_A);
  //menuIndex=controlA>500?1:2; 
}

void showMenu() {

  int controlB = analogRead(CONTROL_B);  
  while (abs(controlB - analogRead(CONTROL_B)) < 10) { //controlB attiva la selezione corrente
    
    int controlA = analogRead(CONTROL_A); 

    int menuSelect = map(controlA, 100, 900, 0, menuItems - 1);
    
    if(menuSelect != menuIndex) {
      menuIndex = menuSelect;
      drawMenu();
    }
  
    //if(abs(controlB - analogRead(CONTROL_B)) < 5){
    //  game=menuSelect;
    // }
  }  

  game=menuIndex;
  display.clearDisplay();
  
  Serial.print("game: ");
  Serial.println(game);
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  
  for(int i = 0; i < menuItems; i++) {
    if(i == menuIndex) {
      display.print("> "); // Indica la selezione corrente
    } else {
      display.print("  ");
    }
    display.println(menu[i]);
  }
  
  display.fillRect(0,SCREEN_HEIGHT-10,SCREEN_WIDTH,10,WHITE);
  display.setTextColor(BLACK);
  centerPrint("A:naviga B:seleziona",SCREEN_HEIGHT-9,1);
  
  display.display();
}

void splash()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  centerPrint("PONG",0,2);
  centerPrint("+",16,1);
  centerPrint("SKETCH",25,2);
  centerPrint("By Bishop87",44,1);
  display.fillRect(0,SCREEN_HEIGHT-10,SCREEN_WIDTH,10,WHITE);
  display.setTextColor(BLACK);
  centerPrint("Attendi il menu...",SCREEN_HEIGHT-9,1);
  display.display();
  delay(2000);
  soundStart();
  /*
  int controlA = analogRead(CONTROL_A);
  int controlB = analogRead(CONTROL_B);
  while (abs(controlA - analogRead(CONTROL_A) + controlB - analogRead(CONTROL_B)) < 10) {
    // show as long as the total absolute change of
    // both potmeters is smaler than 5
  }*/
  
}

void loop()
{
  switch(game){
    case 0: //pong1vs1
      calculateMovement();
      drawPong();
      break;
    case 1: //pongCPU
      calculateMovementCPU();
      drawPong();
      break;  
    case 2: //sketch
      drawSchetch();
      break;
    default: 
      drawMenu();
      showMenu(); //menu
  }
}

void calculateMovement(){
  int controlA = analogRead(CONTROL_A);
  int controlB = analogRead(CONTROL_B);
  paddleLocationA = map(controlA, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  paddleLocationB = map(controlB, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  int paddleSpeedA = paddleLocationA - lastPaddleLocationA;
  int paddleSpeedB = paddleLocationB - lastPaddleLocationB;
  ballX += ballSpeedX;
  ballY += ballSpeedY;
  //bounce from top and bottom
  if (ballY >= SCREEN_HEIGHT - BALL_SIZE || ballY <= 0) {
    ballSpeedY *= -1;
    soundBounce();
  }
  //bounce from paddle A
  if (ballX >= PADDLE_PADDING && ballX <= PADDLE_PADDING+BALL_SIZE && ballSpeedX < 0) {
    if (ballY > paddleLocationA - BALL_SIZE && ballY < paddleLocationA + PADDLE_HEIGHT) {
      soundBounce();
      ballSpeedX *= -1;

      addEffect(paddleSpeedA);
    }
  }
  //bounce from paddle B
  if (ballX >= SCREEN_WIDTH-PADDLE_WIDTH-PADDLE_PADDING-BALL_SIZE && ballX <= SCREEN_WIDTH-PADDLE_PADDING-BALL_SIZE && ballSpeedX > 0) {
    if (ballY > paddleLocationB - BALL_SIZE && ballY < paddleLocationB + PADDLE_HEIGHT) {
      soundBounce();
      ballSpeedX *= -1;

      addEffect(paddleSpeedB);
    }
  }
  //score points if ball hits wall behind paddle
  if (ballX >= SCREEN_WIDTH - BALL_SIZE || ballX <= 0) {
    if (ballSpeedX > 0) {
      scoreA++;
      ballX = SCREEN_WIDTH / 4;
    }
    if (ballSpeedX < 0) {
      scoreB++;
      ballX = SCREEN_WIDTH / 4 * 3;
    }
    soundPoint();  
  }
  //set last paddle locations
  lastPaddleLocationA = paddleLocationA;
  lastPaddleLocationB = paddleLocationB;  
}


void calculateMovementCPU(){
  //int controlA = analogRead(CONTROL_A);
  int controlB = analogRead(CONTROL_B);
  
  //paddleLocationA = map(controlA, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
  if(paddleLocationA + half_paddle > ballY) {
    paddleLocationA -= random(1,3);
  }
  if(paddleLocationA + half_paddle < ballY) {
    paddleLocationA += random(1,3);
  }
  
  paddleLocationB = map(controlB, 0, 1023, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  
  int paddleSpeedA = paddleLocationA - lastPaddleLocationA;
  int paddleSpeedB = paddleLocationB - lastPaddleLocationB;

  ballX += ballSpeedX;
  ballY += ballSpeedY;
  
  //bounce from top and bottom
  if (ballY >= SCREEN_HEIGHT - BALL_SIZE || ballY <= 0) {
    ballSpeedY *= -1;
    soundBounce();
  }
  //bounce from paddle A
  if (ballX >= PADDLE_PADDING && ballX <= PADDLE_PADDING+BALL_SIZE && ballSpeedX < 0) {
    if (ballY > paddleLocationA - BALL_SIZE && ballY < paddleLocationA + PADDLE_HEIGHT) {
      soundBounce();
      ballSpeedX *= -1;

      addEffect(paddleSpeedA);
    }
  }
  //bounce from paddle B
  if (ballX >= SCREEN_WIDTH-PADDLE_WIDTH-PADDLE_PADDING-BALL_SIZE && ballX <= SCREEN_WIDTH-PADDLE_PADDING-BALL_SIZE && ballSpeedX > 0) {
    if (ballY > paddleLocationB - BALL_SIZE && ballY < paddleLocationB + PADDLE_HEIGHT) {
      soundBounce();
      ballSpeedX *= -1;

      addEffect(paddleSpeedB);
    }
  }
  //score points if ball hits wall behind paddle
  if (ballX >= SCREEN_WIDTH - BALL_SIZE || ballX <= 0) {
    if (ballSpeedX > 0) {
      scoreA++;
      ballX = SCREEN_WIDTH / 4;
    }
    if (ballSpeedX < 0) {
      scoreB++;
      ballX = SCREEN_WIDTH / 4 * 3;
    }
    soundPoint();  
  }
  //set last paddle locations
  lastPaddleLocationA = paddleLocationA;
  lastPaddleLocationB = paddleLocationB;  
}

void drawPong(){
  display.clearDisplay();
  display.setTextSize(FONT_SIZE);
  //draw paddle A
  display.fillRect(PADDLE_PADDING,paddleLocationA,PADDLE_WIDTH,PADDLE_HEIGHT,WHITE);
  //draw paddle B
  display.fillRect(SCREEN_WIDTH-PADDLE_WIDTH-PADDLE_PADDING,paddleLocationB,PADDLE_WIDTH,PADDLE_HEIGHT,WHITE);
  //draw center line
  for (int i=0; i<SCREEN_HEIGHT; i+=4) {
    display.drawFastVLine(SCREEN_WIDTH/2, i, 2, WHITE);
  }
  //draw ball
  display.fillRect(ballX,ballY,BALL_SIZE,BALL_SIZE,WHITE);
  //print scores
  //backwards indent score A. This is dirty, but it works ... ;)
  int scoreAWidth = 5 * FONT_SIZE;
  if (scoreA > 9) scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 99) scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 999) scoreAWidth += 6 * FONT_SIZE;
  if (scoreA > 9999) scoreAWidth += 6 * FONT_SIZE;
  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH/2 - SCORE_PADDING - scoreAWidth,0);
  display.print(scoreA);
  display.setCursor(SCREEN_WIDTH/2 + SCORE_PADDING+1,0); //+1 because of dotted line.
  display.print(scoreB);
  display.display();
}

void addEffect(int paddleSpeed)
{
  float oldBallSpeedY = ballSpeedY;
  //add effect to ball when paddle is moving while bouncing.
  //for every pixel of paddle movement, add or substact EFFECT_SPEED to ballspeed.
  for (int effect = 0; effect < abs(paddleSpeed); effect++) {
    if (paddleSpeed > 0) {
      ballSpeedY += EFFECT_SPEED;
    } else {
      ballSpeedY -= EFFECT_SPEED;
    }
  }
  //limit to minimum speed
  if (ballSpeedY < MIN_Y_SPEED && ballSpeedY > -MIN_Y_SPEED) {
    if (ballSpeedY > 0) ballSpeedY = MIN_Y_SPEED;
    if (ballSpeedY < 0) ballSpeedY = -MIN_Y_SPEED;
    if (ballSpeedY == 0) ballSpeedY = oldBallSpeedY;
  }
  //limit to maximum speed
  if (ballSpeedY > MAX_Y_SPEED) ballSpeedY = MAX_Y_SPEED;
  if (ballSpeedY < -MAX_Y_SPEED) ballSpeedY = -MAX_Y_SPEED;
}

void soundStart()
{
  tone(BEEPER, 250);
  delay(100);
  tone(BEEPER, 500);
  delay(100);
  tone(BEEPER, 1000);
  delay(100);
  noTone(BEEPER);
}
void soundBounce()
{
  tone(BEEPER, 500, 50);
}
void soundPoint()
{
  tone(BEEPER, 150, 150);
}
void centerPrint(char *text, int y, int size)
{
  display.setTextSize(size);
  display.setCursor(SCREEN_WIDTH/2 - ((strlen(text))*6*size)/2,y);
  display.print(text);
}

void drawSchetch(){
  //static int phase = 0;
  static unsigned long next_serial_timestamp = 0;
  int H = map(analogRead(A0), 0, 1023, 0, SCREEN_WIDTH - 1);
  int V = map(analogRead(A1), 0, 1023, 0, SCREEN_HEIGHT - 1);

  display.drawPixel(H, V, WHITE);
  display.display();
}

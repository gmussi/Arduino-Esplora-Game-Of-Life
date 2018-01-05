/**********************************************************************************

   This is an implementation of the Conway's Game of Life for the Arduino Esplora.
   To learn more about Game of Life, refer to the following:
   https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life

   Author: Guilherme Mussi <https://github.com/gmussi>
   
   Github project page: https://github.com/gmussi/ArduinoEsploraGameOfLife
   Arduino Project Hub:   https://create.arduino.cc/projecthub/imagile/conway-s-game-of-life-79bb05

 *********************************************************************************

   USING THE APPLICATION:

   Slider:
     Sets the amount (in %) of black squares when randomizing a generation
   UP button:
     Resets the generation with a new random one
   DOWN button:
     Pauses the generations and stay with the current generation on the screen
   LEFT button:
     Automatically advances from one generation to the next ("play")
   RIGHT button:
     Advances one generation and then pauses
   Joystick:
     Select a specific cell in the grid.
     Click the joystick to invert the value of that cell.

 **********************************************************************************

   HARDWARE:

   For the hardware components, I am using the following:

   Arduino Esplora - https://store.arduino.cc/arduino-esplora
   Arduino LCD Screen - https://store.arduino.cc/arduino-lcd-screen

   The scren resolution is 160 pixels wide and 128 pixels high.

 **********************************************************************************

   LIBRARIES:

   Includes the libraries for the Arduino Esplora, SPI and TFT.

 ***********************************************************************************
*/
#include <Esplora.h>
#include <SPI.h>
#include <TFT.h>

const int COLS = 32; // 160 / 5
const int ROWS = 32; // 128 / 4

boolean current_state[ROWS][COLS]; // stores the current state of the cells

boolean play = false; // if true, application will automatically advance to the next state

int lastR = 0; // last cursor row
int lastC = 0; // last cursor column

boolean lastJoystickButton = HIGH; // last joystick button state

void setup() {
  // initialize the dispay
  EsploraTFT.begin();

  // make the background white
  EsploraTFT.background(255, 255, 255);

  // create seed to use the random function
  randomSeed(Esplora.readSlider());

  // initialize cells with random values
  randomize();
}

/**
 * This method transitions the current state to the next state, using the following rules:
 *  
 * 1. Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
 * 2. Any live cell with two or three live neighbours lives on to the next generation.
 * 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
 * 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 *
 * For the purpose of this application, live == TRUE and dead == FALSE
 */
void next() {
  int x;
  int y;
  boolean highlighted;
  boolean value;
  boolean next[32][32]; // stores the next state of the cells

  for (int r = 0; r < ROWS; r++) { // for each row
    for (int c = 0; c < COLS; c++) { // and each column
      // count how many live neighbors this cell has
      int liveNeighbors = 0;
      for (int i = -1; i < 2; i++) {
        y = r + i;
        if (y == -1) {
          y = 31;
        } else if (y == 32) {
          y = 0;
        }
        for (int j = -1; j < 2; j++) {
          if (i != 0 || j != 0) {
            x = c + j;
            if (x == -1) {
              x = 31;
            } else if (x == 32) {
              x = 0;
            }

            if (current_state[y][x]) {
              liveNeighbors++;
            }
          }
        }
      }

      // apply the rules
      if (current_state[r][c] && liveNeighbors >= 2 && liveNeighbors <= 3) { // live cells with 2 or 3 neighbors remain alive
        value = true;
      } else if (!current_state[r][c] && liveNeighbors == 3) { // dead cells with 3 neighbors become alive
        value = true;
      } else {
        value = false;
      }

      next[r][c] = value;

      // checks if the cursor is on top of this cell
      highlighted = (r == lastR && c == lastC);

      // set color of this cell
      setColor(value, highlighted);
      
      // draw the cell
      EsploraTFT.rect(c * 5, r * 4, 5, 4);
    }
  }

  // discard the old state and keep the new one
  memcpy(current_state, next, sizeof next);
}

/**
 * Move the cursor one cell to the left.
 * If this is the last cell, start on the other side of the grid.
 */
void moveCursorLeft() {
  setColor(current_state[lastR][lastC], false);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
  lastC = lastC == 0 ? 31 : lastC - 1;
  setColor(current_state[lastR][lastC], true);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
}

/**
 * @see moveCursorLeft
 */
void moveCursorRight() {
  setColor(current_state[lastR][lastC], false);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
  lastC = lastC == 31 ? 0 : lastC + 1;
  setColor(current_state[lastR][lastC], true);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
}

/**
 * @see moveCursorLeft
 */
void moveCursorUp() {
  setColor(current_state[lastR][lastC], false);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
  lastR = lastR == 31 ? 0 : lastR + 1;
  setColor(current_state[lastR][lastC], true);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
}

/**
 * @see moveCursorLeft
 */
void moveCursorDown() {
  setColor(current_state[lastR][lastC], false);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
  lastR = lastR == 0 ? 31 : lastR - 1;
  setColor(current_state[lastR][lastC], true);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
}

/**
 * Inverts the state (alive, dead) of the cell in which the cursor is currently on
 */
void invertCurrentCell() {
  current_state[lastR][lastC] = !current_state[lastR][lastC];
  setColor(current_state[lastR][lastC], true);
  EsploraTFT.rect(lastC * 5, lastR * 4, 5, 4);
}

/**
 * Main application loop
 * 
 * First, check if cursor needs to move in any direction.
 * If it dooes, move the cursor and apply a small delay, so that it does not move too fast.
 * 
 * Then check for each input button to see if any action must be performed.
 * 
 * In case the application is in PLAY state, transition to the next state automatically.
 */
void loop () {
  boolean advance = false;
  int joyX = Esplora.readJoystickX(); // read value of joystick
  int joyY = Esplora.readJoystickY(); // read value of joystick

  if (joyX > 256) { // go left
    moveCursorLeft();
    delay(100); 
  } else if (joyX < -256) { // go right
    moveCursorRight();
    delay(100);
  }

  if (joyY < -256) { // go down
    moveCursorDown();
    delay(100); 
  } else if (joyY > 256) { // go up
    moveCursorUp();
    delay(100); 
  }

  int joystickButton = Esplora.readJoystickButton();
  if (Esplora.readButton(SWITCH_UP) == LOW) {
    randomize();
  } else if (Esplora.readButton(SWITCH_DOWN) == LOW) {
    play = false;
  } else if (Esplora.readButton(SWITCH_RIGHT) == LOW) {
    play = true;
  } else if (Esplora.readButton(SWITCH_LEFT) == LOW) {
    advance = true;
  } else if (joystickButton == LOW && lastJoystickButton == HIGH) {
    invertCurrentCell();
  }
  if (advance || play) {
    next();
  }

  lastJoystickButton = joystickButton;
}

/**
 * Sets the color of the cell on the following rules:
 * 
 * Cell is alive, cursor is on top: Green
 * Cell is dead, cursor is on top: Orange
 * Cell is alive: Black
 * Cell is dead: White
 */
void setColor(boolean value, boolean highlighted) {
  if (value && highlighted) {
    EsploraTFT.stroke(0, 204, 0);
    EsploraTFT.fill(0, 204, 0);
  } else if (value && !highlighted) {
    EsploraTFT.stroke(0, 0, 0);
    EsploraTFT.fill(0, 0, 0);
  } else if (highlighted) {
    EsploraTFT.stroke(255, 153, 51);
    EsploraTFT.fill(255, 153, 51);
  } else {
    EsploraTFT.stroke(255, 255, 255);
    EsploraTFT.fill(255, 255, 255);
  }
}

/**
 * Creates a random state.
 * The slider can be used to control the percentage of alive and dead cells.
 * The more to the right the slider is, the more alive cells are created.
 */
void randomize() {
  int slider = map(Esplora.readSlider(), 0, 1023, 0, 100);
  int num;
  boolean value;
  boolean highlighted;

  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      num = random(1, 100);

      value = num >= slider;
      current_state[r][c] = value;

      highlighted = (r == lastR && c == lastC);

      setColor(value, highlighted);

      EsploraTFT.rect(c * 5, r * 4, 5, 4);
    }
  }
}

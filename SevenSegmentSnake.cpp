#include "SevenSegmentSnake.h"

SevenSegmentSnake::SevenSegmentSnake(uint8_t pinClk, uint8_t pinDIO) : SevenSegmentFun(pinClk, pinDIO) {
  randomSeed(analogRead(0));

  uint8_t outerEdges = (4 * TM1637_MAX_LINES) + (TM1637_MAX_COLOM * 2);
  uint8_t widthEdged = (TM1637_MAX_COLOM * 2);

  unsigned long incrementer = 0;

  for (uint8_t i=0; i < outerEdges; i++) {
    for (uint8_t j=0; j < TM1637_MAX_COLOM; j++) {
      uint8_t val = 0;

      if ( i == j) {
        val = TM1637_CHAR_SNAKE_0;
      }
      // top right edge
      else if ( i == TM1637_MAX_COLOM && j == 3) {
        val = TM1637_CHAR_SNAKE_1;
      }
      // bottom left edge
      else if ( i == (TM1637_MAX_COLOM + 1) && j == 3) {
        val = TM1637_CHAR_SNAKE_2;
      }
      // bottom edges
      else if ( i + j == (widthEdged + 1) ) {
        val = TM1637_CHAR_SNAKE_3;
      }
      // bottom left edge
      else if ( i == (widthEdged + 2) && j == 0) {
        val = TM1637_CHAR_SNAKE_4;
      }
      // top left edge
      else if ( i == (widthEdged + 3) && j == 0) {
        val = TM1637_CHAR_SNAKE_5;
      }

      _steps[incrementer++] = { loc: j, value: val }; 
    }
  }
}

void SevenSegmentSnake::process() {
  if (_currentStep >= STEPS_LENGTH) {
    _currentStep = 0;
  }

  SnakeStep step = _steps[_currentStep];
  _rawBuffer[step.loc] = step.value;
  printRaw(_rawBuffer, 4, 0);

  _currentStep++;
}

#ifndef SevenSegmentSnake_h
#define SevenSegmentSnake_h

#include <SevenSegmentTM1637.h>
#include <SevenSegmentExtended.h>
#include <SevenSegmentFun.h>

#define STEPS_LENGTH ((4 * TM1637_MAX_LINES) + (TM1637_MAX_COLOM * 2)) * TM1637_MAX_COLOM

struct SnakeStep {
    uint8_t loc;
    uint8_t value;
};

class SevenSegmentSnake : public SevenSegmentFun {
  public:
    SevenSegmentSnake(uint8_t pinClk, uint8_t pinDIO);
    void process();
    void nonBlockingSnake();
  private:
    unsigned long _currentStep = 0;
    SnakeStep _steps[STEPS_LENGTH];
};

#endif

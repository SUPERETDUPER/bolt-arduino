#include "game.h"

#include "button.h"
#include "screen.h"
#include "led.h"
#include "timer.h"
#include "const.h"
#include "logger.h"

namespace game {
  namespace {
    const unsigned long GAME_TIME = 3000;
    
    unsigned long endTime;
    
    unsigned int buttonsPressed = 0;

    void countDown() {
      screen::display("3");
      delay(1000);
      screen::display("2");
      delay(1000);
      screen::display("1");
      delay(1000);
    }

    void runMain() {
      while (millis() < endTime) {
        
        int buttonNumber = random(0, constants::NUMBER_OF_LEDS - 1);  //Generate random button

        button::clearLast();
        led::turnOn(buttonNumber);  //Turn on led and wait for button press
        led::shiftOut();
        
        while(not button::isPressed(buttonNumber) and millis() < endTime){
          timer::checkUpdateDisplay();
        }

        if (millis() < endTime){
          buttonsPressed ++; //Increment counter
        }
        
        led::turnOff(buttonNumber);
      }
    }
  }

  unsigned long getRemainingTime() {
    unsigned long remainingTime = endTime - millis();

    if (remainingTime > 0) {
      return remainingTime;
    }
    
    return 0;
  }

  void start() {
    countDown();
    
    endTime = GAME_TIME + millis();

    runMain();

    screen::display(String(buttonsPressed) + " buttons pressed");
  }
}

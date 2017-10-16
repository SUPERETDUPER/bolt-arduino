#include "button.h"
#include "led.h"
#include "screen.h"
#include "game.h"
#include "timer.h"
#include "const.h"
#include "flasher.h"
#include "logger.h"
#include "bluetooth.h"


void setup() {
  Serial.begin(115200);

  //Generate new random seed
  randomSeed(analogRead(0));

  //Setup
  bluetooth::setup();
  button::setup();
  led::setup();
  logger::log(logger::TYPE_INFO, "main", "Setup done");

  startReadyMode();
}


void loop() {
  
  bluetooth::readReceived();
  
  if (bluetooth::shouldGoOnline()) { //If connected to bluetooth go in online mode
    flasher::stopFlashing(0);
    screen::display("ONLINE");

    bluetooth::listen();

    startReadyMode();
  }
  else if (button::isPressed(0)) { //If middle button pressed go in offline mode
    flasher::stopFlashing(0);

    game::start();

    startReadyMode();
  }

  flasher::checkFlash(); // Will flash if should flash
}

void startReadyMode() {
  screen::display("READY");
  flasher::startFlashing(0);
}

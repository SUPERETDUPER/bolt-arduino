#ifndef BOLTARDUINO
#define BOLTARDUINO

#include "button.h"
#include "led.h"
#include "screen.h"
#include "game.h"
#include "clock.h"
#include "const.h"
#include "controller.h"
#include "flasher.h"


void setup() {
  Serial.begin(115200);

  //Attach interrupt for 64 button shield
  attachInterrupt(digitalPinToInterrupt(P_BUTTON_INTERRUPT), button_ISR, FALLING);

  //Generate new random seed
  randomSeed(analogRead(0));

  led_setup();
  clock_setup();
  flasher_setup(1000);
}

void loop() {
  screen_display("READY");

  led_set(1, LED_STATE_FLASHING);
  button_wait(1);

  game_start();
}

#endif

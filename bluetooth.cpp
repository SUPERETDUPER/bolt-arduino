#include "bluetooth.h"

#include "constants.h"
#include "led.h"
#include "logger.h"
#include "button.h"

#include <SoftwareSerial.h>

namespace bluetooth {
  namespace {
    SoftwareSerial BT(constants::P_SOFTWARE_SERIAL_RX, constants::P_SOFTWARE_SERIAL_TX);

    const char START_OF_PACKET = 0x02;  //Start of text
    const char END_OF_PACKET = 0x03;  //End of text
    const char ACKNOWLEDGE = 0x06; //Acknowledge

    const char C_BEGIN = 0x42; //"B"
    const char C_END = 0x45; //"E"
    const char C_TURN_OFF_LED = 0x49; //"I"
    const char C_TURN_ON_LED = 0x4F; //"O"
    const char C_SHIFT_OUT = 0x53; //"S"

    const unsigned long PACKET_TIMEOUT = 1000;
    const unsigned int ACKNOWLEDGE_TIMEOUT = 2000;
    const long NO_TIMEOUT = -1;

    const char BLUETOOTH_PIN[4] = "1234";

    bool connection = false;

    volatile long acknowledgeTimeout = NO_TIMEOUT;

    void acknowledgePacket() {
      BT.write(ACKNOWLEDGE);
      logger::log(logger::TYPE_INFO, "bluetooth", "Acknowledged packet");
    }

    void processPacketContent(String packetContent) {
      char command = packetContent[0];
      String argument = packetContent.substring(1);

      if (command = C_BEGIN) {
        connection = true;
        acknowledgePacket();
      }

      else if (connection) {
        switch (command) {
          case C_END:
            connection = false;
            break;
          case C_TURN_ON_LED:
            led::turnOn(argument.toInt());
            break;
          case C_TURN_OFF_LED:
            led::turnOff(argument.toInt());
            break;
          case C_SHIFT_OUT:
            led::shiftOut();
            break;
          default:
            logger::log(logger::TYPE_ERROR, "bluetooth", "can't parse packet : " + packetContent);
        }
        acknowledgePacket();
      }
    }

    String getPacketContent() {
      String packet = "";

      unsigned long timeAtLastByte = millis();

      while (millis() < timeAtLastByte + PACKET_TIMEOUT) {
        if (BT.available()) {

          char newByte = BT.read();
          timeAtLastByte = millis();

          if (newByte == END_OF_PACKET) {
            logger::log(logger::TYPE_INFO, "bluetooth", "Received packet : " + packet);
            return packet;
          } else {
            packet += newByte;
          }
        }
      }
      logger::log(logger::TYPE_WARNING, "bluetooth", "Timout while reading packet content");
      return String(C_END); // If while loop ended because off timeout, end game
    }
  }

  void checkSerial() {
    while (Serial.available()){
      if (Serial.read() == 67){
        delay(10);
        String buttonNumber = "";
        while (Serial.available()){
          buttonNumber += (char) Serial.read();
          delay(10);
        }
        Serial.println("Got packet : " + buttonNumber);
        button::buttonPressedCallback(buttonNumber.toInt());
      }
    }
  }

  void readReceived() {
    String unknown = "";

    while (BT.available()) {
      char newByte = BT.read();


      switch (newByte) {
        case ACKNOWLEDGE:
          logger::log(logger::TYPE_INFO, "bluetooth", "Received acknowledge");
          acknowledgeTimeout = NO_TIMEOUT;
          break;
        case START_OF_PACKET:
          processPacketContent(getPacketContent());
          break;
        default:
          unknown += newByte;
          delay(10);
      }
    }

    if (not unknown.equals("")) {
      logger::log(logger::TYPE_WARNING, "bluetooth", "Unknown RX bytes : " + unknown);
    }

    if(constants::IS_DEBUGGING){
      checkSerial();
    }
  }

  void listen() {
    while (connection) {
      readReceived();
      if (acknowledgeTimeout != NO_TIMEOUT and millis() > acknowledgeTimeout) {
        connection = false;
        acknowledgeTimeout = NO_TIMEOUT;
      }
    }
  }

  void sendPacket(String packetContent) {
    logger::log(logger::TYPE_INFO, "bluetooth", "Sent packet : " + packetContent);
    BT.write(START_OF_PACKET);
    for (int i = 0 ; i < packetContent.length(); i++) {
      BT.write(packetContent.charAt(i));
    }
    BT.write(END_OF_PACKET);

    if (acknowledgeTimeout == NO_TIMEOUT) {
      acknowledgeTimeout = millis() + ACKNOWLEDGE_TIMEOUT;
    }
  }

  bool shouldGoOnline() {
    return connection;
  }

  void setup() {
    BT.begin(9600);
    //Set PIN to be same as phone
    BT.write("AT+PIN:");
    BT.write(BLUETOOTH_PIN);
    BT.write(13);
    BT.write(10);
  }
}

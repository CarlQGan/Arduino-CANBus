/*
 *  Manually controlling the go kart using arduino serial monitor
 */

#include <SPI.h> // using CAN_BUS_Shield from Seeed Studio

#define CAN_2515

// For Arduino MCP2515 Hat:
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
#if defined(CAN_2515)
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif

const float MAX_DIST = 3.0; // Maximum safe distance that the actuator can move (inches)
const int COMMAND_ID = 0xff0000; // Default command ID for CAN Actuator
const int REPORT_ID = 0xff0001; // Default report ID for CAN Actuator


void setup() {
    
    SERIAL_PORT_MONITOR.begin(115200);
    while(!Serial){};

    while (CAN_OK != CAN.begin(CAN_500KBPS)) { // init can bus : baudrate = 500k
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(10);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");
    
}

void loop() {
    unsigned char buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char receivedMsg[8];
    unsigned char len = 0;
    String bite2;
    String bite3;
    String dpos_lo;
    String dpos_hi;
  
    while (!Serial) {};

//    Serial.println("Please input actuator distance in inches:");

    // Accept user input
//    float inputDist = Serial.parseFloat();
    float inputDist = 2.0;

    // Clipping input range
    if (inputDist < 0.0) {
        inputDist = 0.0;
    }
    if (inputDist > MAX_DIST) {
        inputDist = MAX_DIST;
    }

    // Convert input to hex
    int intDist = inputDist * 1000 + 500; // in 0.001” steps
    String hexDist = String(intDist, HEX);
    int hexDistLen = hexDist.length();
    Serial.println("The distance in hex is: " + hexDist);

    // the least significant byte of position
    dpos_lo = hexDist.substring(hexDistLen - 2);
    bite2 = dpos_lo;
    // The most significant byte of position
    dpos_hi = hexDist.substring(0, hexDistLen - 2);

    // Clutch on, Motor off
    bite3 = bite3Parser(1, 0, dpos_hi);
    Serial.println(bite3);
    overwriteBuf(data, 0x0F, 0x4A, strHexToInt(bite2.c_str()), strHexToInt(bite3.c_str()), 0, 0, 0, 0);
    CAN.sendMsgBuf(COMMAND_ID, 0, 8, data);
    delay(20);

    // Clutch on, move motor and hold
    bite3 = bite3Parser(1, 1, dpos_hi);
    overwriteBuf(data, 0x0F, 0x4A, strHexToInt(bite2.c_str()), strHexToInt(bite3.c_str()), 0, 0, 0, 0);
    CAN.sendMsgBuf(COMMAND_ID, 0, 8, data);
    delay(1000);

    // Clutch on, Motor off
    bite3 = bite3Parser(1, 0, dpos_hi);
    overwriteBuf(data, 0x0F, 0x4A, strHexToInt(bite2.c_str()), strHexToInt(bite3.c_str()), 0, 0, 0, 0);
    CAN.sendMsgBuf(COMMAND_ID, 0, 8, data);
    delay(20);

    // Clutch off, Motor off
    bite3 = bite3Parser(0, 0, dpos_hi);
    overwriteBuf(data, 0x0F, 0x4A, strHexToInt(bite2.c_str()), strHexToInt(bite3.c_str()), 0, 0, 0, 0);
    CAN.sendMsgBuf(COMMAND_ID, 0, 8, data);
    delay(20);
}

void overwriteBuf(volatile byte* buf, int b0, int b1, int b2, int b3, int b4, int b5, int b6, int b7) {
    buf[0] = b0;
    buf[1] = b1;
    buf[2] = b2;
    buf[3] = b3;
    buf[4] = b4;
    buf[5] = b5;
    buf[6] = b6;
    buf[7] = b7;
}

void printArray(volatile byte* buf) {
    unsigned char size_of_myArray = 8;
    for (int i = 0; i < size_of_myArray; i++) {
        Serial.print(buf[i]);
        Serial.print('  ');
    }
    Serial.println();
}

int strHexToInt(char str[]) {
  // inputs a hex string (without prefix 0x) 
  // and return the int value
  return (int) strtol(str, 0, 16);
}

String bite3Parser(int ce, int m, String dpos_hi) {
    return String((int) (ce * pow(2, 7) + m * pow(2, 6) + strHexToInt(dpos_hi.c_str())), HEX);
}

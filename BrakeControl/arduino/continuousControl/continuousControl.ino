/*
 *  Accepting brake control for the go kart using arduino serial monitor
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

const float MIN_DIST = 0.0; // Minimun distance for actuator
const float MAX_DIST = 3.0; // Maximum safe distance that the actuator can move (inches)
const unsigned long COMMAND_ID = 0xFF0000; // Default command ID for CAN Actuator
const unsigned long REPORT_ID = 0xFF0001; // Default report ID for CAN Actuator
const char CLUTCH_MOTOR_OFF[8] = {0x0F, 0x4A, 0xC4, 0x09, 0, 0, 0, 0};
const char CLUTCH_ON[8] = {0x0F, 0x4A, 0xC4, 0x89, 0, 0, 0, 0};
const char POS_ZERO[8] = {0x0F, 0x4A, 0xC0, 0x00, 0, 0, 0, 0};



void setup() {
//    while (!Serial){
//        Serial.begin(115200);
//    };

    Serial.begin(115200);
    
    actuatorInit();

}

void loop() {
    
    // TODO: Accept User input
    setActuatorPosition(1.0);
    delay(1000);
    setActuatorPosition(2.5);
    delay(1000);
    setActuatorPosition(1.5);
    delay(1000);
    setActuatorPosition(2.0);
    delay(1000);
    setActuatorPosition(0.0);
    delay(3000);

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
  
    for (int i = 0; i < size_of_myArray; i++) { // print the data
        Serial.print(buf[i], HEX);
        Serial.print("\t");
    }
    
    Serial.println();
    Serial.println("-----------------------------");
}


/*  
 *   inputs a hex string (without prefix 0x) 
 *   and return the int value
 */
int strHexToInt(char str[]) {
    // inputs a hex string (without prefix 0x) 
    // and return the int value
    return (int) strtol(str, 0, 16);
}


/*  
 *   Format Byte 3 as String given clutch, motor flags
 *   and significant byte of position
 */
String posCmdBite3Parser(int ce, int m, String dpos_hi) {
    return String((int) (ce * pow(2, 7) + m * pow(2, 6) + strHexToInt(dpos_hi.c_str())), HEX);
}

/*  
 *   Initialize the Actuator
 */
void actuatorInit() {
    while (CAN_OK != CAN.begin(CAN_250KBPS)) { // init can bus : baudrate = 500k
        Serial.println("CAN init fail, retry...");
        delay(10);
    }
    Serial.println("CAN init ok!");

//    // Disable everything
//    CAN.sendMsgBuf(COMMAND_ID, 1, 8, CLUTCH_MOTOR_OFF);
//    delay(20);

    // Enable clutch for loop input
    CAN.sendMsgBuf(COMMAND_ID, 1, 8, CLUTCH_ON);
    delay(20);

    setActuatorPosition(0.0);
    delay(3000);
}

/*  
 *   Move the Actuator to designated position in inches.
 *   The Actuator will execute whatever 
 *   the latest command is immediately.
 */
void setActuatorPosition(float inputDist) {
    unsigned char data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    String bite2;
    String bite3;
    String dpos_lo;
    String dpos_hi;
    
    // Clipping input range
    if (inputDist < 0.0) {
        inputDist = 0.0;
    }
    if (inputDist > MAX_DIST) {
        inputDist = MAX_DIST;
    }
    Serial.println("The distance in inches is: " + String(inputDist));

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

    // Clutch on, Motor on and move to the desired position
    bite3 = posCmdBite3Parser(1, 1, dpos_hi);
    overwriteBuf(data, 0x0F, 0x4A, strHexToInt(bite2.c_str()), strHexToInt(bite3.c_str()), 0, 0, 0, 0);
    
    Serial.print("Sending data from hex ID: ");
    Serial.println(COMMAND_ID, HEX);
    printArray(data);
    
    CAN.sendMsgBuf(COMMAND_ID, 1, 8, data);
}

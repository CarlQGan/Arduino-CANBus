#include <Arduino.h>
#include <SPI.h>
#include <string.h>

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
#endif


/**
 * @brief  main setup function
 * @note   initialize the CAN Communication and reset the Actuator
 * @retval None
 */
void setup();

/**
 * @brief  main loop function
 * @note   take inputs and control the CAN
 * @retval None
 */
void loop();

/**
 * @brief  buffer constructor
 * @note   overwrite the 8 byte message buffer
 * @retval None
 */
void overwriteBuf(volatile byte* buf, int b0, int b1, int b2, int b3, int b4, int b5, int b6, int b7);

/**
 * @brief  print an 8-byte array
 * @note   for message display
 * @retval None
 */
void printArray(volatile byte* buf);

/**
 * @brief  convert a hex number of the type char array into int
 * @note   no preceeding '0x', for example: "8f", "c2", "15"
 * @retval the int value of the input hex
 */
int strHexToInt(char str[]);

/**
 * @brief  parse 
 * @note   overwrite the 8 byte message buffer
 * @retval None
 */
String posCmdBite3Parser(int ce, int m, String dpos_hi);




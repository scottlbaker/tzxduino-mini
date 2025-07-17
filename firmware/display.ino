
//=========================================================================
//
//  display.ino  -- display routines used by tzxduino.ino
//
//=========================================================================

#include "display.h"

// send a command to the display
void sendcommand (unsigned char com) {
  Wire.beginTransmission (OLED_address);    // begin transmitting
  Wire.write (0x80);                        // command mode
  Wire.write (com);
  Wire.endTransmission ();
}

// Adafruit 1306 OLED init sequence
void init_OLED(void) {
  #define OLED_INIT_SIZE    26
  const uint8_t oled_init [OLED_INIT_SIZE] = {
    0xAE,       // DISPLAY OFF
    0xD5,       // SETDISPLAYCLOCKDIV
    0x80,       // the suggested ratio 0x80
    0xA8,       // SSD1306_SETMULTIPLEX
    0x1F,       // 1/48 duty
    0xD3,       // SETDISPLAYOFFSET
    0x00,       // NO OFFSET
    0x40,       // SETSTARTLINE
    0x8D,       // CHARGEPUMP
    0x14,       //
    0x20,       // MEMORYMODE
    0x02,       // COM PIN HW CONFIG
    0xA1,       // SEGREMAP
    0xC8,       // COMSCANDEC
    0xDA,       //
    0x02,       // COMSCANDEC
    0x81,       // SETCONTRAS
    0xCF,       //
    0xD9,       // SETPRECHARGE
    0xF1,       //
    0xDB,       // SETVCOMDETECT
    0x40,       //
    0x2E,       // STOP SCROLL
    0xA4,       // DISPLAYALLON_RESUME
    0xA6,       // NORMAL DISPLAY
    0xAF        // DISPLAY ON
  };
  for (int i = 0; i < OLED_INIT_SIZE; i++)
    sendcommand (oled_init[i]);
  delay(100);
}

// send a byte
void SendByte (unsigned char data) {
  Wire.beginTransmission (OLED_address);    // begin transmitting
  Wire.write (0x40);                        // data mode
  Wire.write (data);
  Wire.endTransmission ();
}

// print a char
void sendChar (unsigned char data) {
  Wire.beginTransmission (OLED_address);    // begin transmitting
  Wire.write (0x40);                        // data mode
  for (int i = 0; i < 8; i++)
  Wire.write (pgm_read_byte (SpecFont[data - 0x20] + i));
  Wire.endTransmission ();
}

// set the cursor position
void setXY(unsigned char col,unsigned char row) {
  sendcommand(0xb0+row);                    // set page address
  sendcommand(0x00+(8*col&0x0f));           // set low col address
  sendcommand(0x10+((8*col>>4)&0x0f));      // set high col address
}

// print a string regardless the cursor position
void sendStr(unsigned char *string) {
  unsigned char i = 0;
  while (*string) {
    for (i = 0; i < 8; i++)
      SendByte (pgm_read_byte (SpecFont[*string - 0x20] + i));
    *string++;
  }
}

// print a string in coordinates X Y
void sendStrXY(char *string, int X, int Y) {
  setXY(X,Y);
  unsigned char i = 0;
  while (*string) {
    for (i = 0; i < 8; i++)
      SendByte (pgm_read_byte (SpecFont[*string - 0x20] + i));
    *string++;
  }
}

// turn display on
void displayOn(void) {
  sendcommand (0xaf);         // display on
}

// turn display off
void displayOff(void) {
  sendcommand (0xae);        // display off
}

// clear the display
void clearScreen(void) {
  unsigned char i, k;
  for (k = 0; k < 4; k++) {
    setXY(0,k);
    for (i = 0; i < 128; i++) SendByte(0);
  }
  setXY(0,0);
}

// clear a line
void clearLine(int row) {
  sendStrXY("                ",0,row);
}

// show logo
void showLogo (void) {
  for (int j=0; j<4; j++) {
    setXY(0,j);
    for (int i=0; i<128; i++)
      SendByte (pgm_read_byte(tzx_logo+(j<<7)+i));
  }
  delay(1500);
  clearScreen();
}

void showFileSize() {
  char tmp[8];
  itoa(fileSize,tmp,7);
  sendStrXY(tmp,0,2);
}

void showBytesRead() {
  char tmp[8];
  itoa(bytesRead,tmp,7);
  sendStrXY(tmp,8,2);
}

void showPercent() {
  setXY (8, 0);
  if (currpct < 10) {
    sendChar ('0' + currpct % 10);
  } else
  if (currpct < 100) {
    sendChar ('0' + currpct / 10);
    sendChar ('0' + currpct % 10);
  } else {
    sendChar ('1');
    sendChar ('0');
    sendChar ('0');
  }
  sendChar('%');
}

// display elapsed time
void showTime() {
  if (timeCount < 10) {
    setXY (15, 0);
    sendChar ('0' + timeCount % 10);
  } else if (timeCount < 100) {
    setXY (14, 0);
    sendChar ('0' + timeCount / 10);
    sendChar ('0' + timeCount % 10);
  } else {
    setXY (13, 0);
    sendChar ('0' + timeCount / 100);
    sendChar ('0' + timeCount / 10);
    sendChar ('0' + timeCount % 10);
  }
  // Serial.println(timeCount);   // for debug
}

// update the time counter
void updateTime() {
  if (millis () - timeDiff2 > 1000) {   // check switch every second
    timeCount++;                        // increment second counter
    timeDiff2 = millis ();              // reset the millisecond count
    if (timeCount > 999) timeCount=0;   // wrap at 999 seconds
    showTime();
  }
}

// for menu -- print a '*' to indicate current selection
void printStar() {
  sendStr(" *");
}

// for menu -- print ON or OFF status
void printOnOff(byte on) {
  if (on) {
    printtextF(PSTR("On"),1);
  } else {
    printtextF(PSTR("Off"),1);
  }
}

// Print constant text to screen
void printtextF(const char* text, int row) {
  char tmp[17];
  strncpy_P(tmp, text, 16);
  sendStrXY("                ",0,row);
  sendStrXY(tmp,0,row);
}

// Print text to screen
void printtext(char* text, int row) {
  sendStrXY("                ",0,row);
  sendStrXY(text,0,row);
}


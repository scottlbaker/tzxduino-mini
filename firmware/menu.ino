
//=========================================================================
//
//  menu.ino  -- UI menu routines used by tzxduino.ino
//
//=========================================================================
//
//  Baud Rate:
//    1200
//    2400
//    3600
//
//  Turbo Boost: Speeds up ZX80/81 TZX/O/P files and Electron UEF files
//    On
//    Off
//
//  Pause @ Start: pause at 10% before sending more data
//    On
//    Off
//
//  Gremlin Loader: oddball loader used by Footballer of the Year
//    On
//    Off
//
//  Hide Dotfiles: Hide files and directories that start with a '.'
//    On
//    Off
//
//=========================================================================

void menuMode() {
  byte menuItem=BAUD_MENU; 
  char* subMenus[6] = {PSTR("Baud"), PSTR("Turbo"), PSTR("Pause @ Start"), PSTR("Gremlin Loader"), PSTR("Hide Dotfiles"), PSTR("Turbo Gap")};
  while(!multiOption(PSTR("Menu Screen"), subMenus, 6, &menuItem, false)) {
    switch(menuItem) {
      case BAUD_MENU: {
          char* baudRates[3] = {PSTR("1200"), PSTR("2400"), PSTR("3600")};
          multiOption(subMenus[0], baudRates, 3, &baudRate, true);
          settings = (settings & ~BAUD_BITS) | baudRate;
          break;
      }
      case TURBO_MENU:
          binaryOption(subMenus[1], TURBO_BIT);
          break;
      case PAUSE_MENU:
          binaryOption(subMenus[2], PAUSE_BIT);
          break;
      case GREMLIN_MENU:
          binaryOption(subMenus[3], GREMLIN_BIT);
          break;
      case HIDE_MENU:
          binaryOption(subMenus[4], HIDE_BIT);
          break;
      case TURBO_BITGAP_MENU:
          numericOption(subMenus[5], &turboBitGap, 10);
          break;
    }
  }
  EEPROM.put(0,settings);
  EEPROM.put(1,turboBitGap);
  printtextF(PSTR(VERSION),0);
}

void navDown(byte* var, byte maxIndex) {
  (*var)++;
  if (*var > maxIndex) *var = 0;
}

void navUp(byte* var, byte maxIndex) {
  if(*var == 0) *var = maxIndex;
  else (*var)--;
}

bool multiOption(char* label, char** choices, int choiceCount, byte* var, bool isLeafMenu) {
  bool updateScreen=true;
  byte choice=0;
  while (STOP_NOT_PRESSED) {
    if (updateScreen) {
      printtextF(label,0);
      printtextF(choices[choice],1);
      if(isLeafMenu && (1 << choice) == *var) sendStr(" *");
      updateScreen=false;
    }
    if (DOWN_PRESSED) {
      while(DOWN_PRESSED) delay(200);
      navDown(&choice, choiceCount - 1);
      updateScreen=true;
    }
    if (UP_PRESSED) {
      while(UP_PRESSED) delay(200);
      navUp(&choice, choiceCount - 1);
      updateScreen=true;
    }
    if (PLAY_PRESSED) {
      while(PLAY_PRESSED) delay(200);
      *var = 1 << choice;
      updateScreen=true;
      if(!isLeafMenu) return false;
    }
  }
  while(STOP_PRESSED) delay(200);
  return true;
}

void binaryOption(char* label, byte mask) {
  bool updateScreen=true;
  while (STOP_NOT_PRESSED) {
    if (updateScreen) {
      printtextF(label,0);
      printtextF(settings & mask ? PSTR("On") : PSTR("Off"),1);
      updateScreen=false;
    }
    if (PLAY_PRESSED) {
      while(PLAY_PRESSED) delay(200);
      settings ^= mask;
      updateScreen=true;
    }
  }
  while(STOP_PRESSED) delay(200);
}

void numericOption(char* label, int* var, byte stride) {
  bool updateScreen=true;
  while (STOP_NOT_PRESSED) {
    if (updateScreen) {
      printtextF(label,0);
      char buff[5];
      snprintf(buff, sizeof(buff), "%d", *var);
      printtext(buff,1);
      updateScreen=false;
    }
    if (UP_PRESSED) {
      while(UP_PRESSED) delay(200);
      *var += stride;
      updateScreen=true;
    }
    if (DOWN_PRESSED) {
      while(DOWN_PRESSED) delay(200);
      *var -= stride;
      updateScreen=true;
    }
  }
  while(STOP_PRESSED) delay(200);
}

//  Setting Byte Definition
//
//  bit 0: 1200               BAUD1200_BIT
//  bit 1: 2400               BAUD2400_BIT
//  bit 2: 3600               BAUD3600_BIT
//  bit 3: NA                 not used
//  bit 4: Hide Dotfiles      HIDE_BIT
//  bit 5: Pause @ Start      PAUSE_BIT
//  bit 6: Gremlin Loader     GREMLIN_BIT
//  bit 7: UEFTurbo           TURBO_BIT

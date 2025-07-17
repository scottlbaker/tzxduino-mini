
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
//=========================================================================

// Main menu selections
#define BAUD_MENU     1
#define BOOST_MENU    2
#define PAUSE_MENU    3
#define GREMLIN_MENU  4

#define FIRST_MENU    BAUD_MENU
#define LAST_MENU     GREMLIN_MENU

// Baud menu selections
#define BAUD_1        1
#define BAUD_2        2
#define BAUD_3        3

#define FIRST_BAUD    BAUD_1
#define LAST_BAUD     BAUD_3

#define ON   0
#define OFF  1

void menuMode() {
  byte lastbtn=true;
  byte menuItem=FIRST_MENU;
  byte subItem=FIRST_BAUD;
  byte updateScreen=true;

  while (STOP_NOT_PRESSED || lastbtn) {
    if (updateScreen) {
      printtextF(PSTR("Menu Screen"),0);
      switch(menuItem) {
        case BAUD_MENU:
          printtextF(PSTR("Baud Rate"),1);
          break;
        case BOOST_MENU:
          printtextF(PSTR("Turbo Boost"),1);
          break;
        case PAUSE_MENU:
          printtextF(PSTR("Pause @ Start"),1);
          break;
        case GREMLIN_MENU:
          printtextF(PSTR("Gremlin Loader"),1);
          break;
        default:
          break;
      }
      updateScreen=false;
    }
    if (DOWN_PRESSED && !lastbtn) {
      menuItem++;
      if (menuItem>LAST_MENU) menuItem=FIRST_MENU;
      lastbtn=true;
      updateScreen=true;
    }
    if (UP_PRESSED && !lastbtn) {
      menuItem--;
      if (menuItem<FIRST_MENU) menuItem=LAST_MENU;
      lastbtn=true;
      updateScreen=true;
    }

    if (PLAY_PRESSED && !lastbtn) {
      switch(menuItem) {
        case BAUD_MENU:
          updateScreen=true;
          lastbtn=true;
          while (STOP_NOT_PRESSED || lastbtn) {
            if (updateScreen) {
              printtextF(PSTR("Baud Rate"),0);
              switch(subItem) {
                case BAUD_1:
                  printtextF(PSTR("1200"),1);
                  if (baudRate==BAUD1200) printStar();
                  break;
                case BAUD_2:
                  printtextF(PSTR("2400"),1);
                  if (baudRate==BAUD2400) printStar();
                  break;
                case BAUD_3:
                  printtextF(PSTR("3600"),1);
                  if (baudRate==BAUD3600) printStar();
                  break;
                default:
                  break;
              }
              updateScreen=false;
            }
            if (DOWN_PRESSED && !lastbtn) {
              subItem++;
              if (subItem>LAST_BAUD) subItem=FIRST_BAUD;
              lastbtn=true;
              updateScreen=true;
            }
            if (UP_PRESSED && !lastbtn) {
              subItem--;
              if (subItem<FIRST_BAUD) subItem=LAST_BAUD;
              lastbtn=true;
              updateScreen=true;
            }
            if (PLAY_PRESSED && !lastbtn) {
              switch(subItem) {
                case BAUD_1:
                  baudRate=BAUD1200;
                break;
                case BAUD_2:
                  baudRate=BAUD2400;
                break;
                case BAUD_3:
                  baudRate=BAUD3600;
                break;
              }
              updateScreen=true;
              lastbtn=true;
            }
            if (NO_BUTTON_PRESSED()) lastbtn=false;
          }
          lastbtn=true;
          updateScreen=true;
          break;

        case BOOST_MENU:
          updateScreen=true;
          lastbtn=true;
          while (STOP_NOT_PRESSED || lastbtn) {
            if (updateScreen) {
              printtextF(PSTR("Turbo Boost"),0);
              printOnOff(uefTurboMode);
              updateScreen=false;
            }
            if (PLAY_PRESSED && !lastbtn) {
              if (uefTurboMode==1) uefTurboMode=0;
              else uefTurboMode=1;
              lastbtn=true;
              updateScreen=true;
            }
            if (NO_BUTTON_PRESSED()) lastbtn=false;
          }
          lastbtn=true;
          updateScreen=true;
          break;

        case PAUSE_MENU:
          updateScreen=true;
          lastbtn=true;
          while (STOP_NOT_PRESSED || lastbtn) {
            if (updateScreen) {
              printtextF(PSTR("Pause @ Start"),0);
              printOnOff(PauseAtStart);
              updateScreen=false;
            }
            if (PLAY_PRESSED && !lastbtn) {
              if (PauseAtStart==1) PauseAtStart=0;
              else PauseAtStart=1;
              lastbtn=true;
              updateScreen=true;
            }
            if (NO_BUTTON_PRESSED()) lastbtn=false;
          }
          lastbtn=true;
          updateScreen=true;
          break;

        case GREMLIN_MENU:
          updateScreen=true;
          lastbtn=true;
          while (STOP_NOT_PRESSED || lastbtn) {
            if (updateScreen) {
              printtextF(PSTR("Gremlin Loader"),0);
              printOnOff(FlipPolarity);
              updateScreen=false;
            }
            if (PLAY_PRESSED && !lastbtn) {
              if (FlipPolarity==1) FlipPolarity=0;
              else FlipPolarity=1;
              lastbtn=true;
              updateScreen=true;
            }
            if (NO_BUTTON_PRESSED()) lastbtn=false;
          }
          lastbtn=true;
          updateScreen=true;
          break;
      }
    }
    if (NO_BUTTON_PRESSED()) lastbtn=false;
  }
  updateEEPROM();
  printtextF(PSTR(VERSION),0);
 }

//  Setting Byte Definition
//
//  bit 0: 1200               BAUD1200_BIT
//  bit 1: 2400               BAUD2400_BIT
//  bit 2: 3600               BAUD3600_BIT
//  bit 3: NA                 not used
//  bit 4: NA                 not used
//  bit 5: Pause @ Start      PAUSE_BIT
//  bit 6: Gremlin Loader     POLAR BIT
//  bit 7: UEFTurbo           TURBO_BIT

#define TURBO_BIT   0x80
#define POLAR_BIT   0x40
#define PAUSE_BIT   0x20

void updateEEPROM() {
  byte settings = baudRate;
  if (uefTurboMode) settings |= TURBO_BIT;
  if (FlipPolarity) settings |= POLAR_BIT;
  if (PauseAtStart) settings |= PAUSE_BIT;
  EEPROM.put(0,settings);
}

void loadEEPROM() {
  byte settings=0;
  EEPROM.get(0,settings);
  if (settings & TURBO_BIT) uefTurboMode = 1;
  else uefTurboMode = 0;
  if (settings & POLAR_BIT) FlipPolarity = 1;
  else FlipPolarity = 0;
  if (settings & PAUSE_BIT) PauseAtStart = 1;
  else PauseAtStart = 0;
  baudRate = settings & 0x07;
}

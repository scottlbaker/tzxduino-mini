
//=========================================================================
//
//  tzxduino.ino  -- main file
//
//=========================================================================

#define VERSION "TZX-Mini 1.17a"
#define DATE    "Jul-23-2025"

// ========================================================================
//
// Load TZX files onto an SD card, and play them directly
//
// The original TZXduino code was written and tested by
// Andrew Beer and Duncan Edwards. It was written using info from
// worldofspectrum.org and TZX2WAV by Francisco Javier Crespo
//
// ========================================================================
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ========================================================================

#include <Wire.h>
#include <EEPROM.h>
#include <SdFat.h>
#include "TZXDuino.h"

// instantiate the file system
SdFat  sd;
SdFile file;

// initialize pins
void init_pins() {
  pinMode(outputPin,  OUTPUT);
  pinMode(chipSelect, OUTPUT);
  digitalWrite(outputPin, LOW);
  pinMode(btnPlay,    INPUT_PULLUP);
  pinMode(btnStop,    INPUT_PULLUP);
  pinMode(btnUp,      INPUT_PULLUP);
  pinMode(btnDown,    INPUT_PULLUP);
  pinMode(btnMotor,   INPUT_PULLUP);
  pinMode(btnRoot,    INPUT_PULLUP);
  pinMode(btnMenu,    INPUT_PULLUP);
}

// init timer1
void init_timer1() {
  TIMSK1 = 0x00;           // disable timer1 interrupt
  TCCR1A = 0x00;           // CTC mode
  TCCR1B = 0x0a;           // prescaler 8
  OCR1A  = 0xfff0;         // load end count
  TCNT1  = 0x00;           // clear timer
}

// initial setup
void setup() {
  init_pins();                      // init pins
  init_timer1();                    // init timer1
  Serial.begin(115200);             // init UART
  Wire.begin();                     // init I2C
  init_OLED();                      // init display
  showLogo();                       // show logo

  // start SD card and check that it's working
  if (!sd.begin(chipSelect,SPI_FULL_SPEED)) {
    printtextF(PSTR("No SD Card"),0);
    return;
  }

  sd.chdir();               // set SD to root directory
  TZXSetup();               // setup TZX specific options

  clearScreen();
  printtextF(PSTR(VERSION),0);
  printtextF(PSTR(DATE),1);
  delay(1000);

  EEPROM.get(0,settings);
  EEPROM.get(1,turboBitGap);
  // check BitGap and default to 500 if not configured yet
  if(turboBitGap < 1 || turboBitGap > ZX81BITGAP) turboBitGap = 500;
  baudRate = settings & BAUD_BITS;
  timeDiff = millis();     // get current millisecond count
  getMaxFile();            // get the total number of files in the directory
  seekFile(currentFile);   // move to the first file in the directory
  if((settings & HIDE_BIT) && (fileName[0] == '.')) downFile();
  updateDisplay();
}

// main loop
void loop(void) {

  if (start==1) {
    TZXLoop();
  } else {
    digitalWrite(outputPin, LOW);    // output LOW when idle
  }

  if ((millis()>=scrollTime) && start==0 && (strlen(fileName)>15)) {
    // filename scrolling only runs if no file is playing to prevent I2C writes
    // conflicting with the playback Interrupt
    scrollTime = millis()+SCROLLSPEED;
    scrollText(fileName);
    scrollPos +=1;
    if (scrollPos>strlen(fileName)) {
      scrollPos=0;
      scrollTime=millis()+SCROLLWAIT;
      scrollText(fileName);
    }
  }
  motorState=digitalRead(btnMotor);
  if (millis() - timeDiff > 50) {   // check switch every 50ms
    timeDiff = millis();            // get current millisecond count

    if (MENU_PRESSED && start==0) {
      menuMode();
      updateDisplay();
    }

    if (PLAY_PRESSED) {
      // handle Play/Pause button
      if (start==0) {
        // if no file is play, start playback
        playFile();
        // showFileSize();  // for debug only
        delay(200);
      } else {
        // if a file is playing, pause or unpause the file
        if (pauseOn == 0) {
          printtextF(PSTR("Paused"),0);
          showPercent();
          showTime();
          pauseOn = 1;
        } else {
          printtextF(PSTR("Playing"),0);
          showPercent();
          pauseOn = 0;
        }
      }
      while (PLAY_PRESSED) {
        // wait until button is released
        delay(200);
      }
    }

    if (STOP_PRESSED && start==1) {
      stopFile();
      // wait until the button is released
      while (STOP_PRESSED) {
        delay(200);
      }
    }
    if (STOP_PRESSED && start==0 && subdir >0) {
      fileName[0]='\0';
      prevSubDir[subdir-1][0]='\0';
      subdir--;
      switch(subdir) {
       case 1:
         // sprintf(fileName,"%s%s",prevSubDir[0],prevSubDir[1]);
         sd.chdir(strcat(strcat(fileName,"/"),prevSubDir[0]),true);
         break;
       case 2:
         // sprintf(fileName,"%s%s/%s",prevSubDir[0],prevSubDir[1],prevSubDir[2]);
         sd.chdir(strcat(strcat(strcat(strcat(fileName,"/"),prevSubDir[0]),"/"),prevSubDir[1]),true);
         break;
      case 3:
         // sprintf(fileName,"%s%s/%s/%s",prevSubDir[0],prevSubDir[1],prevSubDir[2],prevSubDir[3]);
         sd.chdir(strcat(strcat(strcat(strcat(strcat(strcat(fileName,"/"),prevSubDir[0]),"/"),prevSubDir[1]),"/"),prevSubDir[2]),true);
         break;
       default:
         // sprintf(fileName,"%s",prevSubDir[0]);
         sd.chdir("/",true);
       }
       // return to prev Dir of the SD card.
       // sd.chdir(fileName,true);
       // sd.chdir("/CDT");
       // printtext(prevDir,0); // debug back dir
       getMaxFile();
       currentFile=1;
       seekFile(currentFile);
       if((settings & HIDE_BIT) && (fileName[0] == '.')) downFile();
       updateDisplay();
       while (STOP_PRESSED) {
         // prevent button repeats by waiting until the button is released.
         delay(200);
       }
     }

     if (start==0) {
        if (DOWN_PRESSED) {
          // move down a file in the directory
          scrollTime=millis()+SCROLLWAIT;
          scrollPos=0;
          downFile();
          delay(browseDelay);
          reduceBrowseDelay();
        }

       else if (UP_PRESSED) {
         // move up a file in the directory
         scrollTime=millis()+SCROLLWAIT;
         scrollPos=0;
         upFile();
         delay(browseDelay);
         reduceBrowseDelay();
       }
       else {
        resetBrowseDelay();
       }
     }

     if (start==1 && (!oldMotorState==motorState)) {
       // motor control works by pulling the btnMotor to GND to play
       if (motorState==1 && pauseOn==0) {
         printtextF(PSTR("Paused"),0);
         showPercent();
         showTime();
         pauseOn = 1;
       }
       if (motorState==0 && pauseOn==1) {
         printtextF(PSTR("Playing"),0);
         showPercent();
         pauseOn = 0;
       }
       oldMotorState=motorState;
     }
  }
}

byte NO_BUTTON_PRESSED() {
  byte buttons = PINC;
  byte mask = 0x0F;
  delay(10);
  if ((buttons & mask) == mask) return 1;
  else return 0;
}

void reduceBrowseDelay() {
  if (browseDelay >= 100) {
    browseDelay -= 100;
  }
}

void resetBrowseDelay() {
  browseDelay = 500;
}

void upFile() {
  do {
    // move up a file in the directory
    currentFile--;
    if (currentFile<1) {
      getMaxFile();
      currentFile = maxFile;
    }
    UP=1;
    seekFile(currentFile);
  } while((settings & HIDE_BIT) && (fileName[0] == '.'));
  updateDisplay();
}

void downFile() {
  do {
    // move down a file in the directory
    currentFile++;
    if (currentFile>maxFile) { currentFile=1; }
    UP=0;
    seekFile(currentFile);
  } while((settings & HIDE_BIT) && (fileName[0] == '.'));
  updateDisplay();
}

void seekFile(int pos) {
  // move to a set position in the directory
  // store the filename, and display the name on screen
  if (UP==1) {
    file.cwd()->rewind();
    for (int i=1;i<=currentFile-1;i++) {
      file.openNext(file.cwd(),O_READ);
      file.close();
    }
  }
  if (currentFile==1) {file.cwd()->rewind();}
  file.openNext(file.cwd(),O_READ);
  file.getName(fileName,FILENAMELENGTH);
  file.getSFN(sfileName);
  fileSize = file.fileSize();
  ayblklen = fileSize + 3;  //  add 3 file header, data byte and chksum byte to file length
  if (file.isDir() || !strcmp(sfileName, "ROOT")) { isDir=1; } else { isDir=0; }
  file.close();
}

void updateDisplay() {
  if (isDir==1) {
    printtextF(PSTR(VERSION),0);
  } else {
    printtextF(PSTR("Select File.."),0);
  }
  scrollPos=0;
  scrollText(fileName);
}

void stopFile() {
  TZXStop();
  if (start==1) {
    printtextF(PSTR("Stopped"),0);
    showPercent();
    showTime();
    clearLine(2);
    start=0;
  }
}

void playFile() {
  if (isDir==1) {
    // if selected file is a directory move into directory
    changeDir();
  } else {
    if (file.cwd()->exists(sfileName)) {
      printtextF(PSTR("Playing"),0);
      scrollPos=0;
      if (!(settings & PAUSE_BIT)) pauseOn = 0;
      scrollText(fileName);
      currpct=0;
      timeCount=0;
      timeDiff2 = millis();         // reset millisecond count
      TZXPlay(sfileName);           // load using the short filename
      start=1;
      if (settings & PAUSE_BIT) {
        printtextF(PSTR("Paused"),0);
        pauseOn = 1;
        TZXPause();
      }
    } else {
      printtextF(PSTR("No File Selected"),1);
    }
  }
}

void getMaxFile() {
  // gets the total files in the current directory and stores the number in maxFile
  file.cwd()->rewind();
  maxFile=0;
  while (file.openNext(file.cwd(),O_READ)) {
    // file.getName(fileName,FILENAMELENGTH);
    file.close();
    maxFile++;
  }
  file.cwd()->rewind();
}

void changeDir() {
  // change directory, if fileName="ROOT" then return to the root directory
  // sdfat has no easy way to move up a directory, so returning to root is the easiest way.
  // each directory (except the root) must have a file called ROOT (no extension)
  if (!strcmp(fileName, "ROOT")) {
    subdir=0;
    sd.chdir(true);
  } else {
     if (subdir >0) file.cwd()->getName(prevSubDir[subdir-1],FILENAMELENGTH);
     sd.chdir(fileName, true);
     subdir++;
  }
  getMaxFile();
  currentFile=1;
  seekFile(currentFile);
  if((settings & HIDE_BIT) && (fileName[0] == '.')) downFile();
  updateDisplay();
}

void scrollText(char* text) {
  // text scrolling routine. Setup for 16x2 screen so will only display 16 chars
  if (scrollPos<0) scrollPos=0;
  char outtext[17];
  if (isDir) { outtext[0]= 0x3E;
    for (int i=1;i<16;i++) {
      int p=i+scrollPos-1;
      if (p<strlen(text)) {
        outtext[i]=text[p];
      } else {
        outtext[i]='\0';
      }
    }
  } else {
    for (int i=0;i<16;i++) {
      int p=i+scrollPos;
      if (p<strlen(text)) {
        outtext[i]=text[p];
      } else {
        outtext[i]='\0';
      }
    }
  }
  outtext[16]='\0';
  printtext(outtext,1);
}


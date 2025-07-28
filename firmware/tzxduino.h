
//=========================================================================
//
//  tzxduino.h  -- include file used by tzxduino.ino
//
//=========================================================================

// Arduino pin definitions
#define btnPlay           17    // PC3 - play button
#define btnStop           16    // PC2 - stop button
#define btnUp             15    // PC1 - up   button
#define btnDown           14    // PC0 - down button
#define chipSelect        10    // PB2 - SD card chip select
#define outputPin          9    // PB1 - audio output pin
#define btnMenu            8    // PB0 - menu button
#define btnRoot            7    // PD7 - return to SD card root (NOT USED )
#define btnMotor           6    // PD6 - motor Sense (GND to play, NC for pause)

// user pushbutton macros
#define PLAY_PRESSED     (!digitalRead(btnPlay))
#define STOP_PRESSED     (!digitalRead(btnStop))
#define UP_PRESSED       (!digitalRead(btnUp))
#define DOWN_PRESSED     (!digitalRead(btnDown))
#define MENU_PRESSED     (!digitalRead(btnMenu))
#define STOP_NOT_PRESSED (digitalRead(btnStop))

#define OLED_address     0x3C       // OLED I2C address

#define FILENAMELENGTH    100       // maximum length for scrolling filename
#define SCROLLSPEED       250       // text scroll delay
#define SCROLLWAIT       3000       // delay before scrolling starts

#define LowWrite(x,y) (x&=(~(1<<y)))
#define HighWrite(x,y) (x|=(1<<y))

// TZX block list
#define ID10                0x10    // standard speed data block
#define ID11                0x11    // turbo speed data block
#define ID12                0x12    // pure tone
#define ID13                0x13    // sequence of pulses of various lengths
#define ID14                0x14    // pure data block
#define ID15                0x15    // direct recording block
#define ID18                0x18    // csw recording block (NOT SUPPORTED)
#define ID19                0x19    // generalized data block
#define ID20                0x20    // pause block
#define ID21                0x21    // group start
#define ID22                0x22    // group end
#define ID23                0x23    // jump to block
#define ID24                0x24    // loop start
#define ID25                0x25    // loop end
#define ID26                0x26    // call sequence
#define ID27                0x27    // return from sequence
#define ID28                0x28    // select block
#define ID2A                0x2A    // stop the tape is in 48K mode
#define ID2B                0x2B    // set signal level
#define ID30                0x30    // text description
#define ID31                0x31    // message block
#define ID32                0x32    // archive info
#define ID33                0x33    // hardware type
#define ID35                0x35    // custom info block
#define ID4B                0x4B    // kansas City block (MSX/BBC/Acorn/...)
#define IDPAUSE             0x59    // custom Pause processing
#define ID5A                0x5A    // glue block (90 dec, ASCII Letter 'Z')
#define ORIC                0xFA    // oric Tap File
#define AYO                 0xFB    // ay file
#define ZXO                 0xFC    // zx80 O file
#define ZXP                 0xFD    // zx81 P File
#define TAP                 0xFE    // tap File Mode
#define EOF                 0xFF    // end of file
#define UEF                 0xFA    // uef file

//  UEF chunks
#define ID0000              0x0000
#define ID0100              0x0100
#define ID0104              0x0104  // defined tape format data block: data bits per packet/parity/stop bits
#define ID0110              0x0110
#define ID0111              0x0111  // carrier tone (previously high tone) with dummy byte at byte
#define ID0112              0x0112
#define ID0114              0x0114  // security Cycles replaced with carrier tone
#define ID0116              0x0116  // floating point gap: cycles = floatGap * this.baud
#define ID0117              0x0117  // data encoding format change for 300 bauds
#define IDCHUNKEOF          0xffff

// TZX File Tasks
#define GETFILEHEADER         0
#define GETID                 1
#define PROCESSID             2
#define GETAYHEADER           3
#define GETUEFHEADER          4
#define GETCHUNKID            5
#define PROCESSCHUNKID        6

// TZX ID Tasks
#define READPARAM             0
#define PILOT                 1
#define SYNC1                 2
#define SYNC2                 3
#define DATA                  4
#define PAUSE                 5
#define HEADER                6
#define NAME                  7
#define GAP                   8
#define SYNCLAST              9
#define NAMELAST              10

// Buffer size
#define buffsize              64

// Main menu IDs
#define BAUD_MENU             1
#define TURBO_MENU            2
#define PAUSE_MENU            4
#define GREMLIN_MENU          8
#define HIDE_MENU             16
#define TURBO_BITGAP_MENU     32

// Baud rate IDs
#define BAUD1200              1
#define BAUD2400              2
#define BAUD3600              4

// Settings bits
#define TURBO_BIT             0x80
#define GREMLIN_BIT           0x40
#define PAUSE_BIT             0x20
#define HIDE_BIT              0x10
#define BAUD_BITS             0x07

// Spectrum Standards
#define PILOTLENGTH           619
#define SYNCFIRST             191
#define SYNCSECOND            210
#define ZEROPULSE             244
#define ONEPULSE              489
#define PILOTNUMBERL          8063
#define PILOTNUMBERH          3223
#define PAUSELENGTH           1000

// ZX81 pulse patterns
// Zero Bit   1 0 1 0 1 0 1 GAP
// One Bit    1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 GAP

// ZX81 zero/one timing
// For 1200 baud zero is 416us, one is 208us
// For 1500 baud zero is 333us, one is 166us
// For 1550 baud zero is 322us, one is 161us
// For 1600 baud zero is 313us, one is 156us

// TODO: turbo mode experimentation and testing
// faster baud rates are probably possible

// ZX81 non-turbo mode
#define ZX81PULSE               160
#define ZX81BITGAP              1442
#define ZX81PILOTPULSES         outWord<<2;
#define ZX81PILOTLENGTH         208
#define ZX81ZEROPULSE           416
#define ZX81ONEPULSE            208

// ZX81 turbo-mode
#define TURBOPULSE              128
#define TURBOBITGAP             600
#define TURBOPILOTPULSES        320
#define TURBOPILOTLENGTH        166
#define TURBOZEROPULSE          333
#define TURBOONEPULSE           166

// ORIC parameters
#define ORICZEROPULSE           416
#define ORICZEROLOWPULSE        208
#define ORICZEROHIGHPULSE       416
#define ORICONEPULSE            208
#define ORICONELOWPULSE         208
#define ORICONEHIGHPULSE        208

//  AY Header offset start
#define HDRSTART                 0

// constant strings
PROGMEM const char TZXTape[7] = {'Z','X','T','a','p','e','!'};
PROGMEM const char TAPcheck[7] = {'T','A','P','t','a','p','.'};
PROGMEM const char ZX81Filename[9] = {'T','Z','X','D','U','I','N','O',0x9D};
PROGMEM const char AYFile[8] = {'Z','X','A','Y','E','M','U','L'};
PROGMEM const char TAPHdr[20] = {0,0,3,'Z','X','A','Y','F','i','l','e',' ',' ',0x1A,0xB,0x0,0xC0,0x0,0x80,0x6E};
PROGMEM const char UEFFile[9] = {'U','E','F',' ','F','i','l','e','!'};

// ISR variables
volatile byte pos = 0;
volatile word wbuffer[buffsize+1][2];
volatile byte morebuff = HIGH;
volatile byte workingBuffer=0;
volatile byte isStopped=false;
volatile byte pinState=LOW;
volatile byte isPauseBlock = false;
volatile byte wasPauseBlock = false;
volatile byte intError = false;

volatile byte currentBit=0;
volatile byte currentByte=0;
volatile byte currentChar=0;

// global variables
byte settings = BAUD1200;
word turboBitGap = TURBOBITGAP;
byte currentID = 0;
byte lastID = 99;
byte currentTask = 0;
byte currentBlockTask = 0;
word currentPeriod=1;
byte AYpass = 0;
byte hdrptr = 0;
byte blkchksum = 0;
word ayblklen = 0;
byte btemppos = 0;
byte copybuff = LOW;
byte pulsesCountByte=0;
word pilotPulses=0;
word pilotLength=0;
word sync1Length=0;
word sync2Length=0;
word zeroPulse=0;
word onePulse=0;
word TstatesperSample=0;
byte usedBitsInLastByte=8;
word loopCount=0;
byte seqPulses=0;
byte inputBuf[11];
word pauseLength=0;
word temppause=0;
byte bufCount=128;
byte pass=0;
byte EndOfFile=false;
byte lastByte;
byte currpct = 0;
byte baudRate = BAUD1200;
word chunkID = 0;
byte UEFPASS = 0;
byte passforZero=2;
byte passforOne=4;

byte ID15switch    = 0;
byte parity        = 0;  // 0:NoParity 1:ParityOdd 2:ParityEven (default:0)
byte bitChecksum   = 0;  // 0:Even 1:Odd number of one bits

byte  outByte=0;
word  outWord=0;
float outFloat;

char indicators[] = {'|', '/', '-',92};

char fileName[FILENAMELENGTH + 1];  // current filename
char sfileName[13];                 // short filename variable
char prevSubDir[3][25];
int  subdir = 0;
byte scrollPos = 0;                 // stores scrolling text position
byte mselectState = 0;              // motor control state 1=on 0=off
byte motorState = 1;                // current motor control state
byte oldMotorState = 1;             // last motor control state
byte start = 0;                     // currently playing flag
byte pauseOn = 0;                   // pause state
int  currentFile = 1;               // current position in directory
int  maxFile = 0;                   // total number of files in directory
byte isDir = 0;                     // is the current file a directory
int  browseDelay = 500;             // delay between up/down navigation
byte UP = 0;                        // next File, down button pressed

unsigned long bytesRead=0;          // bytes read
unsigned long bytesToRead=0;        // bytes to read
unsigned long fileSize;             // file size in bytes
unsigned long loopStart=0;
unsigned long outLong=0;
unsigned long timeDiff;             // used by button debounce
unsigned long timeDiff2;            // used by second counter
unsigned int  timeCount;            // second counter
unsigned long scrollTime;           // time delay before scrolling


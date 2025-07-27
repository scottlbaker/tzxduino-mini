
//=========================================================================
//
//  processing.ino  -- data processing routines used by tzxduino.ino
//
//=========================================================================

int readfile (byte bytes, unsigned long p) {
  int i = 0;
  int t = 0;
  if (file.seekSet (p)) {
    i = file.read (inputBuf, bytes);
  }
  return i;
}

void clearBuffer () {
  for (int i = 0; i <= buffsize; i++) {
    wbuffer[i][0] = 0;
    wbuffer[i][1] = 0;
  }
}

void printID() {
  byte x;
  switch (currentID) {
  case EOF:
    printtextF (PSTR ("EOF"), 2);
    break;
  default:
    printtextF (PSTR ("ID"), 2);
    setXY (2, 2);
    x = (currentID>>4);
    if (x < 10) sendChar (x+48);
    else sendChar (x+87);
    x = currentID & 0x0f;
    if (x < 10) sendChar (x+48);
    else sendChar (x+87);
    break;
  }
}

bool checkForTap (char *filename) {
  // Check for TAP file extensions as these have no header
  byte len = strlen(filename);
  if (strstr_P (strlwr(filename + (len - 4)), PSTR (".tap"))) {
    return true;
  }
  return false;
}

bool checkForP (char *filename) {
  // Check for P file extensions as these have no header
  byte len = strlen(filename);
  if (strstr_P (strlwr(filename + (len - 2)), PSTR (".p"))) {
    return true;
  }
  return false;
}

bool checkForO (char *filename) {
  // Check for O file extensions as these have no header
  byte len = strlen(filename);
  if (strstr_P (strlwr(filename + (len - 2)), PSTR (".o"))) {
    return true;
  }
  return false;
}

bool checkForAY (char *filename) {
  // check for AY File (need to create Tap header)
  byte len = strlen(filename);
  if (strstr_P (strlwr(filename + (len - 3)), PSTR (".ay"))) {
    return true;
  }
  return false;
}

bool checkForUEF (char *filename) {
  // check for UEF File (need to create Tap header)
  byte len = strlen(filename);
  if (strstr_P (strlwr(filename + (len - 4)), PSTR (".uef"))) {
    return true;
  }
  return false;
}

void UEFCarrierToneBlock() {
  // pure Tone Block - Long string of pulses with the same length
  currentPeriod = pilotLength;
  pilotPulses -= 1;
  if (pilotPulses==0) {
    currentTask = GETCHUNKID;
  }
}

void writeUEFData() {
  // convert byte from file into string of pulses.. one pulse per pass
  byte r;
  if (currentBit==0) {                         // check for byte end/first byte
    if (r=ReadByte(bytesRead)==1) {            // read in a byte
      currentByte = outByte;
      bytesToRead -= 1;
      bitChecksum = 0;
      if (bytesToRead == 0) {                  // check for end of data block
        lastByte = 1;
        if (pauseLength==0) {                  // search for next ID if there is no pause
          currentTask = PROCESSCHUNKID;
        } else {
          currentBlockTask = PAUSE;            // otherwise start the pause
        }
        // return;                             // exit
      }
    } else if (r==0) {                         // if we reached the EOF
      currentTask = GETCHUNKID;
    }
    currentBit = 11;
    pass=0;
  }
  if ((currentBit == 2) && (parity == 0)) currentBit = 1;
  if (currentBit == 11) {
    currentPeriod = zeroPulse;
  } else if (currentBit == 2) {
    currentPeriod = (bitChecksum ^ (parity & 0x01)) ? onePulse : zeroPulse;
  } else if (currentBit == 1) {
      currentPeriod = onePulse;
  } else {
    if (currentByte&0x01) {                       // set next period depending on value of bit 0
      currentPeriod = onePulse;
    } else {
      currentPeriod = zeroPulse;
    }
  }
  pass+=1;      // data is played as 2 x pulses for a zero, and 4 pulses for a one when speed is 1200
  if (currentPeriod == zeroPulse) {
    if (pass==passforZero) {
       if ((currentBit>1) && (currentBit<11)) {
         currentByte >>= 1;                        // shift along to the next bit
       }
       currentBit -= 1;
       pass=0;
       if ((lastByte) && (currentBit==0)) {
         currentTask = GETCHUNKID;
       }
    }
  } else {
    // must be a one pulse
    if (pass==passforOne) {
      if ((currentBit>1) && (currentBit<11)) {
        bitChecksum ^= 1;
        currentByte >>= 1;                        // shift along to the next bit
      }
      currentBit -= 1;
      pass=0;
      if ((lastByte) && (currentBit==0)) {
        currentTask = GETCHUNKID;
      }
    }
  }
}

void OricDataBlock () {
  // convert byte from file into string of pulses
  byte r;
  if (currentBit == 0) {                    // check for byte end/first byte
    if (r = ReadByte(bytesRead) == 1) {     // read in a byte
      currentByte = outByte;
      bytesToRead -= 1;
      bitChecksum = 0;
      if (bytesToRead == 0) {               // check for end of data block
        lastByte = 1;
      }
    } else if (r == 0) {                    // if we reached the EOF
      EndOfFile = true;
      temppause = 0;
      bufCount = 255;
      currentID = IDPAUSE;
      return;
    }
    currentBit = 11;
    pass = 0;
  }
  OricBitWrite();
}

void OricBitWrite () {
  if (currentBit == 11) {   //Start Bit
    if (pass == 0) currentPeriod = ORICZEROLOWPULSE;
    if (pass == 1) currentPeriod = ORICZEROHIGHPULSE;
  } else if (currentBit == 2) {   // inverse parity
    if (pass == 0) currentPeriod = bitChecksum ? ORICZEROLOWPULSE : ORICONEPULSE;
    if (pass == 1) currentPeriod = bitChecksum ? ORICZEROHIGHPULSE : ORICONEPULSE;
  } else if (currentBit == 1) {
    currentPeriod = ORICONEPULSE;
  } else {
    if (currentByte & 0x01) {                      // set next period depending on value of bit 0
      currentPeriod = ORICONEPULSE;
    } else {
      if (pass == 0) currentPeriod = ORICZEROLOWPULSE;
      if (pass == 1) currentPeriod = ORICZEROHIGHPULSE;
    }
  }
  pass += 1;
  if (currentPeriod == ORICONEPULSE) {
    if ((currentBit > 2) && (currentBit < 11) && (pass == 2)) {
      bitChecksum ^= 1;
      currentByte >>= 1;                           // shift along to the next bit
      currentBit -= 1;
      pass = 0;
    }
    if ((currentBit == 1) && (pass == 6)) {
      currentBit -= 1;
      pass = 0;
    }
    if (((currentBit == 2) || (currentBit == 11)) && (pass == 2)) {
      currentBit -= 1;
      pass = 0;
    }
    if ((currentBit == 0) && (lastByte)) {
      bufCount = 255;
      currentBlockTask = PAUSE;
    }
  } else {
    // must be a zero pulse
    if (pass == 2) {
      if ((currentBit > 2) && (currentBit < 11)) {
        currentByte >>= 1;
      }
      currentBit -= 1;
      pass = 0;
      if ((currentBit == 0) && (lastByte)) {
        bufCount = 255;
        currentBlockTask = PAUSE;
      }
    }
  }
}

word TickToUs (word ticks) {
  return (word)((((float)ticks) / 3.5) + 0.5);
}

void checkForEXT (char *filename) {
  if (checkForTap(filename)) {                 // check for Tap file
    currentTask = PROCESSID;
    currentID = TAP;
    if ((readfile (1, bytesRead)) == 1) {
      if (inputBuf[0] == 0x16) {
        currentID = ORIC;
      }
    }
    // printtextF(PSTR("TAP Playing"),0);
  }
  if (checkForP(filename)) {                   // check for P File
    currentTask = PROCESSID;
    currentID = ZXP;
    // printtextF(PSTR("ZX81 P Playing"),0);
  }
  if (checkForO(filename)) {                   // check for O File
    currentTask = PROCESSID;
    currentID = ZXO;
    // printtextF(PSTR("ZX80 O Playing"),0);
  }
  if (checkForAY(filename)) {                  // check for AY File
    currentTask = GETAYHEADER;
    currentID = AYO;
    AYpass = 0;
    hdrptr = HDRSTART;
    // printtextF(PSTR("AY Playing"),0);
  }
  if (checkForUEF(filename)) {                // check for UEF File
    currentTask = GETUEFHEADER;
    currentID = UEF;
    // printtextF(PSTR("UEF Playing"),0);
  }
}

void TZXPlay (char *filename) {
  Timer1.stop();                               // stop timer interrupt
  if (!file.open (filename, O_READ)) {         // open file and check for errors
    printtextF (PSTR ("Error Opening File"), 0);
  }
  bytesRead = 0;                                // start of file
  currentTask = GETFILEHEADER;                  // first task: search for header
  checkForEXT(filename);
  currentBlockTask = READPARAM;                 // first block task is to read in parameters
  clearBuffer();
  isStopped = false;
  pinState = LOW;                               // always Start on a LOW output for simplicity
  bufCount = 255;                               // end of file buffer flush
  EndOfFile = false;
  if (pinState == LOW) LowWrite(PORTB, PORTB1);
  else HighWrite(PORTB, PORTB1);
  Timer1.setPeriod (1000);                      // set 1ms wait at start of a file.
}

void TZXStop () {
  Timer1.stop ();
  isStopped = true;
  file.close ();
  bytesRead = 0;
  blkchksum = 0;
  AYpass = 0;
  ID15switch = 0;
}

void TZXPause () {
  isStopped = pauseOn;
}

void TZXLoop () {
  noInterrupts();                  // pause interrupts to prevent var reads and copy values out
  copybuff = morebuff;
  morebuff = LOW;
  isStopped = pauseOn;
  interrupts();
  if (copybuff == HIGH) {
    btemppos = 0;                  // buffer has swapped, start from the beginning of the new page
    copybuff = LOW;
  }
  if (btemppos <= buffsize) {      // keep filling until full
    TZXProcess();                  // generate the next period to add to the buffer
    if (currentPeriod > 0) {
      noInterrupts();              // pause interrupts while we add a period to the buffer
      wbuffer[btemppos][workingBuffer ^ 1] = currentPeriod;
      interrupts();
      btemppos += 1;
    }
  } else {
    if (currpct == 100) {
      showPercent();
      currpct=111;
    }
    if (currpct < 100) {
      if (bytesRead == fileSize) currpct=100;
      else currpct = (100 * bytesRead) / fileSize;
      showPercent();
      // showBytesRead();  // for debug only
    }
  }
  if (pauseOn == 0) updateTime();
}

void TZXSetup () {
  scrollTime = millis() + SCROLLWAIT;
  isStopped = true;
  pinState = LOW;
  Timer1.initialize(100000);        // timer1 100ms pause
  Timer1.attachInterrupt(wave);
  Timer1.stop();                    // stop timer1
}

void ReadTZXHeader () {
  // read and check first 10 bytes for a TZX header
  char tzxHeader[11];
  int i = 0;
  if (file.seekSet (0)) {
    i = file.read (tzxHeader, 10);
    if (memcmp_P (tzxHeader, TZXTape, 7) != 0) {
      printtextF (PSTR ("Not TZXTape"), 1);
      TZXStop();
    }
  } else {
    printtextF (PSTR ("Error Reading File"), 0);
  }
  bytesRead = 10;
}

void ReadAYHeader () {
  // read and check first 8 bytes for a TZX header
  char ayHeader[9];
  int i = 0;
  if (file.seekSet (0)) {
    i = file.read (ayHeader, 8);
    if (memcmp_P (ayHeader, AYFile, 8) != 0) {
      printtextF (PSTR ("Not AY File"), 1);
      TZXStop();
    }
  } else {
    printtextF (PSTR ("Error Reading File"), 0);
  }
  bytesRead = 0;
}

void ReadUEFHeader() {
  // read and check first 12 bytes for a UEF header
  char uefHeader[9];
  int i=0;
  if (file.seekSet(0)) {
    i = file.read(uefHeader,9);
    if (memcmp_P(uefHeader,UEFFile,9)!=0) {
      printtextF(PSTR("Not UEF File"),1);
      TZXStop();
    }
  } else {
    printtextF(PSTR("Error Reading File"),0);
  }
  bytesRead =12;
}

int ReadByte (unsigned long pos) {
  // read a byte from the file, and move file position on one if successful
  byte out[1];
  int i = 0;
  if (file.seekSet (pos)) {
    i = file.read (out, 1);
    if (i == 1) bytesRead += 1;
  }
  outByte = out[0];
  // blkchksum = blkchksum ^ out[0];
  return i;
}

int ReadWord (unsigned long pos) {
  // read 2 bytes from the file, and move file position on two if successful
  byte out[2];
  int i = 0;
  if (file.seekSet (pos)) {
    i = file.read (out, 2);
    if (i == 2) bytesRead += 2;
  }
  outWord = word (out[1], out[0]);
  // blkchksum = blkchksum ^ out[0] ^ out[1];
  return i;
}

int ReadLong (unsigned long pos) {
  // read 3 bytes from the file, and move file position on three if successful
  byte out[3];
  int i = 0;
  if (file.seekSet (pos)) {
    i = file.read (out, 3);
    if (i == 3) bytesRead += 3;
  }
  outLong = ((unsigned long)word (out[2], out[1]) << 8) | out[0];
  // outLong = (word(out[2],out[1]) << 8) | out[0];
  // blkchksum = blkchksum ^ out[0] ^ out[1] ^ out[2];
  return i;
}

int ReadDword (unsigned long pos) {
  // read 4 bytes from the file, and move file position on four if successful
  byte out[4];
  int i = 0;
  if (file.seekSet (pos)) {
    i = file.read (out, 4);
    if (i == 4) bytesRead += 4;
  }
  outLong = ((unsigned long)word (out[3], out[2]) << 16) | word (out[1], out[0]);
  return i;
}

void TZXProcess () {
  char tmp[16];
  byte r = 0;
  currentPeriod = 0;
  if (currentTask == GETFILEHEADER) {
    // grab 7 byte string
    ReadTZXHeader();
    // set current task to GETID
    currentTask = GETID;
  }
  if (currentTask == GETAYHEADER) {
    // grab 8 byte string
    ReadAYHeader();
    // set current task to PROCESSID
    currentTask = PROCESSID;
  }
  if (currentTask == GETUEFHEADER) {
    // grab 12 byte string
    ReadUEFHeader();
    // set current task to GETCHUNKID
    currentTask = GETCHUNKID;
  }
  if (currentTask == GETCHUNKID) {
    if (r = ReadWord(bytesRead) == 2) {
      chunkID = outWord;
      if (r = ReadDword (bytesRead) == 4) {
        bytesToRead = outLong;
        parity = 0;
        if (chunkID == ID0104) {
          bytesToRead -= 3;
          bytesRead += 1;
          if (ReadByte(bytesRead) == 1) {
            if (outByte == 'O') parity = 1;
            else if (outByte == 'E') parity = 2;
            else parity = 0;
          }
          bytesRead += 1;
        }
      } else {
        chunkID = IDCHUNKEOF;
      }
    } else {
      chunkID = IDCHUNKEOF;
    }
    if (TurboMode) {
      zeroPulse = TURBOZEROPULSE;
      onePulse  = TURBOONEPULSE;
    } else {
      zeroPulse = ZX81ZEROPULSE;
      onePulse  = ZX81ONEPULSE;
    }
    lastByte = 0;

    // reset data block values
    currentBit = 0;
    pass = 0;
    // set current task to PROCESSCHUNKID
    currentTask = PROCESSCHUNKID;
    currentBlockTask = READPARAM;
    UEFPASS = 0;
  }
  if (currentTask == PROCESSCHUNKID) {

    switch (chunkID) {
    case ID0000:
      bytesRead += bytesToRead;
      currentTask = GETCHUNKID;
      break;

    case ID0100:
      writeUEFData();
      break;

    case ID0104:
      writeUEFData();
      break;

    case ID0110:
      if (currentBlockTask == READPARAM) {
        if (r = ReadWord(bytesRead) == 2) {
          if (TurboMode) {
            pilotPulses = TURBOPILOTPULSES;
            pilotLength = TURBOPILOTLENGTH;
          } else {
            pilotPulses = ZX81PILOTPULSES;
            pilotLength = ZX81PILOTLENGTH;
          }
        }
        currentBlockTask = PILOT;
      } else {
        UEFCarrierToneBlock();
      }
      break;

    case ID0111:
      if (currentBlockTask == READPARAM) {
        if (r = ReadWord(bytesRead) == 2) {
          pilotPulses = ZX81PILOTPULSES;
          pilotLength = ZX81PILOTLENGTH;
        }
        currentBlockTask = PILOT;
        UEFPASS += 1;
      } else if (UEFPASS == 1) {
        UEFCarrierToneBlock();
        if (pilotPulses == 0) {
          currentTask = PROCESSCHUNKID;
          currentByte = 0xAA;
          lastByte = 1;
          currentBit = 10;
          pass = 0;
          UEFPASS = 2;
        }
      } else if (UEFPASS == 2) {
        parity = 0;
        writeUEFData();
        if (currentBit == 0) {
          currentTask = PROCESSCHUNKID;
          currentBlockTask = READPARAM;
        }
      } else if (UEFPASS == 3) {
        UEFCarrierToneBlock();
      }
      break;

    case ID0112:
      if (r = ReadWord(bytesRead) == 2) {
        if (outWord > 0) {
          temppause = outWord;
          currentID = IDPAUSE;
          currentPeriod = temppause;
          bitSet (currentPeriod, 15);
          currentTask = GETCHUNKID;
        } else {
          currentTask = GETCHUNKID;
        }
      }
      break;

    case ID0114:
      if (r = ReadWord(bytesRead) == 2) {
        pilotPulses = ZX81PILOTPULSES;
        bytesRead -= 2;
      }
      UEFCarrierToneBlock();
      bytesRead += bytesToRead;
      currentTask = GETCHUNKID;
      break;

    case ID0116:
      if (r = ReadDword (bytesRead) == 4) {
        byte *FloatB = (byte *)&outLong;
        outWord = (((*(FloatB + 2) & 0x80) >> 7) | (*(FloatB + 3) & 0x7f) << 1) + 10;
        outWord = *FloatB | (*(FloatB + 1)) << 8 | ((outWord & 1) << 7) << 16 | (outWord >> 1) << 24;
        outFloat = *((float *)&outWord);
        outWord = (int)outFloat;

        if (outWord > 0) {
          temppause = outWord;
          currentID = IDPAUSE;
          currentPeriod = temppause;
          bitSet (currentPeriod, 15);
          currentTask = GETCHUNKID;
        } else {
          currentTask = GETCHUNKID;
        }
      }
      break;

    case ID0117:
      if (r = ReadWord(bytesRead) == 2) {
        if (outWord == 300) {
          passforZero = 8;
          passforOne = 16;
          currentTask = GETCHUNKID;
        } else {
          passforZero = 2;
          passforOne = 4;
          currentTask = GETCHUNKID;
        }
      }
      break;

    case IDCHUNKEOF:
      bytesRead += bytesToRead;
      stopFile();
      return;

    default:
      bytesRead += bytesToRead;
      currentTask = GETCHUNKID;
      break;
    }
  }

  if (currentTask == GETID) {
    // grab 1 byte ID
    if (ReadByte(bytesRead) == 1) {
      currentID = outByte;
    } else {
      currentID = EOF;
    }
    // reset data block values
    currentBit = 0;
    pass = 0;
    // set current task to PROCESSID
    currentTask = PROCESSID;
    currentBlockTask = READPARAM;
  }
  if (currentTask == PROCESSID) {
    if (currentID != lastID) {
      printID();
      lastID = currentID;
    }
    switch (currentID) {

    case ID10:
      // process ID10 - Standard Block
      switch (currentBlockTask) {
      case READPARAM:
        if (r = ReadWord(bytesRead) == 2) {
          pauseLength = outWord;
        }
        if (r = ReadWord(bytesRead) == 2) {
          bytesToRead = outWord + 1;
        }
        if (r = ReadByte(bytesRead) == 1) {
          if (outByte == 0) {
            pilotPulses = PILOTNUMBERL;
          } else {
            pilotPulses = PILOTNUMBERH;
          }
          bytesRead -= 1;
        }
        pilotLength = PILOTLENGTH;
        sync1Length = SYNCFIRST;
        sync2Length = SYNCSECOND;
        zeroPulse = ZEROPULSE;
        onePulse = ONEPULSE;
        currentBlockTask = PILOT;
        usedBitsInLastByte = 8;
        break;

      default:
        StandardBlock();
        break;
      }
      break;

    case ID11:
      // process ID11 - Turbo Tape Block
      switch (currentBlockTask) {
      case READPARAM:
        if (r = ReadWord(bytesRead) == 2) {
          pilotLength = TickToUs (outWord);
        }
        if (r = ReadWord(bytesRead) == 2) {
          sync1Length = TickToUs (outWord);
        }
        if (r = ReadWord(bytesRead) == 2) {
          sync2Length = TickToUs (outWord);
        }
        if (r = ReadWord(bytesRead) == 2) {
          zeroPulse = TickToUs (outWord);
        }
        if (r = ReadWord(bytesRead) == 2) {
          onePulse = TickToUs (outWord);
        }
        if (r = ReadWord(bytesRead) == 2) {
          pilotPulses = outWord;
        }
        if (r = ReadByte(bytesRead) == 1) {
          usedBitsInLastByte = outByte;
        }
        if (r = ReadWord(bytesRead) == 2) {
          pauseLength = outWord;
        }
        if (r = ReadLong (bytesRead) == 3) {
          bytesToRead = outLong + 1;
        }
        currentBlockTask = PILOT;
        break;

      default:
        StandardBlock();
        break;
      }
      break;

    case ID12:
      // process ID12 - Pure Tone Block
      if (currentBlockTask == READPARAM) {
        if (r = ReadWord(bytesRead) == 2) {
          pilotLength = TickToUs (outWord);
        }
        if (r = ReadWord(bytesRead) == 2) {
          pilotPulses = outWord;
        }
        currentBlockTask = PILOT;
      } else {
        PureToneBlock();
      }
      break;

    case ID13:
      // process ID13 - Sequence of Pulses
      if (currentBlockTask == READPARAM) {
        if (r = ReadByte(bytesRead) == 1) {
          seqPulses = outByte;
        }
        currentBlockTask = DATA;
      } else {
        PulseSequenceBlock();
      }
      break;

    case ID14:
      // process ID14 - Pure Data Block
      if (currentBlockTask == READPARAM) {
        if (r = ReadWord(bytesRead) == 2) {
          zeroPulse = TickToUs (outWord);
        }
        if (r = ReadWord(bytesRead) == 2) {
          onePulse = TickToUs (outWord);
        }
        if (r = ReadByte(bytesRead) == 1) {
          usedBitsInLastByte = outByte;
        }
        if (r = ReadWord(bytesRead) == 2) {
          pauseLength = outWord;
        }
        if (r = ReadLong (bytesRead) == 3) {
          bytesToRead = outLong + 1;
        }
        currentBlockTask = DATA;
      } else {
        PureDataBlock();
      }
      break;

    case ID15:
      // process ID15 - Direct Recording
      if (currentBlockTask == READPARAM) {
        if (r = ReadWord(bytesRead) == 2) {
          // Number of T-states per sample (bit of data) 79 or 158 - 22.6757uS for 44.1KHz
          TstatesperSample = TickToUs (outWord);
        }
        if (r = ReadWord(bytesRead) == 2) {
          // pause after this block in milliseconds
          pauseLength = outWord;
        }
        if (r = ReadByte(bytesRead) == 1) {
          // used bits in last byte (other bits should be 0)
          usedBitsInLastByte = outByte;
        }
        if (r = ReadLong (bytesRead) == 3) {
          // length of samples' data
          bytesToRead = outLong + 1;
        }
        currentBlockTask = DATA;
      } else {
        currentPeriod = TstatesperSample;
        bitSet (currentPeriod, 14);
        DirectRecording();
      }
      break;

    case ID19:
      // process ID19 - Generalized data block
      switch (currentBlockTask) {
      case READPARAM:

        if (r = ReadDword (bytesRead) == 4) {
          // bytesToRead = outLong;
        }
        if (r = ReadWord(bytesRead) == 2) {
          // pause after this block in milliseconds
          pauseLength = outWord;
        }
        bytesRead += 86;         // skip until DataStream filename
        currentBlockTask = DATA;
        break;

      case DATA:
        ZX81DataBlock();
        break;
      }
      break;

    case ID20:
      // process ID20 - Pause Block
      if (r = ReadWord(bytesRead) == 2) {
        if (outWord > 0) {
          temppause = outWord;
          currentID = IDPAUSE;
        } else {
          currentTask = GETID;
        }
      }
      break;

    case ID21:
      // process ID21 - Group Start
      if (r = ReadByte(bytesRead) == 1) {
        bytesRead += outByte;
      }
      currentTask = GETID;
      break;

    case ID22:
      // process ID22 - Group End
      currentTask = GETID;
      break;

    case ID24:
      // process ID24 - Loop Start
      if (r = ReadWord(bytesRead) == 2) {
        loopCount = outWord;
        loopStart = bytesRead;
      }
      currentTask = GETID;
      break;

    case ID25:
      // process ID25 - Loop End
      loopCount -= 1;
      if (loopCount != 0) {
        bytesRead = loopStart;
      }
      currentTask = GETID;
      break;

    case ID2A:
      // skip
      bytesRead += 4;
      currentTask = GETID;
      break;

    case ID2B:
      // skip
      bytesRead += 5;
      currentTask = GETID;
      break;

    case ID30:
      // process ID30 - Text Description
      if (r = ReadByte(bytesRead) == 1) {
        bytesRead += outByte;
      }
      currentTask = GETID;
      break;

    case ID31:
      // process ID31 - Message block
      if (r = ReadByte(bytesRead) == 1) {
        // dispaytime = outByte;
      }
      if (r = ReadByte(bytesRead) == 1) {
        bytesRead += outByte;
      }
      currentTask = GETID;
      break;

    case ID32:
      // process ID32 - Archive Info
      // block Skipped until larger screen used
      if (ReadWord(bytesRead) == 2) {
        bytesRead += outWord;
      }
      currentTask = GETID;
      break;

    case ID33:
      // process ID32 - Archive Info
      // block Skipped until larger screen used
      if (ReadByte(bytesRead) == 1) {
        bytesRead += (long(outByte) * 3);
      }
      currentTask = GETID;
      break;

    case ID35:
      // process ID35 - Custom Info Block
      // block Skipped
      bytesRead += 0x10;
      if (r = ReadDword (bytesRead) == 4) {
        bytesRead += outLong;
      }
      currentTask = GETID;
      break;

    case ID4B:
      // process ID4B - Kansas City Block (MSX specific implementation only)
      switch (currentBlockTask) {
      case READPARAM:
        if (r = ReadDword (bytesRead) == 4) {  // data size to read
          bytesToRead = outLong - 12;
        }
        if (r = ReadWord(bytesRead) == 2) {   // pause after block in ms
          pauseLength = outWord;
        }
        // fixed speedup baudrate, reduced pilot duration
        pilotPulses = 10000;
        bytesRead += 10;
        switch (baudRate) {
          case BAUD3600:
            pilotLength = onePulse = TickToUs (243);
            zeroPulse = TickToUs (486);
            break;
          case BAUD2400:
            pilotLength = onePulse = TickToUs (365);
            zeroPulse = TickToUs (730);
            break;
          default:           // baud1200
            pilotLength = onePulse = TickToUs (729);
            zeroPulse = TickToUs (1458);
            break;
        }
        currentBlockTask = PILOT;
        break;

      case PILOT:
        // start with Pilot Pulses
        if (!pilotPulses--) currentBlockTask = DATA;
        else currentPeriod = pilotLength;
        break;

      case DATA:
        // data playback
        writeData4B();
        break;

      case PAUSE:
        // close block with a pause
        temppause = pauseLength;
        currentID = IDPAUSE;
        break;
      }
      break;

    case TAP:
      // pure Tap file block
      switch (currentBlockTask) {
      case READPARAM:
        pauseLength = PAUSELENGTH;
        if (r = ReadWord(bytesRead) == 2) {
          bytesToRead = outWord + 1;
        }
        if (r = ReadByte(bytesRead) == 1) {
          if (outByte == 0) {
            pilotPulses = PILOTNUMBERL + 1;
          } else {
            pilotPulses = PILOTNUMBERH + 1;
          }
          bytesRead -= 1;
        }
        pilotLength = PILOTLENGTH;
        sync1Length = SYNCFIRST;
        sync2Length = SYNCSECOND;
        zeroPulse = ZEROPULSE;
        onePulse = ONEPULSE;
        currentBlockTask = PILOT;
        usedBitsInLastByte = 8;
        break;

      default:
        StandardBlock();
        break;
      }
      break;

    case ZXP:
      switch (currentBlockTask) {
      case READPARAM:
        pauseLength = PAUSELENGTH * 5;
        currentChar = 0;
        currentBlockTask = PILOT;
        break;

      case PILOT:
        ZX81FilenameBlock();
        break;

      case DATA:
        ZX81DataBlock();
        break;
      }
      break;

    case ZXO:
      switch (currentBlockTask) {
      case READPARAM:
        pauseLength = PAUSELENGTH * 5;
        currentBlockTask = DATA;
        break;

      case DATA:
        ZX81DataBlock();
        break;
      }
      break;

    case AYO:                        // AY File - Pure AY file block - no header, must emulate it
      switch (currentBlockTask) {
      case READPARAM:
        pauseLength = PAUSELENGTH;   // 1 sec pause

        // here we must generate the TAP header which in pure AY files is missing.
        // this was done with a DOS utility called FILE2TAP which does not work under
        // recent 32bit OSs (only using DOSBOX).
        // taped AY files begin with a standard 0x13 0x00 header (0x13 bytes to follow) and contain the
        // name of the AY file (max 10 bytes) which we will display as "ZXAYFile " followed by the
        // length of the block (word), checksum plus 0xFF to indicate next block is DATA.
        // 13 00[00 03(5A 58 41 59 46 49 4C 45 2E 49)1A 0B 00 C0 00 80]21<->[1C 0B FF<AYFILE>CHK]

        // if (hdrptr==1) {
        //   bytesToRead = 0x13-2; // 0x13 0x0 - TAP Header minus 2 (FLAG and CHKSUM bytes) 17 bytes total
        // }
        if (hdrptr == HDRSTART) {
          pilotPulses = PILOTNUMBERL + 1;
        } else {
          pilotPulses = PILOTNUMBERH + 1;
        }
        pilotLength = PILOTLENGTH;
        sync1Length = SYNCFIRST;
        sync2Length = SYNCSECOND;
        zeroPulse = ZEROPULSE;
        onePulse = ONEPULSE;
        currentBlockTask = PILOT;               // now send pilot, SYNC1, SYNC2 and DATA
        if (hdrptr == HDRSTART) AYpass = 1;     // set AY TAP data read flag only if first run
        if (AYpass == 2) {                      // if we have already sent TAP header
          blkchksum = 0;
          bytesRead = 0;
          bytesToRead = ayblklen + 2;       // set length to read plus data byte and CHKSUM
          AYpass = 5;                       // reset flag to read from file and output header
        }
        usedBitsInLastByte = 8;
        break;

      default:
        StandardBlock();
        break;
      }
      break;

    case ORIC:
      switch (currentBlockTask) {
      case READPARAM:
      case SYNC1:
        if (currentBit > 0) OricBitWrite();
        else {
          ReadByte(bytesRead);
          currentByte = outByte;
          currentBit = 11;
          bitChecksum = 0;
          lastByte = 0;
          if (currentByte == 0x16) bufCount--;
          else {
            currentBit = 0;
            currentBlockTask = SYNC2;
          }
        }
        break;
      case SYNC2:
        if (currentBit > 0) OricBitWrite();
        else {
          if (bufCount > 0) {
            currentByte = 0x16;
            currentBit = 11;
            bitChecksum = 0;
            lastByte = 0;
            bufCount--;
          } else {
            bufCount = 1;
            currentBlockTask = SYNCLAST;
          }
        }
        break;

      case SYNCLAST:
        if (currentBit > 0) OricBitWrite();
        else {
          if (bufCount > 0) {
            currentByte = 0x24;
            currentBit = 11;
            bitChecksum = 0;
            lastByte = 0;
            bufCount--;
          } else {
            bufCount = 9;
            lastByte = 0;
            currentBlockTask = HEADER;
          }
        }
        break;

      case HEADER:
        if (currentBit > 0) OricBitWrite();
        else {
          if (bufCount > 0) {
            ReadByte(bytesRead);
            currentByte = outByte;
            currentBit = 11;
            bitChecksum = 0;
            lastByte = 0;
            if (bufCount == 5) bytesToRead = 256 * outByte;
            else if (bufCount == 4) bytesToRead += (outByte + 1);
            else if (bufCount == 3) bytesToRead -= (256 * outByte);
            else if (bufCount == 2) bytesToRead -= outByte;
            bufCount--;
          } else currentBlockTask = NAME;
        }
        break;

      case NAME:
        if (currentBit > 0) OricBitWrite();
        else {
          ReadByte(bytesRead);
          currentByte = outByte;
          currentBit = 11;
          bitChecksum = 0;
          lastByte = 0;
          if (currentByte == 0x00) {
            bufCount = 1;
            currentBit = 0;
            currentBlockTask = NAMELAST;
          }
        }
        break;

      case NAMELAST:
        if (currentBit > 0) OricBitWrite();
        else {
          if (bufCount > 0) {
            currentByte = 0x00;
            currentBit = 11;
            bitChecksum = 0;
            lastByte = 0;
            bufCount--;
          } else {
            bufCount = 100;
            lastByte = 0;
            currentBlockTask = GAP;
          }
        }
        break;

      case GAP:
        if (bufCount > 0) {
          currentPeriod = ORICONEPULSE;
          bufCount--;
        } else {
          currentBlockTask = DATA;
        }
        break;

      case DATA:
        OricDataBlock();
        break;

      case PAUSE:
        if (bufCount) {
          currentPeriod = 32769;
          bufCount -= 1;
        } else {
          bufCount = 255;
          currentBlockTask = SYNC1;
        }
        break;
      }
      break;

    case IDPAUSE:
      if (temppause > 0) {
        if (temppause > 8300) {
          currentPeriod = 8300;
          temppause -= 8300;
        } else {
          currentPeriod = temppause;
          temppause = 0;
        }
        bitSet (currentPeriod, 15);
      } else {
        currentTask = GETID;
        if (EndOfFile == true) currentID = EOF;
      }
      break;

    case EOF:
      // Handle end of file
      if (bufCount) {
        currentPeriod = 32767;
        bufCount -= 1;
      } else {
        stopFile();
        return;
      }
      break;

    default:
      // ID not recognized
      printtextF (PSTR ("ID? "), 0);
      itoa(currentID, tmp, 16);
      sendStrXY(tmp, 4, 0);
      itoa(bytesRead, tmp, 16);
      strcat_P (tmp, PSTR (" - L: "));
      printtext (tmp, 1);
      itoa(loopCount, tmp, 10);
      sendStrXY(tmp, 10, 1);
      delay(5000);
      stopFile();
      break;
    }
  }
}

void StandardBlock () {
  // standard Block Playback
  switch (currentBlockTask) {
  case PILOT:
    // start with Pilot Pulses
    currentPeriod = pilotLength;
    pilotPulses -= 1;
    if (pilotPulses == 0) {
      currentBlockTask = SYNC1;
    }
    break;

  case SYNC1:
    // first Sync Pulse
    currentPeriod = sync1Length;
    currentBlockTask = SYNC2;
    break;

  case SYNC2:
    // second Sync Pulse
    currentPeriod = sync2Length;
    currentBlockTask = DATA;
    break;

  case DATA:
    // data Playback
    if ((AYpass == 0) | (AYpass == 4) | (AYpass == 5)) writeData();
    // check if we are playing from file or Vector String and we need to
    // send first 0xFF byte or checksum byte at EOF
    else {
      // write TAP Header data from String Vector (AYpass=1)
      writeHeader();
    }
    break;

  case PAUSE:
    // close block with a pause
    if ((currentID != TAP) && (currentID != AYO)) {  // check if we have !=AYO too
      temppause = pauseLength;
      currentID = IDPAUSE;
    } else {
      currentPeriod = pauseLength;
      bitSet (currentPeriod, 15);
      currentBlockTask = READPARAM;
    }
    if (EndOfFile == true) currentID = EOF;
    break;
  }
}

void PureToneBlock () {
  // pure Tone Block - Long string of pulses with the same length
  currentPeriod = pilotLength;
  pilotPulses -= 1;
  if (pilotPulses == 0) {
    currentTask = GETID;
  }
}

void PulseSequenceBlock () {
  // pulse Sequence Block - String of pulses each with a different length
  // mainly used in speedload blocks
  byte r = 0;
  if (r = ReadWord(bytesRead) == 2) {
    currentPeriod = TickToUs (outWord);
  }
  seqPulses -= 1;
  if (seqPulses == 0) {
    currentTask = GETID;
  }
}

void PureDataBlock () {
  // pure Data Block - Data & pause only, no header, sync
  switch (currentBlockTask) {
  case DATA:
    writeData();
    break;
  case PAUSE:
    temppause = pauseLength;
    currentID = IDPAUSE;
    break;
  }
}

void writeData4B () {
  // convert byte (4B Block) from file into string of pulses.  One pulse per pass
  byte r;
  byte dataBit;
  // continue with current byte
  if (currentBit > 0) {
    if (currentBit == 11) {
      currentPeriod = zeroPulse;
      pass += 1;
      if (pass == 2) {
        currentBit -= 1;
        pass = 0;
      }
    } else
    if (currentBit <= 2) {
      currentPeriod = onePulse;
      pass += 1;
      if (pass == 4) {
        currentBit -= 1;
        pass = 0;
      }
    } else {
      // data bits
      dataBit = currentByte & 1;
      currentPeriod = dataBit == 1 ? onePulse : zeroPulse;
      pass += 1;
      if ((dataBit == 1 && pass == 4) || (dataBit == 0 && pass == 2)) {
        currentByte >>= 1;
        currentBit -= 1;
        pass = 0;
      }
    }
  } else
  if (currentBit == 0 && bytesToRead != 0) {
    // read new byte
    if (r = ReadByte(bytesRead) == 1) {
      bytesToRead -= 1;
      currentByte = outByte;
      currentBit = 11;
      pass = 0;
    } else if (r == 0) {
      // end of file
      currentID = EOF;
      return;
    }
  }

  // end of block?
  if (bytesToRead == 0 && currentBit == 0) {
    temppause = pauseLength;
    currentBlockTask = PAUSE;
  }
}

void writeSampleData () {
  // Convert byte from file into string of pulses.  One pulse per pass
  byte r;
  ID15switch = 1;
  if (currentBit == 0) {                      // check for byte end/first byte
    if (r = ReadByte(bytesRead) == 1) {       // read in a byte
      currentByte = outByte;
      bytesToRead -= 1;
      if (bytesToRead == 0) {                 // check for end of data block
        bytesRead -= 1;                       // rewind a byte if we've reached the end
        if (pauseLength == 0) {               // search for next ID if there is no pause
          currentTask = GETID;
        } else {
          currentBlockTask = PAUSE;           // otherwise start the pause
        }
        return;
      }
    } else if (r == 0) {
      EndOfFile = true;
      if (pauseLength == 0) {
        // ID15switch = 0;
        currentTask = GETID;
      } else {
        currentBlockTask = PAUSE;
      }
      return;
    }
    if (bytesToRead != 1) {                   // if we're not reading the last byte play all 8 bits
      currentBit = 8;
    } else {
      currentBit = usedBitsInLastByte;        // otherwise only play back the bits needed
    }
    pass = 0;
  }
  if (bitRead (currentPeriod, 14)) {
    if (currentByte & 0x80) bitSet (currentPeriod, 13);
    pass += 2;
  } else {
    if (currentByte & 0x80) {                 // set next period depending on value of bit 0
      currentPeriod = onePulse;
    } else {
      currentPeriod = zeroPulse;
    }
    pass += 1;
  }
  if (pass == 2) {
    currentByte <<= 1;                        // shift along to the next bit
    currentBit -= 1;
    pass = 0;
  }
}

void DirectRecording () {
  // direct Recording - Output bits based on specified sample rate (Ticks per clock) either 44.1KHz or 22.05
  switch (currentBlockTask) {
  case DATA:
    writeSampleData();
    break;

  case PAUSE:
    temppause = pauseLength;
    currentID = IDPAUSE;
    break;
  }
}

void ZX81FilenameBlock () {
  // output ZX81 filename
  if (currentBit == 0) {                    // check for byte end/first byte
    currentByte = pgm_read_byte (ZX81Filename + currentChar);
    currentChar += 1;
    if (currentChar == 10) {
      currentBlockTask = DATA;
      return;
    }
    currentBit = 9;
    pass = 0;
  }
  ZX81ByteWrite();
}

void ZX81DataBlock () {
  byte r;
  if (currentBit == 0) {                   // check for byte end/first byte
    if (r = ReadByte(bytesRead) == 1) {    // read in a byte
      currentByte = outByte;
      bytesToRead -= 1;
    } else if (r == 0) {
      temppause = pauseLength;
      currentID = IDPAUSE;
    }
    currentBit = 9;
    pass = 0;
  }
  ZX81ByteWrite();
}

void ZX81ByteWrite () {
  if (TurboMode) {
    currentPeriod = TURBOPULSE;
    if (pass == 1) currentPeriod = TURBOBITGAP;
  } else {
    currentPeriod = ZX81PULSE;
    if (pass == 1) currentPeriod = ZX81BITGAP;
  }
  if (pass == 0) {
    if (currentByte & 0x80) {     // set next period depending on value of bit 0
      pass = 19;
    } else {
      pass = 9;
    }
    currentByte <<= 1;            // shift along to the next bit
    currentBit -= 1;
    currentPeriod = 0;
  }
  pass -= 1;
}

void writeData () {
  // convert byte from file into string of pulses.  One pulse per pass
  byte r;

  if (currentBit == 0) {                    // check for byte end/first byte
    if (r = ReadByte(bytesRead) == 1) {     // read in a byte
      currentByte = outByte;
      if (AYpass == 5) {
        currentByte = 0xFF;                 // only insert first DATA byte if sending AY TAP DATA Block and don't decrement counter
        AYpass = 4;                         // set Checksum flag to be sent when EOF reached
        bytesRead -= 1;                     // rollback ptr and compensate for dummy read byte
        bytesToRead += 2;                   // add 2 bytes to read as we send 0xFF (data flag header byte) and chksum at the end of the block
      } else {
        bytesToRead -= 1;
      }
      blkchksum = blkchksum ^ currentByte;      // keep calculating checksum
      if (bytesToRead == 0) {                   // check for end of data block
        bytesRead -= 1;                         // rewind a byte if we've reached the end
        if (pauseLength == 0) {                 // search for next ID if there is no pause
          currentTask = GETID;
        } else {
          currentBlockTask = PAUSE;         // otherwise start the pause
        }
        return;                             // exit
      }
    } else if (r == 0) {                    // if we reached the EOF
      if (AYpass != 4) {                    // check if need to send checksum
        EndOfFile = true;
        if (pauseLength == 0) {
          currentTask = GETID;
        } else {
          currentBlockTask = PAUSE;
        }
        return;                             // return here if normal TAP or TZX
      } else {
        currentByte = blkchksum;            // else send calculated chksum
        bytesToRead += 1;                   // add one byte to read
        AYpass = 0;                         // reset flag to end block
      }
    }
    if (bytesToRead != 1) {                 // if we're not reading the last byte play all 8 bits
      currentBit = 8;
    } else {
      currentBit = usedBitsInLastByte;      // otherwise only play back the bits needed
    }
    pass = 0;
  }
  if (currentByte & 0x80) {                 // set next period depending on value of bit 0
    currentPeriod = onePulse;
  } else {
    currentPeriod = zeroPulse;
  }
  pass += 1;                                // data is played as 2 x pulses
  if (pass == 2) {
    currentByte <<= 1;                      // shift along to the next bit
    currentBit -= 1;
    pass = 0;
  }
}

void writeHeader () {
  // convert byte from HDR Vector String into string of pulses
  // and calculate checksum. One pulse per pass
  if (currentBit == 0) {            // check for byte end/new byte
    if (hdrptr == 19) {             // if we've reached end of header block send checksum byte
      currentByte = blkchksum;
      AYpass = 2;                   // set flag to Stop playing from header in RAM
      currentBlockTask = PAUSE;     // we've finished outputting the TAP header so
      return;                       // now PAUSE and send DATA block normally from file
    }
    hdrptr += 1;                    // increase header string vector pointer
    if (hdrptr < 20) {              // read a byte until we reach end of tap header
      currentByte = pgm_read_byte (TAPHdr + hdrptr);
      if (hdrptr == 13) {                          // insert calculated block length minus LEN bytes
        currentByte = lowByte (ayblklen - 3);
      } else if (hdrptr == 14) {
        currentByte = highByte (ayblklen);
      }
      blkchksum = blkchksum ^ currentByte;         // keep track of Chksum
      currentBit = 8;
    } else {
      currentBit = usedBitsInLastByte;    // otherwise only play back the bits needed
    }
    pass = 0;
  }
  if (currentByte & 0x80) {               // set next period depending on value of bit 0
    currentPeriod = onePulse;
  } else {
    currentPeriod = zeroPulse;
  }
  pass += 1;                              // data is played as 2 x pulses
  if (pass == 2) {
    currentByte <<= 1;                    // shift along to the next bit
    currentBit -= 1;
    pass = 0;
  }
}

void wave () {
  // ISR Output routine
  word workingPeriod = wbuffer[pos][workingBuffer];
  byte pauseFlipBit = false;
  unsigned long newTime = 1;
  intError = false;
  if (isStopped == 0 && workingPeriod >= 1) {
    if (bitRead (workingPeriod, 15)) {
      // if bit 15 of the current period is set we're about to run a pause
      // pauses start with a 1.5ms where the output is untouched after which the output is set LOW
      // pause block periods are stored in milliseconds not microseconds
      isPauseBlock = true;
      bitClear (workingPeriod, 15);         // Clear pause block flag
      pinState = !pinState;
      pauseFlipBit = true;
      wasPauseBlock = true;
    } else {
      if (workingPeriod >= 1 && wasPauseBlock == false) {
        pinState = !pinState;
      } else if (wasPauseBlock == true && isPauseBlock == false) {
        wasPauseBlock = false;
      }
      // if (wasPauseBlock==true && isPauseBlock==false) wasPauseBlock=false;
    }
    if (ID15switch == 1) {
      if (!bitRead (workingPeriod, 14)) {
        if (pinState == LOW) LowWrite(PORTB, PORTB1);
        else HighWrite(PORTB, PORTB1);
      } else {
        if (!bitRead (workingPeriod, 13)) LowWrite (PORTB, PORTB1);
        else {
          HighWrite (PORTB, PORTB1);
          bitClear (workingPeriod, 13);
        }
        bitClear (workingPeriod, 14);         // Clear ID15 flag
        workingPeriod = TstatesperSample;
      }
    } else {
      if (pinState == LOW) LowWrite(PORTB, PORTB1);
      else HighWrite(PORTB, PORTB1);
    }
    if (pauseFlipBit == true) {
      newTime = 1500;                       // set 1.5ms initial pause block
      if (FlipPolarity == 0) {
        pinState = LOW;
      } else {
        pinState = HIGH;
      }
      wbuffer[pos][workingBuffer] = workingPeriod - 1;  // reduce pause by 1ms
      pauseFlipBit = false;
    } else {
      if (isPauseBlock == true) {
        newTime = long(workingPeriod) * 1000; // set pause length in microseconds
        isPauseBlock = false;
      } else {
        newTime = workingPeriod;            // after all that, if it's not a pause block set the pulse period
      }
      pos += 1;
      if (pos > buffsize) {                 // swap buffer pages if we've reached the end
        pos = 0;
        workingBuffer ^= 1;
        morebuff = HIGH;                    // request more data to fill inactive page
      }
    }
  } else if (workingPeriod <= 1 && isStopped == 0) {
    newTime = 1000;                         // just in case we have a 0 in the buffer
    pos += 1;
    if (pos > buffsize) {
      pos = 0;
      workingBuffer ^= 1;
      morebuff = HIGH;
    }
  } else {
    newTime = 1000000;                         // just in case we have a 0 in the buffer
  }
  Timer1.setPeriod (newTime + 4);              // finally set the next pulse length
}


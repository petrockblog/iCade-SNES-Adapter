// Requires the use of the "UsbKeyboard" library available from
//   http://code.rancidbacon.com/ProjectLogArduinoUSB
#include "UsbKeyboard.h"

#include <SNESpad.h>

// Define NES pins
const int latch  = 6;
const int clock  = 7;
const int datin1 = 8;
const int datin2 = 10;

// put your own strobe/clock/data pin numbers here -- see the pinout in readme.txt
SNESpad nintendo1 = SNESpad(latch,clock,datin1);
SNESpad nintendo2 = SNESpad(latch,clock,datin2);

// iCade key mapping http://www.ionaudio.com/downloads/iCade_Dev_Resource_v1.3.pdf
// NES controller mapping      A      B    X Y L R select start   up    down   left   right
bool t1 = 1, prev_nes1[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };

// Define the button pin and LED pins
const int buttonPin = 12;
const int buttonLow  = 3;
const int buttonHigh = 11;
int buttonState;             // the current reading from the button pin

int state1 = 0;
int state2 = 0;

byte usbmap1[12];
byte usbmap2[12];
int  usbrecidx = 0;
byte keyStrokes[6];
bool doSend = false;

/**
 * Configure button inputs and set up the USB connection to the host
 */
void setup()
{
  // button setup ------------
  // Set up the LEDs for indicating the button status
  pinMode(buttonLow,OUTPUT);
  pinMode(buttonHigh,OUTPUT);
  // Set the button pin to be input
  pinMode(buttonPin, INPUT);

  // USB setup -----------------
  // Disable timer0 since it can mess with the USB timing. Note that
  // this means some functions such as delay() will no longer work.
  TIMSK0&=!(1<<TOIE0); // ++

  // Clear interrupts while performing time-critical operations
  cli();

  // Force re-enumeration so the host will detect us
  usbDeviceDisconnect();
  delayMs(250);
  usbDeviceConnect();

  // Set interrupts again
  sei();
  
  for (int k=0; k<sizeof(usbmap1); k++ ) {
    usbmap1[k] = (byte)255;
    usbmap2[k] = (byte)255;
  }  
  memset(keyStrokes,0,sizeof(keyStrokes));  
  
}

void setButtonLEDs() {
  
  // read the state of the switch into a local variable:
  buttonState = digitalRead(buttonPin);
  
  // set the LED using the state of the button:
  if ( buttonState==HIGH ) {
    digitalWrite( buttonLow,LOW );
    digitalWrite( buttonHigh,HIGH );
  } else {
    digitalWrite( buttonLow,HIGH );
    digitalWrite( buttonHigh,LOW );
  }
    
}

int setKeyStrokeIndex( int state, int SNES_MASK,byte* usbmap,int usbmidx,
                        byte* keyStrokes,byte USBKEY,int usbrecidx, bool* doSend ) {
  if (state & SNES_MASK) {
    // new button press
    if (usbmap[usbmidx]==255) {
      usbmap[usbmidx] = usbrecidx;
      keyStrokes[usbrecidx] = USBKEY;
      *doSend = true;
      for (int k=0; k<5; k++) {
        usbrecidx = (usbrecidx+1)%6;
        if (keyStrokes[usbrecidx]==0) 
          break;
      }
    }
  } else { // button release
      if (usbmap[usbmidx]!=255) {
        keyStrokes[usbmap[usbmidx]] = 0;      
        *doSend = true;
        usbmap[usbmidx] = 255;
    }
  }
  return usbrecidx;
}

void handleICadeKey( int state, int SNES_MASK, int prevIdx, byte keyPress, 
                     byte keyRelease, bool prev_nes[12] ) {

  t1 = state & SNES_MASK;
  if (prev_nes[prevIdx] != t1) { // button changed
    if (t1) {
      UsbKeyboard.sendMultiKeyStrokePR(0,keyPress,0,0,0,0,0);
    } else {
      UsbKeyboard.sendMultiKeyStrokePR(0,keyRelease,0,0,0,0,0);
    }
    prev_nes[prevIdx] = t1;
  }
      
}

/**
 * Main program loop. Scan for keypresses and send a matching keypress
 * event to the host
 * FIXME: currently repeats as fast as it can. Add transition detection
 */
void loop()
{
  
  state1 = nintendo1.buttons();
  state2 = nintendo2.buttons();

  UsbKeyboard.update();

  setButtonLEDs();
  
  doSend = false;
  
  if (buttonState==HIGH) { // normal keyboard mode
  
    // Pad 1 =================
    usbrecidx=setKeyStrokeIndex( state1,SNES_A,usbmap1, 0, keyStrokes, KEY_1,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_B,usbmap1, 1, keyStrokes, KEY_2,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_X,usbmap1, 2, keyStrokes, KEY_3,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_Y,usbmap1, 3, keyStrokes, KEY_4,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_L,usbmap1, 4, keyStrokes, KEY_5,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_R,usbmap1, 5, keyStrokes, KEY_6,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_SELECT,usbmap1, 6, keyStrokes, KEY_7,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_START,usbmap1, 7, keyStrokes, KEY_8,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_LEFT,usbmap1, 8, keyStrokes, KEY_ARROW_LEFT,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_RIGHT,usbmap1, 9, keyStrokes, KEY_ARROW_RIGHT,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_UP,usbmap1, 10, keyStrokes, KEY_ARROW_UP,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state1,SNES_DOWN,usbmap1, 11, keyStrokes, KEY_ARROW_DOWN,usbrecidx,&doSend );
  
    // Pad 2 =================
    usbrecidx=setKeyStrokeIndex( state2,SNES_A,usbmap2, 0, keyStrokes, KEY_E,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_B,usbmap2, 1, keyStrokes, KEY_R,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_X,usbmap2, 2, keyStrokes, KEY_T,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_Y,usbmap2, 3, keyStrokes, KEY_Y,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_L,usbmap2, 4, keyStrokes, KEY_U,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_R,usbmap2, 5, keyStrokes, KEY_I,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_SELECT,usbmap2, 6, keyStrokes, KEY_O,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_START,usbmap2, 7, keyStrokes, KEY_P,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_LEFT,usbmap2, 8, keyStrokes, KEY_A,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_RIGHT,usbmap2, 9, keyStrokes, KEY_D,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_UP,usbmap2, 10, keyStrokes, KEY_W,usbrecidx,&doSend );
    usbrecidx=setKeyStrokeIndex( state2,SNES_DOWN,usbmap2, 11, keyStrokes, KEY_S,usbrecidx,&doSend );
    
    if (doSend) {
      UsbKeyboard.sendMultiKeyStroke(0,keyStrokes[0],keyStrokes[1],keyStrokes[2],keyStrokes[3],keyStrokes[4],keyStrokes[5]);
    }

  } else { // iCade mode
  
      // controller 1
      handleICadeKey( state1,SNES_A,0,     KEY_L,KEY_V, &prev_nes1[0] );
      handleICadeKey( state1,SNES_B,1,     KEY_H,KEY_R, &prev_nes1[0] );
      handleICadeKey( state1,SNES_X,2,     KEY_O,KEY_G, &prev_nes1[0] );
      handleICadeKey( state1,SNES_Y,3,     KEY_Y,KEY_T, &prev_nes1[0] );
      handleICadeKey( state1,SNES_L,4,     KEY_I,KEY_M, &prev_nes1[0] );
      handleICadeKey( state1,SNES_R,5,     KEY_K,KEY_P, &prev_nes1[0] );
      handleICadeKey( state1,SNES_START,6, KEY_J,KEY_N, &prev_nes1[0] );
      handleICadeKey( state1,SNES_SELECT,7,KEY_U,KEY_F, &prev_nes1[0] );
      handleICadeKey( state1,SNES_LEFT,8,  KEY_A,KEY_Q, &prev_nes1[0] );
      handleICadeKey( state1,SNES_RIGHT,9, KEY_D,KEY_C, &prev_nes1[0] );
      handleICadeKey( state1,SNES_UP,10,   KEY_W,KEY_E, &prev_nes1[0] );
      handleICadeKey( state1,SNES_DOWN,11, KEY_X,KEY_Y, &prev_nes1[0] );

  }

  delayMs(20);
  
}


/**
 * Define our own delay function so that we don't have to rely on
 * operation of timer0, the interrupt used by the internal delay()
 */
void delayMs(unsigned int ms)
{
  for (int i = 0; i < ms; i++) {
    delayMicroseconds(1000);
  }
}




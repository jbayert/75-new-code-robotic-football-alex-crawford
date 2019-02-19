#include <PS3BT.h>
#include <usbhub.h>

//===========Uncomment a LED===========================
#include "Leds/Leds.cpp"
//#include "Leds/OldLeds.cpp"

//===========Uncomment a drive train===================
#include "DriveTrains/BasicDrive.cpp"
//#include "DriveTrains/SquareOmniDrive.cpp"

//===========Uncomment for tackle sensor===============
//#define TACKLE

//===========Uncomment to choose a Peripheral==========
//#include "Peripherals/CenterPeripheral.cpp" #define PERIPHERALS
//#include "Peripherals/QBPeripheral.cpp"     #define PERIPHERALS
//#include "Peripherals/KickerPeripheral.cpp" #define PERIPHERALS

//==========Uncomment if not using bag motors==========
#define OldMotors

//This just enables and disables the old motors
#ifdef OldMotors
int motorType = -1;
#else
int motorType = 1;
#endif

#if defined(OldMotors) and defined(BASIC_DRIVE) 
  #define MAX_TURN 90
#endif
#if not defined(OldMotors) and defined(BASIC_DRIVE)
  #define MAX_TURN 60
#endif
#if defined(SQUARE_OMNI_DRIVE)
  #define MAX_TURN 50
#endif 

//===================================

#define TACKLE_INPUT    6           // Tackle sensor is wired to pin 6
bool hasIndicated = false;
bool stayTackled = false;
int handicap = 3;
bool kidsMode = false;
int newconnect = 0;
int leftX, leftY, rightX, rightY;
USB Usb;
USBHub Hub1(&Usb);
BTD Btd(&Usb);
PS3BT PS3(&Btd);


void setup() {// This is stuff for connecting the PS3 to USB.
  Serial.begin(115200);       //Begin Serial Communications
  driveSetup(motorType);//Setup the drive train
  ledsSetup();          //Setup the leds
  flashLeds();          //flash the leds

  int newconnect = 0;         // Variable(boolean) for connection to ps3, also activates rumble
  
  #ifdef PERIPHERALS
  peripheralSetup();//Call the peripheral setup	
  #endif

#ifdef TACKLE
  pinMode(TACKLE_INPUT, INPUT); // define the tackle sensor pin as an input
#endif  
  if (Usb.Init() == -1)       // this is for an error message with USB connections
  {
    Serial.print(F("\r\nOSC did not start"));
    while (1);
  }
  Serial.print(F("\r\nPS3 Bluetooth Library Started"));
}

void loop() {

  Usb.Task();                           // This updates the input from the PS3 controller
  if (PS3.PS3Connected)                 // run if the controller is connected
  {
    if (newconnect == 0)                // this is the vibration that you feel when you first connect
    {
      green();
      PS3.moveSetRumble(64);
      PS3.setRumbleOn(100, 255, 100, 255); //VIBRATE!!!
      newconnect++;
    }
    if (PS3.getButtonClick(PS)) {
      PS3.disconnect();
      newconnect = 0;
    }
    //========================================Get Controller Input==========================================
    leftX = map(PS3.getAnalogHat(RightHatY), 0, 255, -90, 90);     // Recieves PS3
    leftY = map(PS3.getAnalogHat(RightHatX), 0, 255, 90, -90);     // Recieves PS3
    rightX = map(PS3.getAnalogHat(LeftHatY), 0, 255, -MAX_TURN, MAX_TURN);   // Recieves PS3
    rightY = map(PS3.getAnalogHat(LeftHatX), 0, 255, -90, 90);   // Recieves PS3
    if (abs(leftX) < 8) leftX = 0;                                // deals with the stickiness
    if (abs(leftY) < 8) leftY = 0;
    if (abs(rightX) < 8) rightX = 0;
    if (abs(rightY) < 8) rightY = 0;
    //======================Specify the handicap================================    
    if (PS3.getButtonPress(R2) && (kidsMode == false)) {
      handicap = 1;
      //Serial.println(handicap);
    } else if (PS3.getButtonPress(L2)) {
      handicap = 6;
    } else if (kidsMode == false) {
      handicap = 3;
    }
    if (PS3.getButtonClick(START)) {
      if (kidsMode == false) {
        handicap = 7;
        kidsMode = true;
        PS3.setLedRaw(9);               // ON OFF OFF ON
        PS3.setRumbleOn(5, 255, 5, 255);// vibrate both, then left, then right
        //Serial.println(handicap);
      } else if (kidsMode == true) {
        
        kidsMode = false;
        PS3.setLedRaw(1);               // OFF OFF OFF ON
        PS3.setRumbleOn(5, 255, 5, 255);// vibrate both, then left, then right
        //Serial.println(handicap);
      }
    }
    
    //==========================================================================

    //=================================Tackle Sensor================================
#ifdef TACKLE
    // NORMAL OPERATION MODE
    // for the if statement for whether or not
    // tackle is enabled. cool stuff
    if (PS3.getButtonClick(LEFT)) {
      if (stayTackled == true) {
        stayTackled = false;
      } else {
        stayTackled = true;
      }
      PS3.setRumbleOn(30, 255, 30, 255);
    }
    if (!digitalRead(TACKLE_INPUT))
    {
      red();
      if (!hasIndicated) {
        PS3.setRumbleOn(10, 255, 10, 255);
        hasIndicated = true;
      }
    }
    else
    {
      if (stayTackled == false) {
        hasIndicated = false;
        green();
      }
    }
#endif
    //===============================================================================================
    #if defined(SQUARE_OMNI_DRIVE)
    if (PS3.getButtonPress(R1)) {
      Serial.println("START");
      Serial.println(leftX);
      leftX  *= -1;
      leftY  *= -1;
      rightX *= -1;
      rightY *= -1;         
      Serial.println(leftX);
    }
    #endif
    driveCtrl(handicap, leftX, leftY, rightX, rightY);//Drive the drive train      
    

#ifdef PERIPHERALS
    peripheral(PS3);//Call the peripheral
#endif
  }
  if (!PS3.PS3Connected) {
    blue();
    driveStop();
  }
}

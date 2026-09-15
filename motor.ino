class LCD{

private:
  int currentX;
  int currentY;
  int rs;
  int e;
  int d4;
  int d5;
  int d6; 
  int d7;
  bool screenActiveVal; 
  bool cursorActiveVal;
  bool blinkingCursorVal;


  // write nibble
  void writeNibble(bool rs_1, uint8_t nibble){
    digitalWrite(rs, rs_1);
    delay(2);
    digitalWrite(e, HIGH);
    delay(2);
    digitalWrite(d4, (nibble & 1) ? HIGH : LOW);
    digitalWrite(d5, (nibble & 2) ? HIGH : LOW);
    digitalWrite(d6, (nibble & 4) ? HIGH : LOW);
    digitalWrite(d7, (nibble & 8) ? HIGH : LOW);
    digitalWrite(e, LOW);
  };

  void funcSet(){

    // timing  table
    writeNibble(false, 3);
    delay(5);
    writeNibble(false, 3);
    delayMicroseconds(150);
    writeNibble(false, 3);
    writeNibble(false, 2);

    uint8_t command = 0;
    command |= 1 <<5; // Command Start
    command |= 1 <<3; // N
    sendCommand(false, command);
  };

  void entryMode(){
    uint8_t command = 0; 
    command |= 1 <<1;
    command |= 1<<2;
    sendCommand(false, command);
  };

public:

  // Constructor
  LCD(int rs_constr, int e_constr, int d4_constr, int d5_constr, int d6_constr, int d7_constr){
    currentX = 0;
    currentY = 0; 
    rs = rs_constr;
    e = e_constr;
    d4 = d4_constr;
    d5 = d5_constr;
    d6 = d6_constr;
    d7 = d7_constr;
    screenActiveVal = false; 
    cursorActiveVal = false;
    blinkingCursorVal = false;

    // pins
    pinMode(rs, OUTPUT);
    pinMode(e,  OUTPUT);
    pinMode(d4, OUTPUT);
    pinMode(d5, OUTPUT);
    pinMode(d6, OUTPUT);
    pinMode(d7, OUTPUT);    

    // init the LCD
    funcSet();
    screenActive(true);
    cursorActive(true);
    entryMode();
    moveCursor(0,0);
    blinkingCursor(true);
    clear();

  };

 void moveCursor(int x, int y){
  uint8_t command = 1<<7;
  command += x;
  if(y == 1){
    command += 0x40;
  }
  sendCommand(false, command);
}

  // write and move 
  void writeAt(int x, int y, String message){
    moveCursor(x,y);
    write(message);
  };

  //writes at current pos
  void write(String message){
    for(char c: message){
      sendCommand(true, c);
    }
  };

  //writes at current pos
  void writeChar(char message){
      sendCommand(true, message);
  };

  //clears screen
  void clear(){
    sendCommand(false, 1);
  };

  // backdoor 
  void sendCommand(bool rs, uint8_t command){
    uint8_t commandupper = command >> 4;
    writeNibble(rs, commandupper);
    writeNibble(rs, command);
    delay(2);
  };

  //clear line
  void clearLine(int y){
    writeAt(0, y, "                ");
  };

  // screen turn on and off
  void screenActive(bool on){
    screenActiveVal = on; 
    uint8_t command = 0;
    command = 1<<3;
    if(on){
      command = command | 1<<2;
    }
    command = command | cursorActiveVal<<1;
    command = command | blinkingCursorVal;
    sendCommand(false, command);
  };

  // blinking cursor
  void blinkingCursor(bool on){
    blinkingCursorVal = on; 
    uint8_t command = 0;
    command = 1<<3;
    command = command | on;
    command = command | cursorActiveVal<<1;
    command = command | screenActiveVal<<2;
    sendCommand(false, command);
  };

  //turn cursor on/ off
  void cursorActive(bool on){
    cursorActiveVal = on; 
    uint8_t command = 0;
    command = 1<<3;
    command = command | on<<1;
    command = command | blinkingCursorVal;
    command = command | screenActiveVal<<2;
    sendCommand(false, command);
  };
};


/**
 * @brief Control commands available for the robot.
 */
#define FORWARD     'F'
#define LEFT        'L'
#define BACKWARD    'B'
#define RIGHT       'R'
#define STOP        'S'
#define IDLEE       'I' 

/**
 * @brief Start and end delimiters
 * @details Ideally these would be high ASCII characters and MUST
 * be something that doesn't occur inside of a legal packet!
 */
const char SoP = 'C';
const char EoP = 'E';

/**
 * @brief Other constants and variables for communication
 */
const char nullTerminator = '\0';
unsigned char inByte;
#define MESSAGE_MAX_SIZE 5
char message[MESSAGE_MAX_SIZE];
char command;

/**
 * @brief Definitions of different speed levels.
 * @details Here we define idle as 0 and full speed as 150. For now,
 * we don't have intermediate speed levels in the program. But you
 * are welcome to add them and change the code.
 */
const int IDLE  = 0;
const int SPEED = 150;

/**
 * @brief H bridge (SN754410) pin connections to Arduino.
 * @details The main ideas is to drive two enable signals with PWMs to
 * control the speed. Connect the 4 logic inputs to any 4 Arduino
 * digital pins. We can also connect enables to vcc to save pins, but
 * then we need 2 or 4 pwm pins. These methods are all feasible, but you
 * need to adjust the setup and the function 'motorControl' accordingly.
 */
/// pwm pin, control left motor
const int EN1 = 3;
/// pwm pin, control right motor
const int EN2 = 5;
/// control Y1 (left motor positive)
const int A_1 = 7;
/// control Y2 (left motor negative)
const int A_2 = 8;
/// control Y3 (right motor positive)
const int A_3 = 9;
/// control Y4 (right motor negative)
const int A_4 = 13;
#define LEFT_MOTOR  true
#define RIGHT_MOTOR false

/**
 * @brief Packet parser for serial communication
 * @details Called in the main loop to get legal message from serial port.
 * @return true on success, false on failure
 */
bool parsePacket();

/**
 * @brief Control the robot
 * @param command FORWARD, LEFT, BACKWARD or STOP.
 */
void moveRobot(char command);

/**
 * @brief Control motor
 * @details Called by `moveRobot` to break down high level command
 * to each motor.
 *
 * @param ifLeftMotor Either Left motor or right motor.
 * @param command FORWARD, STOP or BACKWARD
 */
void motorControl(bool ifLeftMotor, char command);

/**
 * @brief Pin setup and initialize state
 * @details Enable serial communication, set up H bridge connections,
 * and make robot stop at the beginning.
 */

void setup() {
    Serial.begin(38400);
    Serial.println("START");
    pinMode(EN1, OUTPUT);
    pinMode(EN2, OUTPUT);
    pinMode(A_1, OUTPUT);
    pinMode(A_2, OUTPUT);
    pinMode(A_3, OUTPUT);
    pinMode(A_4, OUTPUT);

    moveRobot(STOP);
    delay(10);
    Serial.println("Ready, Steady, Go");
    delay(10);

}

/**
 * @brief Main loop of the program
 * @details In this function, we get messages from serial port and
 * execute them accordingly.
 */
void loop() {

     // DECLARE LCD
    LCD lcd(2, 4,6,10, 11,12);
    while(true){
    /// 1. get legal message
    if (!parsePacket()){
        moveRobot('I');
        return;
    }

    /// 2. action, for now we only use option 1
    if (message[0] == '1') {
        // Move command
        command = message[1];
        moveRobot(command);
       Serial.println("MOVING");

    }
    else if (message[0] == '2') {
        // Display Read
        // ...
    }
    else if (message[0] == '3') {
        // Distance Read
        // ...
    }
    else if (message[0] == '4') {
        // Display Write
        Serial.println(message);
        return;
    }
    else if(message[0] == '5'){
        command = message[1];
        Serial.println("PRINTING");
        if(command == '-'){
            lcd.clear();
        }
        lcd.writeChar(command);
    }
    else {
        Serial.println("ERROR: unknown message");
        return;
    }
    }
}


/**
 * @brief FUNCTION IMPLEMENTATIONS BELOW
 */


bool parsePacket() {
    /// step 1. get SoP
    while (Serial.available() < 1) {};
    inByte = Serial.read();
    if (inByte != SoP) {
        Serial.print("ERROR: Expected SOP, got: ");
        Serial.write((byte)inByte);
        Serial.print("\n");
        return false;
    }

    /// step 2. get message length
    while (Serial.available() < 1) {};
    inByte = Serial.read();
    if (inByte == EoP || inByte == SoP) {
        Serial.println("ERROR: SoP/EoP in length field");
        return false;
    }
    int message_size = inByte - '0';
    if (message_size > MESSAGE_MAX_SIZE || message_size < 0) {
        Serial.println("ERROR: Packet Length out of range");
        return false;
    }

    /// step 3. get message
    for (int i = 0; i < message_size; i++) {
        while (Serial.available() < 1) {};
        inByte = Serial.read();
        if ((inByte == EoP || inByte == SoP)) {
            Serial.println("ERROR: SoP/EoP in command field");
            return false;
        }
        message[i] = (char)inByte;
    }
    message[message_size] = nullTerminator;

    /// step 4. get EoP
    while (Serial.available() < 1) {};
    inByte = Serial.read();
    if (inByte != EoP) {
        Serial.println("EoP not found");
        return false;
    } else {
        return true;
    }
}

void moveRobot(char command) {
    switch(command) {
        case FORWARD:
            Serial.println("FORWARD");
            motorControl(LEFT_MOTOR, FORWARD);
            motorControl(RIGHT_MOTOR, FORWARD);
            break;
        case LEFT:
            Serial.println("LEFT");
            motorControl(LEFT_MOTOR, STOP);
            motorControl(RIGHT_MOTOR, FORWARD);
            break;
        case BACKWARD:
            Serial.println("BACKWARD");
            motorControl(LEFT_MOTOR, BACKWARD);
            motorControl(RIGHT_MOTOR, BACKWARD);
            break;
        case RIGHT:
            Serial.println("RIGHT");
            motorControl(LEFT_MOTOR, FORWARD);
            motorControl(RIGHT_MOTOR, STOP);
            break;
        case STOP:
            Serial.println("STOP");
            motorControl(LEFT_MOTOR,STOP);
            motorControl(RIGHT_MOTOR,STOP);
            break;
        case IDLEE:
            Serial.println("IDLE");
            motorControl(LEFT_MOTOR,IDLEE);
            motorControl(RIGHT_MOTOR,IDLEE);
            break;
        default:
            Serial.println("ERROR: Unknown command in legal packet");
            break;
    }
}

/**
 * @brief Please rewrite the function if you change H bridge connections.
 */
void motorControl(bool ifLeftMotor, char command) {
    int enable   = ifLeftMotor ? EN1 : EN2;
    int motorPos = ifLeftMotor ? A_1 : A_3;
    int motorNeg = ifLeftMotor ? A_2 : A_4;
    switch (command) {
        case FORWARD:
            analogWrite(enable, SPEED);
            digitalWrite(motorPos, HIGH);
            digitalWrite(motorNeg, LOW);
            break;
        case BACKWARD:
            analogWrite(enable, SPEED);
            digitalWrite(motorPos, LOW);
            digitalWrite(motorNeg, HIGH);
            break;
        case STOP:
            digitalWrite(motorPos, LOW);
            digitalWrite(motorNeg, LOW);
            break;
        case IDLEE:
            analogWrite(enable, IDLE);
            break;
        default:
            break;
    }
}

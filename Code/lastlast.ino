#include <Servo.h>

#define MOTOR1_PIN 5
#define MOTOR2_PIN 3
#define MOTOR3_PIN 9
#define MOTOR4_PIN 10
Servo motor1, motor2, motor3, motor4;

// Motor position variables
byte motor1Pos = 90;
byte motor2Pos = 90;
byte motor3Pos = 90;
byte motor4Pos = 90;
byte selectedMotor = 1;

// Recording and playback variables
struct Movement {
  byte motor1;
  byte motor2;
  byte motor3;
  byte motor4;
  unsigned long delayTime;
};

#define MAX_STEPS 50
Movement recordedSteps[MAX_STEPS];
byte totalSteps = 0;
unsigned long lastRecordTime = 0;

enum Mode { MANUAL, RECORD, PLAYBACK };
Mode currentMode = MANUAL;

// Playback variables
unsigned long playbackStartTime = 0;
byte currentPlayStep = 0;
bool isMoving = false;
bool isPlaying = false;
unsigned long moveStartTime = 0;
byte startPositions[4];
byte targetPositions[4];
unsigned long stepDelayEndTime = 0;

void setup() {
  pinMode(4, INPUT_PULLUP); // Playback button
  pinMode(6, INPUT_PULLUP); // Record button

  motor1.attach(MOTOR1_PIN);
  motor2.attach(MOTOR2_PIN);
  motor3.attach(MOTOR3_PIN);
  motor4.attach(MOTOR4_PIN);
  
  // Initialize to current positions (no forced neutral position)
  motor1Pos = motor1.read();
  motor2Pos = motor2.read();
  motor3Pos = motor3.read();
  motor4Pos = motor4.read();
  
  motor1.write(motor1Pos);
  motor2.write(motor2Pos);
  motor3.write(motor3Pos);
  motor4.write(motor4Pos);
  
  Serial.begin(9600);
  Serial.println("READY");
}

void loop() {
  checkModeButtons();
  
  switch(currentMode) {
    case MANUAL:
      // Motors maintain their positions automatically
      break;
    case RECORD:
      // Nothing needed here for now
      break;
    case PLAYBACK:
      playbackMode();
      break;
  }

  processSerialCommands();
}

void checkModeButtons() {
  static bool lastRecordState = HIGH;
  static bool lastPlayState = HIGH;
  static unsigned long lastPlayPressTime = 0;
  
  bool recordState = digitalRead(6);
  bool playState = digitalRead(4);
  
  // Record button pressed
  if(recordState == LOW && lastRecordState == HIGH) {
    delay(50);
    if(digitalRead(6)) return;
    
    if(currentMode != RECORD) {
      enterRecordMode();
    } else {
      saveCurrentPosition();
    }
  }
  
  // Play button pressed - double press to stop
  if(playState == LOW && lastPlayState == HIGH) {
    delay(50);
    if(digitalRead(4)) return;
    
    unsigned long currentTime = millis();
    if(currentTime - lastPlayPressTime < 500) { // Double press
      if(currentMode == PLAYBACK) {
        exitPlaybackMode();
      }
    } else { // Single press
      if(currentMode != PLAYBACK) {
        enterPlaybackMode();
      }
    }
    lastPlayPressTime = currentTime;
  }
  
  lastRecordState = recordState;
  lastPlayState = playState;
}

void enterRecordMode() {
  currentMode = RECORD;
  totalSteps = 0;
  lastRecordTime = millis();
  saveCurrentPosition();
  Serial.println("RECORD MODE - Press record button to save positions");
}

void exitRecordMode() {
  currentMode = MANUAL;
  Serial.print("Recording saved with ");
  Serial.print(totalSteps);
  Serial.println(" steps");
}

void saveCurrentPosition() {
  if(totalSteps >= MAX_STEPS) {
    Serial.println("MAX STEPS REACHED");
    exitRecordMode();
    return;
  }
  
  unsigned long currentTime = millis();
  
  if(totalSteps > 0) {
    recordedSteps[totalSteps-1].delayTime = currentTime - lastRecordTime;
  }
  
  recordedSteps[totalSteps].motor1 = motor1Pos;
  recordedSteps[totalSteps].motor2 = motor2Pos;
  recordedSteps[totalSteps].motor3 = motor3Pos;
  recordedSteps[totalSteps].motor4 = motor4Pos;
  recordedSteps[totalSteps].delayTime = 0;
  
  totalSteps++;
  lastRecordTime = currentTime;
  
  Serial.print("Position saved (Step ");
  Serial.print(totalSteps);
  Serial.println(")");
}

void enterPlaybackMode() {
  if(totalSteps == 0) {
    Serial.println("NO RECORDED MOVEMENTS");
    return;
  }
  
  currentMode = PLAYBACK;
  currentPlayStep = 0;
  isPlaying = true;
  startMovementToStep(0);
  Serial.println("PLAYBACK STARTED");
}

void exitPlaybackMode() {
  currentMode = MANUAL;
  isPlaying = false;
  isMoving = false;
  Serial.println("PLAYBACK STOPPED");
}

void startMovementToStep(byte stepIndex) {
  isMoving = true;
  moveStartTime = millis();
  
  startPositions[0] = motor1.read();
  startPositions[1] = motor2.read();
  startPositions[2] = motor3.read();
  startPositions[3] = motor4.read();
  
  targetPositions[0] = recordedSteps[stepIndex].motor1;
  targetPositions[1] = recordedSteps[stepIndex].motor2;
  targetPositions[2] = recordedSteps[stepIndex].motor3;
  targetPositions[3] = recordedSteps[stepIndex].motor4;
}

void playbackMode() {
  if(!isPlaying) return;

  if(isMoving) {
    // Smooth movement between positions (500ms duration)
    unsigned long elapsed = millis() - moveStartTime;
    float progress = min((float)elapsed / 500.0, 1.0);
    
    motor1.write(startPositions[0] + (targetPositions[0] - startPositions[0]) * progress);
    motor2.write(startPositions[1] + (targetPositions[1] - startPositions[1]) * progress);
    motor3.write(startPositions[2] + (targetPositions[2] - startPositions[2]) * progress);
    motor4.write(startPositions[3] + (targetPositions[3] - startPositions[3]) * progress);
    
    if(progress >= 1.0) {
      isMoving = false;
      stepDelayEndTime = millis() + recordedSteps[currentPlayStep].delayTime;
      
      // Update motor positions after movement completes
      motor1Pos = targetPositions[0];
      motor2Pos = targetPositions[1];
      motor3Pos = targetPositions[2];
      motor4Pos = targetPositions[3];
    }
  } else {
    // Check if delay time has passed
    if(millis() >= stepDelayEndTime) {
      currentPlayStep++;
      
      if(currentPlayStep >= totalSteps) {
        currentPlayStep = 0; // Loop back to start
      }
      
      startMovementToStep(currentPlayStep);
    }
  }
}

void processSerialCommands() {
  if(Serial.available() > 0) {
    char cmd = Serial.read();
    while(Serial.available() > 0) Serial.read();
    
    switch(cmd) {
      case '1': 
        selectedMotor = 1; 
        Serial.println("Motor 1 selected"); 
        break;
      case '2': 
        selectedMotor = 2; 
        Serial.println("Motor 2 selected"); 
        break;
      case '3': 
        selectedMotor = 3; 
        Serial.println("Motor 3 selected"); 
        break;
      case '4': 
        selectedMotor = 4; 
        Serial.println("Motor 4 selected"); 
        break;
      
      case 'F': 
        changeMotorPos(selectedMotor, +5); 
        break;
      case 'B': 
        changeMotorPos(selectedMotor, -5); 
        break;
      case '+': 
        changeMotorPos(selectedMotor, +1); 
        break;
      case '-': 
        changeMotorPos(selectedMotor, -1); 
        break;
      
      case 'P': 
        if(currentMode != PLAYBACK) enterPlaybackMode(); 
        else exitPlaybackMode();
        break;
      case 'R': 
        if(currentMode != RECORD) enterRecordMode(); 
        else exitRecordMode();
        break;
    }
  }
}

void changeMotorPos(byte motor, int posChange) {
  byte *posPtr = getMotorPosPtr(motor);
  *posPtr = constrain(*posPtr + posChange, 0, 180);
  updateMotor(motor);
  printMotorStatus(motor);
}

byte* getMotorPosPtr(byte motor) {
  switch (motor) {
    case 1: return &motor1Pos;
    case 2: return &motor2Pos;
    case 3: return &motor3Pos;
    case 4: return &motor4Pos;
    default: return &motor1Pos;
  }
}

void updateMotor(byte motor) {
  switch (motor) {
    case 1: motor1.write(motor1Pos); break;
    case 2: motor2.write(motor2Pos); break;
    case 3: motor3.write(motor3Pos); break;
    case 4: motor4.write(motor4Pos); break;
  }
}

void printMotorStatus(byte motor) {
  Serial.print("Motor ");
  Serial.print(motor);
  Serial.print(": ");
  Serial.println(*getMotorPosPtr(motor));
}
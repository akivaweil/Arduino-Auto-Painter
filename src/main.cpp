#include <AccelStepper.h>
#include <Bounce2.h>

// Pin Definitions
const int X_STEP_PIN = 5;
const int X_DIR_PIN = 6;
const int Y_STEP_PIN = 11;
const int Y_DIR_PIN = 10;
const int ROTATION_STEP_PIN = A1;
const int ROTATION_DIR_PIN = A0;
const int PAINT_RELAY_PIN = 4;
const int X_HOME_SENSOR_PIN = 12;
const int Y_HOME_SENSOR_PIN = 8;

// System Configuration
const int X_STEPS_PER_INCH = 127;    // X-axis calibration
const int Y_STEPS_PER_INCH = 169;    // Y-axis calibration (adjust as needed)
const int STEPS_PER_ROTATION = 800;  
int X_SPEED = 5000;      
int Y_SPEED = 5000;      
int ROTATION_SPEED = 1000;
int X_ACCEL = 20000;     
int Y_ACCEL = 5000;    
int ROTATION_ACCEL = 200;

// Forward declarations
class Command;
void executeCommand(const Command& cmd);
void processPattern();

// Command Structure
class Command {
public:
    char type;      // 'X' for X move, 'Y' for Y move, 'R' for rotate, 'S' for spray
    float value;    // Distance in inches or degrees for rotation
    bool sprayOn;   // Whether spray should be on during movement
    
    Command(char t, float v, bool s) : type(t), value(v), sprayOn(s) {}
};

// Command Creation Macros
#define MOVE_X(dist, spray) Command('X', dist, spray)
#define MOVE_Y(dist, spray) Command('Y', dist, spray)
#define ROTATE(deg) Command('R', deg, false)
#define SPRAY_ON() Command('S', 0, true)
#define SPRAY_OFF() Command('S', 0, false)

// State Management
enum SystemState {
    IDLE,
    HOMING_X,
    HOMING_Y,
    EXECUTING_PATTERN,
    ERROR,
    CYCLE_COMPLETE
};

// Global Variables
SystemState systemState = IDLE;
bool motorsRunning = false;
int currentSide = 0;
int currentCommand = 0;


// Initialize Hardware
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperRotation(AccelStepper::DRIVER, ROTATION_STEP_PIN, ROTATION_DIR_PIN);
Bounce xHomeSensor = Bounce();
Bounce yHomeSensor = Bounce();




/*
 * CNC Paint Gun Movement Program
 * Symbol Legend:
 * → = Right movement
 * ← = Left movement
 * ● = Spray On
 * ○ = Spray Off
 * ↑ = Positive Y
 * ↓ = Negative Y
 */

Command SIDE1_PATTERN[] = {
    // Initial Movement
    MOVE_X(4.5, false),     // →3.5→ - Initial offset

    // Row 1
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26, true),      // →26→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 2
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26, true),     // ←26← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 3
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26, true),      // →26→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 4
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26, true),     // ←26← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 5
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26, true),      // →26→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26, true),     // ←26← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 7
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26, true),      // →26→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(4.16, false),   // ↑4.16↑ - Move up

    // Row 8
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26, true),     // ←26← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray

    // Rotation
    ROTATE(180)            // Rotate tray 180 degrees
};

Command SIDE2_PATTERN[] = {
    // Row 1
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26, true),      // →26→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 2
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26, true),     // ←26← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 3
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26, true),      // →26→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 4
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26, true),     // ←26← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 5
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26, true),      // →26→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 6
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26, true),     // ←26← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 7
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(26, true),      // →26→ - Move right with spray
    SPRAY_OFF(),           // ○ - Stop spray
    MOVE_Y(-4.16, false),  // ↓4.16↓ - Move down

    // Row 8
    SPRAY_ON(),            // ● - Start spray
    MOVE_X(-26, true),     // ←26← - Move left with spray
    SPRAY_OFF(),           // ○ - Stop spray

    // Final movement and rotation
    MOVE_X(-4.5, false), // ←3.5← Return to start
    ROTATE(90)             // Rotate tray 90 degrees
};

Command SIDE3_PATTERN[] = {
    // Initial Movement
    MOVE_Y(4.5, false),     // ↑3.5↑ Initial offset

    // Row 1
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(35, true),       // →35→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(4.415, false),   // ↑4.415↑ - Move up

    // Row 2
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-35, true),      // ←35← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(4.415, false),   // ↑4.415↑ - Move up

    // Row 3
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(35, true),       // →35→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(4.415, false),   // ↑4.415↑ - Move up

    // Row 4
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-35, true),      // ←35← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(4.415, false),   // ↑4.415↑ - Move up

    // Row 5
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(35, true),       // →35→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(4.415, false),   // ↑4.415↑ - Move up

    // Row 6
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-35, true),      // ←35← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray

    // Rotation
    ROTATE(180)             // Rotate tray 180 degrees
};

Command SIDE4_PATTERN[] = {
    // Row 1
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(35, true),       // →35→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 2
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-35, true),      // ←35← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 3
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(35, true),       // →35→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 4
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-35, true),      // ←35← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 5
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(35, true),       // →35→ - Move right with spray
    SPRAY_OFF(),            // ○ - Stop spray
    MOVE_Y(-4.415, false),  // ↓4.415↓ - Move down

    // Row 6
    SPRAY_ON(),             // ● - Start spray
    MOVE_X(-35, true),      // ←35← - Move left with spray
    SPRAY_OFF(),            // ○ - Stop spray

    // Final movement
    MOVE_Y(-4.5, false)   // ↓3.5↓ Return to start
};

// Pattern Sizes
const int SIDE1_SIZE = sizeof(SIDE1_PATTERN) / sizeof(Command);
const int SIDE2_SIZE = sizeof(SIDE2_PATTERN) / sizeof(Command);
const int SIDE3_SIZE = sizeof(SIDE3_PATTERN) / sizeof(Command);
const int SIDE4_SIZE = sizeof(SIDE4_PATTERN) / sizeof(Command);


void executeCommand(const Command& cmd) {
    switch(cmd.type) {
        case 'X':
            if(cmd.sprayOn) digitalWrite(PAINT_RELAY_PIN, LOW);
            stepperX.move(cmd.value * X_STEPS_PER_INCH);  // Use X-specific calibration
            break;
            
        case 'Y':
            if(cmd.sprayOn) digitalWrite(PAINT_RELAY_PIN, LOW);
            stepperY.move(cmd.value * Y_STEPS_PER_INCH);  // Use Y-specific calibration
            break;
            
        case 'R':
            stepperRotation.move((cmd.value * 5000) / 360);
            break;
            
        case 'S':
            digitalWrite(PAINT_RELAY_PIN, cmd.sprayOn ? LOW : HIGH);
            break;
    }
}

void setup() {
    Serial.begin(115200);
    
    pinMode(PAINT_RELAY_PIN, OUTPUT);
    digitalWrite(PAINT_RELAY_PIN, HIGH);
    
    pinMode(X_HOME_SENSOR_PIN, INPUT_PULLUP);
    pinMode(Y_HOME_SENSOR_PIN, INPUT_PULLUP);
    
    xHomeSensor.attach(X_HOME_SENSOR_PIN);
    yHomeSensor.attach(Y_HOME_SENSOR_PIN);
    xHomeSensor.interval(10);
    yHomeSensor.interval(10);
    
    stepperX.setMaxSpeed(500);
    stepperX.setAcceleration(X_ACCEL);
    stepperX.setPinsInverted(true);
    
    stepperY.setMaxSpeed(500);
    stepperY.setAcceleration(Y_ACCEL);
    stepperY.setPinsInverted(false);
    
    stepperRotation.setMaxSpeed(ROTATION_SPEED);
    stepperRotation.setAcceleration(ROTATION_ACCEL);
    
    Serial.println(F("CNC Paint Sprayer Ready"));
    Serial.println(F("Commands: H-Home S-Start E-Stop R-Reset"));
}

void processPattern() {
    if (!motorsRunning) {
        Command* currentPattern;
        int patternSize;
        
     stepperX.setMaxSpeed(X_SPEED);
    stepperX.setAcceleration(X_ACCEL);
    
    stepperY.setMaxSpeed(Y_SPEED);
    stepperY.setAcceleration(Y_ACCEL);

        switch(currentSide) {
            case 0:
                currentPattern = SIDE1_PATTERN;
                patternSize = SIDE1_SIZE;
                break;
            case 1:
                currentPattern = SIDE2_PATTERN;
                patternSize = SIDE2_SIZE;
                break;
            case 2:
                currentPattern = SIDE3_PATTERN;
                patternSize = SIDE3_SIZE;
                break;
            case 3:
                currentPattern = SIDE4_PATTERN;
                patternSize = SIDE4_SIZE;
                break;
        }
        
        if (currentCommand < patternSize) {
            executeCommand(currentPattern[currentCommand]);
            currentCommand++;
        } else {
            currentCommand = 0;
            currentSide++;
            if (currentSide >= 4) {
                systemState = CYCLE_COMPLETE;
            }
        }
    }
}

void loop() {
    xHomeSensor.update();
    yHomeSensor.update();
    
    motorsRunning = stepperX.isRunning() || 
                   stepperY.isRunning() || 
                   stepperRotation.isRunning();
    
    stepperX.run();
    stepperY.run();
    stepperRotation.run();
    
    if (Serial.available()) {
        char cmd = Serial.read();
        switch(cmd) {
            case 'H':
            case 'h':
                if (systemState == IDLE) {
                    systemState = HOMING_X;
                }
                break;
                
            case 'S':
            case 's':
                if (systemState == IDLE) {
                    currentSide = 0;
                    currentCommand = 0;
                    systemState = EXECUTING_PATTERN;
                }
                break;
                
            case 'E':
            case 'e':
                systemState = ERROR;
                digitalWrite(PAINT_RELAY_PIN, HIGH);
                stepperX.stop();
                stepperY.stop();
                stepperRotation.stop();
                break;
                
            case 'R':
            case 'r':
                if (systemState == ERROR) {
                    systemState = IDLE;
                }
                break;
        }
    }
    
    switch(systemState) {
        case IDLE:
            break;
            
        case HOMING_X:
            if (!xHomeSensor.read()) {
                stepperX.setCurrentPosition(0);
                systemState = HOMING_Y;
            } else if (!stepperX.isRunning()) {
                stepperX.moveTo(-1000000);
            }
            break;
            
        case HOMING_Y:
            if (!yHomeSensor.read()) {
                stepperY.setCurrentPosition(0);
                systemState = EXECUTING_PATTERN;
            } else if (!stepperY.isRunning()) {
                stepperY.moveTo(-1000000);
            }
            break;
            
        case EXECUTING_PATTERN:
            processPattern();
            break;
            
        case ERROR:
            break;
            
        case CYCLE_COMPLETE:
            if (!motorsRunning) {
                Serial.println(F("Cycle complete"));
                systemState = IDLE;
            }
            break;
    }
}
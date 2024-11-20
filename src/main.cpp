// # Control Commands for CNC Paint System

// ### Machine Setup ###
// HOME = "home"                    # Return paint head to origin position (0,0)
// PRIME = "prime"                  # Prime paint gun (3-second test spray off-canvas)
// CLEAN = "clean"                  # Flush paint gun (3-second water cycle)

// ### Paint Operations ###
// START_FULL = "start_full"        # Execute complete painting sequence
// RUN_SIDES = "run_sides"          # Paint specific canvas sides (e.g., "13" for sides 1 and 3)

// ### Paint Head Speed ###
// # Adjust movement speed for each canvas side (1-4)
// SPEED_SIDE_1 = "speed_1"         # Set paint head speed for side 1 (value: 1-100%)
// SPEED_SIDE_2 = "speed_2"         # Set paint head speed for side 2 (value: 1-100%)
// SPEED_SIDE_3 = "speed_3"         # Set paint head speed for side 3 (value: 1-100%)
// SPEED_SIDE_4 = "speed_4"         # Set paint head speed for side 4 (value: 1-100%)

// ### System Control ###
// PAUSE = "pause"                  # Halt paint head movement and spray
// EMERGENCY_STOP = "emergency"     # Immediate system shutdown
//                                 # - Stops paint spray
//                                 # - Powers off Arduino
//                                 # - Saves current coordinates
//                                 # - Enables position recovery for auto-resume

// ### Future Integration ###
// WEBSITE_UPDATE = "update"        # Auto-sync painted colors to tracking website
//                                 # (Feature in development)

// # Usage Examples:
// # run_command(HOME)              # Return paint head to starting position
// # run_command(RUN_SIDES, "13")   # Execute painting sequence for sides 1 and 3
// # run_command(SPEED_SIDE_1, 75)  # Set paint head speed to 75% for side 1

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
    HOMED_WAITING,    // New state for waiting after homing
    EXECUTING_PATTERN,
    ERROR,
    CYCLE_COMPLETE
};

// Global Variables
SystemState systemState = IDLE;
bool motorsRunning = false;
int currentSide = 0;
int currentCommand = 0;
bool sidesToPaint[4] = {true, true, true, true}; // Array to track which sides to paint


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
    Serial.println(F("Commands:"));
    Serial.println(F("H - Home"));
    Serial.println(F("S - Start"));
    Serial.println(F("E - Stop"));
    Serial.println(F("R - Reset"));
    Serial.println(F("12/13/14/23/24/34 etc. - Select sides to paint"));
}

void parseSideSelection(String input) {
    // Reset all sides to false
    for (int i = 0; i < 4; i++) {
        sidesToPaint[i] = false;
    }
    
    // Parse each character and set corresponding sides
    for (int i = 0; i < input.length(); i++) {
        int side = input.charAt(i) - '0';
        if (side >= 1 && side <= 4) {
            sidesToPaint[side-1] = true;
        }
    }
}

void processPattern() {



    if (!motorsRunning) {
        // Find next side to paint
        while (currentSide < 4 && !sidesToPaint[currentSide]) {
            currentSide++;
            currentCommand = 0;
        }
        
        if (currentSide >= 4) {
            systemState = CYCLE_COMPLETE;
            return;
        }
        
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
        String input = Serial.readStringUntil('\n');
        input.trim();
        
        if (input.length() == 1) {
            char cmd = input.charAt(0);
            switch(cmd) {
                case 'H':
                case 'h':
                    if (systemState == IDLE) {
                        systemState = HOMING_X;
                    }
                    break;
                    
                case 'S':
                case 's':
                    if (systemState == HOMED_WAITING) {
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
        } else if (input.length() >= 2) {
            // Process side selection
            parseSideSelection(input);
            Serial.print(F("Selected sides to paint: "));
            for (int i = 0; i < 4; i++) {
                if (sidesToPaint[i]) {
                    Serial.print(i + 1);
                    Serial.print(" ");
                }
            }
            Serial.println();
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
                systemState = HOMED_WAITING;  // Changed to new waiting state
                Serial.println(F("Homing complete. Enter 'S' to start painting."));
            } else if (!stepperY.isRunning()) {
                stepperY.moveTo(-1000000);
            }
            break;
            
        case HOMED_WAITING:
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
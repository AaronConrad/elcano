
#include <Common.h>
#include <Matrix.h>
#include <Elcano_Serial.h>
#include <SPI.h>

/*
// Elcano Contol Module C3: Pilot.

The Pilot program reads a serial line that specifies the desired path and speed 
of the vehicle. It computes the analog signals to control the throttle, brakes 
and steering and sends these to C2.

Input will be recieved and sent using the functions writeSerial and readSerial in 
Elcano_Serial. Inputs will be received from C5(sensors) and C4(planner). Output is sent to 
C2(Low Level).

In USARSIM simulation, these could be written on the serial line as
wheel spin speeds and steering angles needed to
follow the planned path. This would take the form of a DRIVE command to the C1 module over a 
serial line. The format of the DRIVE command is documented in the C1 code.

[in] Digital Signal 0: J1 pin 1 (RxD) Serial signal from C4 Path planner, which passes on
data from C6 Navigator
*/

// TargetLocation struct is used to store the data of each TargetLocation of the path given by the planner.
// We create a new struct becuase the SerialData should only be used to send data.
struct TargetLocation
{
   long int targetSpeed;
   long int bearing;
   long int northPos;
   long int eastPos;
};

// Process segement assures that we received a valid TargetLocation and not noise.
// it then stores the data in another struct that holds similar data. This is 
// done for loose coupling. If we need to change the point data stored locally
// we don't need to try to change the elcano serial file.
bool ProcessTargetLocation(TargetLocation *currentTargetLocation, SerialData instructions)
{
  // Each statement checks that the data received is not int_max.
  if(instructions.speed_cmPs == 2147483648)
  {
    return false;
  }
  else if(instructions.posE_cm == 2147483648)
  {
    return false;
  }
  else if(instructions.posN_cm == 2147483648)
  {
    return false;
  }
  else if(instructions.bearing_deg == 2147483648)
  {
    return false;
  }
  // If none of the data was corrupted then we can store it in the TargetLocation struct provided.
  currentTargetLocation->targetSpeed = instructions.speed_cmPs;
  currentTargetLocation->bearing = instructions.bearing_deg;
  currentTargetLocation->eastPos = instructions.posE_cm;
  currentTargetLocation->northPos = instructions.posN_cm;
  return true;
}


bool ReadWaypoints(TargetLocation* TargetLocationArray)
{
  //set up variables
  int count = 0;
  SerialData dataRead;
  TargetLocation currentTargetLocation;
  // loop until we hit the end of the data
   while(true)
   {
    //check if we're done receiveing
    readSerial(&Serial1,&dataRead);
    if(dataRead.number == 789 || count == MAX_WAYPOINTS) // bad number there is no more data or end of data
    {
      if(count == 0) // nothing was read
      {
        return false;
      }
      break;
    }
    else
    //process and store if valid. 
    {
      if(ProcessTargetLocation(&currentTargetLocation, dataRead))
      {
        TargetLocationArray[count] = currentTargetLocation;
        count++;
      }
      // send back acknowledgement
    }
   }
   return true;
}

//this will be the square test of the first autonomous baby step.
void Drive(int myAngle, int myX, int myY, int targetAngle, int targetX, int targetY)
{
  
}

// This function will rotate the bike to the desired angle. 
// This includes calculation of the difference in its current heading and the target 
// angle. Low level commands will be sent to C2 low level controller. 
void RotateToAngle(int targetAngle, int currentHeading)
{
  //We must know full turing angle and lowest speed
  //calculate angle distance and decide which direction to turn

  //Have we reached our target?
  if(targetAngle == currentHeading)
  {
    return;
  }

  //if not we set the steering angle and conitune turning.
  if(ShortestAngle(targetAngle, currentHeading))
  {
  }

  //was is our max steering angle?
  
  //test with turn around twice.   
}

/* The Float Comparison function allows you to compare floats to any X number 
 * of decimal places. This allows us to compare two floats without errors
 * that = comparison will give.
 */

float ShortestAngle(float currentAngle, float targetAngle)
{
     // handle cases of positve past 180 and negative past -180
     // This simplifies our calulations of smallest angle
     currentAngle = UniformAngle(currentAngle);
     targetAngle = UniformAngle(targetAngle);

     //find the lowest angle

     // case of positive positve
     if( currentAngle >= 0 && targetAngle >= 0)
     {
        if(currentAngle > targetAngle)
        {
          return (currentAngle - targetAngle) * -1;
        }
        else
        {
          return (targetAngle - currentAngle);
        }
     }
     
     // case of negative negative
     else if( currentAngle <= 0 && targetAngle <= 0)
     {
        if(currentAngle > targetAngle)
        {
          return (targetAngle - currentAngle);
        }
        else
        {
          return (currentAngle - targetAngle) * -1;
        }
     }
     
     // case of positve negative
     else if( currentAngle >= 0 && targetAngle <= 0)
     {
        float retVal = (-currentAngle + targetAngle);
        if(abs((180 - currentAngle) - (-180 - targetAngle)) < abs(retVal))
        {
          retVal = ((180 - currentAngle) - (-180 - targetAngle));
        }
        return retVal;
     }
     //case of negative positve
     else if( currentAngle <= 0 && targetAngle >= 0)
     {
        float retVal = (-currentAngle + targetAngle);
        if(abs(-(180 + currentAngle) - (180 - targetAngle)) < abs(retVal))
        {
          retVal = (-(180 + currentAngle) - (180 - targetAngle));
        }
        return retVal;
     }
}

// This function converts any angle we are dealing with to be from 0 to 180 and anything
// greater than 180 and less than 0 to be represented as a negative angle. Our circle
// starts with 0 at the top as true north
//             0
//       -90         90
//            180
float UniformAngle(float angle)
{
    if(angle > 180)
     {
        angle = -(360 - angle); 
     }
     if(angle < -180)
     {
        angle = 360 + angle; 
     }
     return angle;
}

// Float comparison allows comparison of floats not using the = operator
// this will return a boolean of the comparison of a and b to the number
// of decimal places defined by places. 
bool FloatComparison(float a, float b, int places)
{
  // values are cast to an integer for = comparison of
  // values.
  int aVal;
  int bVal;
  // each case represents the number of decimal places compared.
  switch(places)
  {
    case 1:
        aVal = a*10;
        bVal = b*10;
        break;
    case 2:
        aVal = a*100;
        bVal = b*100;
        break;
    case 3:
        aVal = a*1000;
        bVal = b*1000;
        break;
    case 4:
        aVal = a*10000;
        bVal = b*10000;
        break;

    default:
        aVal = a;
        bVal = b;
        break;
  }
  // return cases.
  if(aVal == bVal)
  {
    return true;
  }
  else 
  {
    return false;
  }
}


/* This function calculates the angle from the current point to the target point
 * in relation to true north.
 * Quadrants are relative to the current position with y lines pointing true north.
 * for reference quadrants are:
 * 2 1
 * 3 4
 */
float NorthOffset(int currentX, int currentY, int targetX, int targetY)
{
  // quadrant 4
  if ((currentX > targetX) && (currentY > targetY))
  {
    return (-180 + (atan(float(currentX+targetX)/float(currentY+targetY)) * 57.2957795));
  }
  // quadrant 3
  else if((currentX < targetX) && (currentY > targetY))
  {
    return (180 + (atan(float(currentX+targetX)/float(currentY+targetY)) * 57.2957795));
  }
  // quadarant 1 or 2
  else
  {
     return (atan(float(currentX+targetX)/float(currentY+targetY)) * 57.2957795);
  }
}

// Calculate the hypotenuse side of the triangle passed in.
int PothagarianDistance(int currentX, int currentY, int targetX, int targetY)
{
    return sqrt(sq(abs(currentX - targetX)) + sq(abs(currentY - targetY)));
}

// Returns if the two points are with in the target range of eachother.
// Units passed in should be the same for both x, y, and range.
bool ValidRange(float x1,float y1, float x2,float y2, float range)
{
  bool retVal = false;
  float temp = PothagarianDistance(x1,y1,x2,y2);
  if(temp < range)
  {
    retVal = true; 
  }
  return retVal;
}

////////////////////////////////////////////////////////////////////////////
void setup() 
{  
        Serial1.begin(9600); 
        //Serial2.begin(9600);
        //Serial3.begin(9600); 
        pinMode(8,OUTPUT);
}

void loop() 
{
    int steeringAngle = 35;
    int speedSetting = 300;
    // get newest map data from C4 planner
    // Using Elcano_Serial.h Using the SerialData struct in the .h file.
    // Receive a TargetLocation from C4. C4 will only ever send TargetLocations to C3.


    //-----------------------C4 input--------------------------//
    SerialData instructions;
    readSerial(&Serial1, &instructions);
    TargetLocation currentTargetLocation;
    ProcessTargetLocation(&currentTargetLocation,instructions);
    TargetLocation allTargetLocations[MAX_WAYPOINTS];
    ReadWaypoints(allTargetLocations);
    

    //Test of input from C4.
    //Serial.println("test");
    //Serial.println(instructions.kind);
  
    //-----------------------C5 input-------------------------//
    //SerialData sensorData;
    //readSerial(&Serial2, &sensorData);
    

    //---------------------C2 output-------------------------------//

    //Send data to low level.
    //SerialData toLowLevel;
    //toLowLevel.kind = MSG_DRIVE;
    //toLowLevel.angle_deg = steeringAngle;
    //toLowLevel.speed_cmPs = speedSetting;
    //writeSerial(&Serial3, &toLowLevel);

    //Test of output to C2.
    // Outputting to C2 uses the Elcano Serial kind 1 to send a "drive signal to C2"
    // The drive signal includes angle and speed.
    
    //Test Data for instructions C4. This is an example of a semgment
    /*instructions.kind = 4;
    instructions.number = 1;
    instructions.speed_cmPs = 100;
    instructions.bearing_deg = 35;
    instructions.posE_cm = 400;
    instructions.posN_cm = 400;

    //Test Data for C5 sensor input. This is an example of a sensor signal.
    /*
     sensorData.kind = 2;
     sensorData.speedcmPs = 100;
     sensorData.angle_deg = 12;
     sensorData.posE_cm = 50;
     sensorData.posN_cm = 50;
     sensorData.bearing_deg = 15;
    */

    //Test Data for C2 drive output. Example drive commands.
    /*
    toLowLevel.kind = 1;
    toLowLevel.speed_cmPs = 400;
    toLowLevel.angle_deg = 35;
     */
    
    //turning test
    //Drive()
    //{
      
    //}

}



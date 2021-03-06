#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, dgtl1,  elevBottom,     sensorTouch)
#pragma config(Sensor, dgtl2,  autoJumper,     sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           gate,          tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           frontRight,    tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           backRight,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           backLeft,      tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           frontLeft,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           rightLauncher, tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port7,           leftLauncher,  tmotorVex393_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port8,           intakeLower,   tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           intakeUpper,   tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          cuteIntake,    tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

/*+++++++++++++++++++++++++++++++++++++++++++++| Notes |++++++++++++++++++++++++++++++++++++++++++++++
CompAutonomous.c
- Basic driver control program for 2014 VEX omni drive base.
- The left joystick Y-axis controls the robot's forward and backward movement.
- The left joystick X-axis controls the robot's left and right movement.
- The right joystick X-axis controls the robot's rotation.
- Button 5 controls the grabber.
- Button 6 controls the elevator.
[I/O Port]          [Name]              [Type]                [Description]
Motor Port 1
Motor Port 2        frontRight          VEX Motor 393         Front Right motor
Motor Port 3        backRight           VEX Motor 393         Back Right motor
Motor Port 4        frontLeft           VEX Motor 393         Front Left motor
Motor Port 5        backLeft            VEX Motor 393         Back Left motor
Motor Port 6        rightLauncher       VEX Motor 393         Right Launcher primary motor
Motor Port 7        leftLauncher        VEX Motor 393         Left Launcher primary motor
Motor Port 8        intakeLower         VEX Motor 393         Lower intake motor
Motor Port 9				intakeUpper					VEX Motor 393					Upper intake motor
Motor Port 10
----------------------------------------------------------------------------------------------------*/

// Change log:

// 2015-nov-10 DF  Reverted to PID of 10-24, bumped KI up
// 2015-Nov-10 DF  Moved intake controls to operator joysticks

// TODO - Add closed-loop control code for drive motor control.

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)    // This is longer than 15 seconds to include external timer padding.
#pragma userControlDuration(120)  // This is longer than 105 seconds to include external timer padding.

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!
//#include "AutonomousFunctions.c"

#define ONE_POINT_AUTO true

//Global Constants
int const dt = 20;  // number of milliseconds per each control loop
int const maxSteer = 50;  // percent of drive to apply to steering
// int const incDelayAmount = 10; // larger number delays the launcher speed increase

//Global variables
int monitoredMotorSpeedR = 0;
int actualSpeedL = 0;  // to see when shooter wheels are up to speed
int actualSpeedR = 0;

int launcherSpeed = 0;
int lowerSpeed = 0;
int upperSpeed = 0;
int gateSpeed = 0;
int maxLauncherSpeed = 800;  // was 160;
int speedTolerance = 30;    // how much speed can be wrong and let it shoot
int speedStep = 6;  // was 5    rate of speed change when button pressed/released
int per   = 200;    // was 200
int kp    = 5;      // was 6
int ki    = 5;      // was 4
int kd    = 10;     // was 12
int kpden = 100;
int kiden = 100;
int kdden = 100;
//int victory = 0;
int driveSpeed = 0;//The forward drive speed.
int turnCoef = 0;//The turning amount.

/*task victoryDance() {
	motor[frontRight] = 127;
	motor[backRight] = 127;
	motor[frontLeft] = -127;
	motor[backLeft] = -127;
	motor[gate] = 127;
	wait1Msec(500);
	motor[gate] = -127;
	wait1Msec(500);
	motor[gate] = 0;
	motor[frontRight] = 0;
	motor[backRight] = 0;
	motor[frontLeft] = 0;
	motor[backLeft] = 0;
}*/

//CALCODE Modifies the inout to create a linear speed curve.
int linearize(int vel){
	int pwm;
	int linear[129] = {0, 0, 18, 18, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20,
		21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 24, 24, 24, 25, 25, 25,
		25, 26, 26, 26, 26, 27, 27, 27, 27, 28, 28, 29, 29, 29, 29, 30, 30, 30,
		31, 31, 31, 32, 32, 33, 33, 33, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38,
		38, 39, 39, 40, 40, 41, 41, 41, 42, 43, 43, 44, 44, 45, 45, 46, 47, 48,
		49, 49, 50, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 61, 62, 64, 65, 66,
		67, 68, 69, 70, 72, 73, 75, 76, 77, 79, 81, 83, 84, 84, 85, 85, 86, 86,
		87, 87, 88, 90, 96, 105, 105};
	if(vel > 127) {vel = 127;}
	if(vel < -127) {vel = -127;}
	if (vel < 0){
		pwm = -linear[-vel];
		} else {
		pwm = linear[vel];
	}
	return pwm;
}

//CALCODE Implements a deadband so it doesn't move without moving a joystick.
int deadband(int vel) {
	return (abs(vel) < 24) ? 0: vel;
}

//CALCODE Drives the robot.
task Drive_control
{
	while(true)
	{
		if(driveSpeed > 127) {driveSpeed = 127;}
		if(driveSpeed < -127) {driveSpeed = -127;}
		if(turnCoef > 127) {turnCoef = 127;}
		if(turnCoef < -127) {turnCoef = -127;}
		motor[frontLeft]  =  linearize(driveSpeed + (maxSteer*turnCoef/100));
		motor[backLeft]   = -linearize(driveSpeed + (maxSteer*turnCoef/100));
		motor[frontRight] = -linearize(driveSpeed - (maxSteer*turnCoef/100));
		motor[backRight]  =  linearize(driveSpeed - (maxSteer*turnCoef/100));
		// backLeft & frontRight are backwards
		wait1Msec(dt);
	}
}

void drive(int t, int speed, int turn)
{
	motor[frontLeft]  =  linearize(speed + (maxSteer*turn/100));
	motor[backLeft]   = -linearize(speed + (maxSteer*turn/100));
	motor[frontRight] = -linearize(speed - (maxSteer*turn/100));
	motor[backRight]  =  linearize(speed - (maxSteer*turn/100));
	sleep(t);
}

// spin the right shooter wheel at launcherSpeed
task Pid1() {
	int prevPos = 0;
	int currentPos = 0;
	int actualSpeed = 0;  // speed calculated from encoder
	int motorSpeed = 0;   // speed command to motor
	int speedError = 0;
	int prevError = 0;    // last iteration's speed error
	int intError = 0;     // integrated error since last reset
	int difError = 0;
	int desiredSpeed;
	resetMotorEncoder(rightLauncher);
	while (true) {
		sleep(per);

		desiredSpeed = launcherSpeed;  // commanded speed from main loop

		currentPos = nMotorEncoder[rightLauncher];
		actualSpeed = (currentPos - prevPos) * 1000 / per;  // now independent of per 11/15
		actualSpeedR = actualSpeed;   // to let ball into shooter when spun up
		speedError = (desiredSpeed - actualSpeed);
		intError += speedError;
		difError = prevError - speedError;
//		motorSpeed = desiredSpeed + (speedError * kp / kpden) + (difError * kd / kdden) + (intError * ki / kiden) ;
		motorSpeed = (speedError * kp / kpden) + (difError * kd / kdden) + (intError * ki / kiden) ;
		//motorSpeed = 127;  //  test full speed motor
		prevPos = currentPos;
		prevError = speedError;
		if (motorSpeed > 127) {   // limit motor command to 0-127
			motorSpeed = 127;
		}
		if (motorSpeed < 0) {
			motorSpeed = 0;
		}
		monitoredMotorSpeedR = motorSpeed; // for debugging
		motor[rightLauncher] = motorSpeed;
	}
}

task Pid2() {
	int prevPos = 0;
	int currentPos = 0;
	int actualSpeed = 0;
	int motorSpeed = 0;
	int speedError = 0;
	int prevError = 0;
	int intError = 0;
	int difError = 0;
	int desiredSpeed;
	resetMotorEncoder(leftLauncher);
	while (true) {
		sleep(per);

		desiredSpeed = launcherSpeed;  // commanded speed from main loop

		currentPos = nMotorEncoder[leftLauncher];
		actualSpeed = (currentPos - prevPos) * 1000 / per;  // now independent of per 11/15
		actualSpeedL = actualSpeed;   // to let ball into shooter when spun up
		speedError = (desiredSpeed - actualSpeed);
		intError += speedError;
		difError = prevError - speedError;
		motorSpeed = (speedError * kp / kpden) + (difError * kd / kdden) + (intError * ki / kiden) ;
//		motorSpeed = desiredSpeed + (speedError * kp / kpden) + (difError * kd / kdden) + (intError * ki / kiden) ;
		prevPos = currentPos;
		prevError = speedError;
		if (motorSpeed > 127) {
			motorSpeed = 127;
		}
		if (motorSpeed < 0) {
			motorSpeed = 0;
		}
		motor[leftLauncher] = motorSpeed;
//		motor[leftLauncher] = 0;   // disable this one while debugging other motor PID loop!
	}
}

// All activities that occur before the competition starts
// Example: clearing encoders, setting servo positions, ...
void pre_auton()
{
	// Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
	// Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
	bStopTasksBetweenModes = true;

}

// Task for the autonomous portion of the competition.
task autonomous()
{
	drive(2000, 100, 0); // drive forward from starting tile past ball stacks
	drive(500, 100, -50); // steer left around stack
	drive(1000, 100, 0); // move forward
	drive(500, 100, 50); // turn to face net
	drive(1000, 100, 0); // go to net
	drive(1,0,0); // stop motors
}

// Task for the driver controlled portion of the competition.
task usercontrol()
{
	startTask(Pid1);
	startTask(Pid2);
	startTask(Drive_control);


	while (true)
	{
		// Drive control
		driveSpeed = deadband(vexRT[Ch3]);
		turnCoef = deadband(vexRT[Ch1]);

		// victory dance for when victory is achieved
		/*if (vexRT(Btn7L) == 1) {
			if (victory == 0) {
				startTask(victoryDance);
				victory = 1;
			}
		} else {
			victory = 0;
		}*/

		sleep(20);

		// Intake control
		upperSpeed = 0;
		lowerSpeed = 0;
		gateSpeed = 0;
//		if (vexRT[Btn6U] == 1) {  // run both intake motors in when button 6 up pressed
		if (vexRT[Ch2Xmtr2] > 50) {  // run both intake motors in when operator right stick up
			lowerSpeed = -127;
			// check if shooter wheels are spinning at the right speed
			if ((abs(actualSpeedL - maxLauncherSpeed) < speedTolerance) &&
		      (abs(actualSpeedR - maxLauncherSpeed) < speedTolerance)) {
				upperSpeed = -127;
			}
//		} else if (vexRT[Btn6D] == 1){ // both intake motors out when button 6D pressed
		} else if (vexRT[Ch2Xmtr2] < -50){ // both intake motors out when operator right stick down
			lowerSpeed = 127;
			upperSpeed = 127;
		}
		if (vexRT[Btn8UXmtr2] == 1){
			gateSpeed = 127;
		}
		if (vexRT[Btn8DXmtr2] == 1){
			gateSpeed = -127;
		}

		motor[gate] = gateSpeed;

		// Individual intake control
//		if (vexRT[Btn5U] == 1) {
		if (vexRT[Ch3Xmtr2] > 50) {
			upperSpeed = -127;  // upper intake runs in when operator left joystick up
//		} else if (vexRT[Btn5D] == 1) {
		} else if (vexRT[Ch3Xmtr2] < -50) {
			lowerSpeed = 127;  // lower intake runs out when operator left joystick down
		}
		motor[intakeLower] = lowerSpeed;
		motor[intakeUpper] = upperSpeed;
		motor[cuteIntake] = lowerSpeed;

		// Launcher
		// NEW CODE:
		// Attempting slower incrementing of launcherspeed by requiring the button to remain pressed for incDelay times
    // Previously the increment was as fast the userloop goes
		if (vexRT[Btn6UXmtr2] == 1 && launcherSpeed < maxLauncherSpeed) {
	//		incDelay--;
	//		if (incDelay <= 0) { // this slows down the increase of the launcher speed
				launcherSpeed = launcherSpeed + speedStep; // increase the launcher speed
	//			incDelay = incDelayAmount; // launcher speed was changed, reset delay
	//	  }
		} else if (launcherSpeed > 0 && vexRT[Btn6UXmtr2] == 0) {
				launcherSpeed = launcherSpeed - speedStep; // decrease the launcher speed
		}
		//motor[leftLauncher] = launcherSpeed;
		//motor[rightLauncher] = launcherSpeed;
		// now done by pid loops
	}
}

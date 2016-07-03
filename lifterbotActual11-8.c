#pragma config(Sensor, dgtl1,  HappyLed,       sensorLEDtoVCC)
#pragma config(Sensor, dgtl2,  catapultCocked, sensorTouch)
#pragma config(Motor,  port1,           liftLeft,      tmotorVex393_HBridge, openLoop, reversed, driveLeft)
#pragma config(Motor,  port2,           frontRight,    tmotorVex393_MC29, openLoop, reversed, driveRight)
#pragma config(Motor,  port3,           frontLeft,     tmotorVex393_MC29, openLoop, reversed, driveLeft)
#pragma config(Motor,  port4,           backLeft,      tmotorVex393_MC29, openLoop, driveLeft)
#pragma config(Motor,  port5,           backRight,     tmotorVex393_MC29, openLoop, reversed, driveRight)
#pragma config(Motor,  port6,           catapultRight, tmotorVex393_MC29, openLoop, driveRight)
#pragma config(Motor,  port7,           catapultLeft,  tmotorVex393_MC29, openLoop, reversed, driveLeft)
#pragma config(Motor,  port8,           rampLeft,      tmotorVex393_MC29, openLoop, reversed, driveLeft)
#pragma config(Motor,  port9,           rampRight,     tmotorVex393_MC29, openLoop, driveRight)
#pragma config(Motor,  port10,          liftRight,     tmotorVex393_HBridge, openLoop, driveRight)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

// Code to run the Bit Buckets Nothing but Net lifter robot
// Designed for straight-ish driving. has catapult and lifter mechs.

// 2015-Nov-8 DF  Adding lift and ramp deployment motors

#pragma platform(VEX)  // tell it not to assume vex iq

//Global Constants
int const dt = 20;  // number of milliseconds per each control loop
int const maxSteer = 50;  // percent of drive to apply to steering

//Global Variables
int driveSpeed = 0;//The forward drive speed.
int turnCoef = 0;//The turning amount.

//Functions;

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

//CALCODE Limits the acceleration I think...........
int acc_limit(int input, int old, int max_acc)
{
	if ((input - old) > max_acc) {input += max_acc;}
	if ((input - old) < -max_acc) {input -= max_acc;}
	return (input);
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
		wait1Msec(dt);
	}
}
// work on the ramps and lifting stuff
task liftPlatform
{
	while (true)
	{
		if (vexRT[Btn6U] == 1)  // lift the ramps up or down
		{
			motor[liftRight] = 127;
			motor[liftLeft] = 127;
		} else if (vexRT[Btn6D] == 1)
		{
			motor[liftRight] = -127;
			motor[liftLeft] = -127;
		} else {
			motor[liftRight] = 0;
			motor[liftLeft] = 0;
		}
		if (vexRT[Btn8U] == 1)    // deploy the ramps
		{
			motor[rampRight] = 80;  // not too fast
			motor[rampLeft] = 80;
		} else if (vexRT[Btn8D] == 1)
		{
			motor[rampRight] = -40;  // extra slow and careful
			motor[rampLeft] = -40;
		} else {
			motor[rampRight] = 0;
			motor[rampLeft] = 0;
		}
		sleep(10);
	}
}

task catapult
{
	while (true)
	{
		if (SensorValue[catapultCocked] == 0 || vexRT[Btn5U] == 1)
		{
			motor[catapultRight] = 127;
			motor[catapultLeft] = 127;
		} else {
			motor[catapultRight] = 0;
			motor[catapultLeft] = 0;
		}
		sleep(1);
	}
}

//Called to run the robot.
task main()
{
	startTask (Drive_control);
	startTask (catapult);
	startTask (liftPlatform);

	while (true)
	{
		driveSpeed = deadband(vexRT[Ch3]);
		turnCoef = deadband(vexRT[Ch1]);

		wait1Msec(dt);
	}
}

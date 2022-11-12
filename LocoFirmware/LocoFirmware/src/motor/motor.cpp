/**
 * Init DC motor
 * if passphrase is set the most secure supported mode will be automatically selected
 * @param motorDirectionPin   int pin to control direction    
 * @param motorPWMpin         int pin to cobtrol speed with PWM
 * @param pwmChannel          PWM channel to use
 * @param frequency           Optional. PWM Frequency
 * @param resolution          Optional. PWM Resolution
 * @return
 */

#include <Arduino.h>
#include "motor.h"

void hwconf(int directionPin, int PwmPin, int PwmChannel, int frequency, int resolution);

motor_status_t Motor::begin(int directionPin, int PWMpin, int pwmChannel, int frequency, int resolution)
{
  //Motor setup
  motorDirectionPin = directionPin;
  motorPwmPin = PWMpin;
  motorPwmChannel = pwmChannel;
  motorFrequency = frequency;
  motorResolution = resolution;
  motorDirection = MOTOR_FORWARD_DIR;
  motorSpeed = 0;

  hwconf(motorDirectionPin, motorPwmPin, motorPwmChannel, motorFrequency, motorResolution);
  motorStatus = MOTOR_INIT_STATE;

  return motorStatus;
}
//We need at least motor pin and pwm pin. We can assume PWM channel, frequency and resolution.
motor_status_t Motor::begin(int DirectionPin, int PWMpin)
{
  //Motor setup
  motorDirectionPin = motorDirectionPin;
  motorPwmPin = PWMpin;
  motorPwmChannel = MOTOR_DEFAULT_PWM_CHANNEL;
  motorFrequency = MOTOR_DEFAULT_PWM_FREQ;
  motorResolution = MOTOR_DEFAULT_PWM_RESOLUTION;
  motorDirection = MOTOR_FORWARD_DIR;
  motorSpeed = 0;

  //Motor setup
  hwconf(motorDirectionPin, motorPwmPin, motorPwmChannel, motorFrequency, motorResolution);
  motorStatus = MOTOR_INIT_STATE;

  return motorStatus;
}

bool Motor::enable()
{
    motorStatus = MOTOR_RUNNING;
    return true;
};

bool Motor::disable()
{
    motorStatus = MOTOR_STOP;
    return true;
};

bool Motor::set_direction(motor_direction_t directionToSet)
{
    motorDirection = directionToSet;
    return true;
};

bool Motor::set_speed(unsigned int speedToSet)
{
    motorSpeed = speedToSet; 
    return true;
};

motor_status_t Motor::update()
{
  if (motorStatus == MOTOR_RUNNING)
  {
    digitalWrite(motorDirectionPin, bool(motorDirection));
    ledcWrite(motorPwmChannel, motorSpeed); 
  }
  else
  {
    ledcWrite(motorPwmChannel, 0); 
  }
return motorStatus;
}

void hwconf(int directionPin, int PwmPin, int PwmChannel, int frequency, int resolution)
{
  pinMode(directionPin, OUTPUT);
  pinMode(PwmPin, OUTPUT);
  // configure LED PWM lib to use with motor speed
  ledcSetup(PwmChannel, frequency, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PwmPin, PwmChannel);
}
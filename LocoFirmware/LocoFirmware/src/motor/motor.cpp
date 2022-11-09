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
#include <motor.h>

static hwconf(motorconf *motor_config_t);

motor_status_t motor::begin(int motorDirectionPin, int motorPWMpin, int pwmChannel, int frequency, int resolution)
{
  //Motor setup
  motor_config.directionPin = motorDirectionPin;
  motor_config.pwmPin = motorPWMpin;
  motor_config.pwmChannel = pwmChannel;
  motor_config.frequency = frequency;
  motor_config.resolution = resolution;
  motor_config.motor_direction = MOTOR_FORWARD_DIR;
  motor_config.motor_speed = 0;

  hwconf(&motorconf);
  motor_config.motor_status = MOTOR_INIT_STATE;

  return motor_config.motor_status;
}
//We need at least motor pin and pwm pin. We can assume PWM channel, frequency and resolution.
motor_status_t motor::begin(int motorDirectionPin, int motorPWMpin)
{
  //Motor setup
  motor_config.directionPin = motorDirectionPin;
  motor_config.pwmPin = motorPWMpin;
  motor_config.pwmChannel = MOTOR_DEFAULT_PWM_CHANNEL;
  motor_config.frequency = MOTOR_DEFAULT_PWM_FREQ;
  motor_config.resolution = MOTOR_DEFAULT_PWM_RESOLUTION;
  motor_config.motor_direction = MOTOR_FORWARD_DIR;
  motor_config.motor_speed = 0;

  //Motor setup
  hwconf(&motorconf);
  motor_config.motor_status = MOTOR_INIT_STATE;

  return motor_config.motor_status;
}

bool motor::enable()
{
    motor_config.motor_status = MOTOR_RUNNING;
    return true;
};

bool motor::disable()
{
    motor_config.motor_status = MOTOR_STOP;
    return true;
};

bool motor::set_direction(motor_direction_t direction)
{
    motor_config.motor_direction = direction;
    return true;
};

bool motor::set_speed(unsigned int speed)
{
    motor_config.motor_speed = speed; 
    return true;
};

motor_status_t update()
{
  if (motor_config.motor_status == MOTOR_RUNNING)
  {
    digitalWrite(motor_conf->directionPin, motor_conf->motor_direction);
    ledcWrite(motor_conf->pwmChannel, motor_conf->motor_speed); 
  }
  else
  {
    ledcWrite(motor_conf->pwmChannel, 0); 
  }
};

static hwconf(motorconf *motor_config_t);
{
  pinMode(motorconf->directionPin, OUTPUT);
  pinMode(motorconf->pwmPin, OUTPUT);
  // configure LED PWM lib to use with motor speed
  ledcSetup(motorconf->pwmChannel, motorconf->frequency, motorconf->resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(motorconf->pwmPin, motorconf->pwmChannel);
}
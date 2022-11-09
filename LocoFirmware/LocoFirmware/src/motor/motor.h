/*
 ESP8266WiFiSTA.h - esp8266 Wifi support.
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#define MOTOR_DEFAULT_PWM_CHANNEL    = 0u
#define MOTOR_DEFAULT_PWM_FREQ       = 8000u
#define MOTOR_DEFAULT_PWM_RESOLUTION = 256u

typedef enum {
    MOTOR_NOT_CONFIGURED = 255,
    MOTOR_INIT_STATE     = 0,
    MOTOR_RUNNING        = 1,
    MOTOR_STOP           = 2,
} motor_status_t;

typedef enum {
    MOTOR_FORWARD_DIR    = 0,
    MOTOR_BACK_DIR       = 1,
} motor_direction_t;

class motor
{
public:

    motor_status_t begin(int motorDirectionPin, int motorPWMpin, int pwmChannel, int frequency, int resolution);
    motor_status_t begin(int motorDirectionPin, int motorPWMpin);

    bool enable();
    bool disable();
    bool set_direction(motor_direction_t direction);
    bool set_speed(unsigned int speed);
    motor_status_t update();

protected:
    /* Do we need this protected? Currently we do not use feedback from the motor, and this data represents only values set by the module */
    struct motor_config{
        int directionPin = 0;
        int pwmPin = 0;
        int pwmChannel = MOTOR_DEFAULT_PWM_CHANNEL;
        int frequency = MOTOR_DEFAULT_PWM_FREQ;
        int resolution = MOTOR_DEFAULT_PWM_RESOLUTION;
        motor_status_t motor_status = MOTOR_NOT_CONFIGURED;
        motor_direction_t motor_direction = MOTOR_FORWARD_DIR;
        int motor_speed = 0;
    };
};

#endif

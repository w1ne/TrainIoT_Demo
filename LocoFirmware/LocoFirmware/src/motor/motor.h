#ifndef MOTOR_H_
#define MOTOR_H_

#define MOTOR_DEFAULT_PWM_CHANNEL    0u
#define MOTOR_DEFAULT_PWM_FREQ       8000u
#define MOTOR_DEFAULT_PWM_RESOLUTION 256u

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

class Motor
{
public:

    motor_status_t begin(int directionPin, int PWMpin, int pwmChannel, int frequency, int resolution);
    motor_status_t begin(int directionPin, int PWMpin);

    bool enable();
    bool disable();
    bool set_direction(motor_direction_t direction);
    bool set_speed(unsigned int speed);
    motor_status_t update();

private:
    int motorDirectionPin = 0;
    int motorPwmPin = 0;
    int motorPwmChannel = MOTOR_DEFAULT_PWM_CHANNEL;
    int motorFrequency = MOTOR_DEFAULT_PWM_FREQ;
    int motorResolution = MOTOR_DEFAULT_PWM_RESOLUTION;
    motor_status_t motorStatus = MOTOR_NOT_CONFIGURED;
    motor_direction_t motorDirection = MOTOR_FORWARD_DIR;
    int motorSpeed = 0;
};

#endif

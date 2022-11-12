
#ifndef WEBSERVER_H_
#define WEBSERVER_H_


void webserver_getMotor(int *dcspeed, int *dcdirection, bool *dcswitch);
void webserver_setPower(int power);


#endif
#ifndef _SERIAL_H 
#define _SERIAL_H 
#include <iostream>
#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"
#include <vector>
using namespace itas109;
using namespace std;


extern CSerialPort sp;


void search_port();
void send_code(const char *name, uint8_t d);






#endif	// _SERIAL_H

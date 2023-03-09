#include <iostream>
#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"
#include <vector>
using namespace itas109;
using namespace std;


CSerialPort sp;


void search_port();
void send_code(const char *name, uint8_t d);
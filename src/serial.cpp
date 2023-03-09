#include "serial.hpp"


void search_port()
{
    //mac m1 /dev/cu.usbserial-2140

    std::cout << "Version : " << sp.getVersion() << std::endl << std::endl;

    vector<SerialPortInfo> m_availablePortsList = CSerialPortInfo::availablePortInfos();
    if (0 == m_availablePortsList.size())
    {
        std::cout << "No valid port" << std::endl;
        return 0;
    }
    std::cout << "found port : ";
    for (auto i = m_availablePortsList.begin(); i != m_availablePortsList.end(); i++)
    {
        std::cout << i->portName << "\t";
    }
    std::cout << std::endl; 

}


void send_code(const char *name, uint8_t d)
{
    sp.init(name, // windows:COM1 Linux:/dev/ttyS0
            itas109::BaudRate115200, 
            itas109::ParityNone, 
            itas109::DataBits8, 
            itas109::StopOne);

    sp.open();

    if (sp.isOpen())
    {
        std::cout << "open " << m_availablePortsList[0].portName << " success" << std::endl;
    }
    else
    {
        std::cout << "open " << m_availablePortsList[0].portName << " failed" << std::endl;
        return 0;
    }
    // 写入数据
    uint8_t code[1] = {d};
    sp.writeData(code, 1);

    sp.close();

}

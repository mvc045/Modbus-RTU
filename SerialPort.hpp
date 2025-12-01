//
//  SerialPort.hpp
//  Parking
//
//  Created by Михаил Конюхов on 01.12.2025.
//

#ifndef SerialPort_hpp
#define SerialPort_hpp

#include <stdio.h>
#include <string>

// Драйвер для шлагбаума
class SerialPort {
private:
    int fileDescriptor;
    bool isConnect;

public:
    SerialPort();
    ~SerialPort();
    
    bool connect(const std::string& portName);
    void disconnect();
    bool sendData(const std::string& data);
};

#endif /* SerialPort_hpp */

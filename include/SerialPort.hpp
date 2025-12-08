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
#include <vector>
#include <mutex>
#include "ICommunication.h"

using namespace std;

class SerialPort: public ICommunication {
private:
    mutex portMutex;
    int fileDescriptor;
    bool isConnect;
public:
    SerialPort();
    ~SerialPort() override;

    bool connect(const std::string& address) override;
    void disconnect() override;
    bool sendBytes(const std::vector<uint8_t>& data) override;
    int readBytes(std::vector<uint8_t>& buffer, int expected, int timeout) override;
    void flush() override;
};

#endif /* SerialPort_hpp */

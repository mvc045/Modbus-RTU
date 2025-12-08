//
//  GateController.hpp
//  Parking
//
//  Created by Михаил Конюхов on 03.12.2025.
//

#ifndef GateController_hpp
#define GateController_hpp

#include <stdio.h>
#include "SerialPort.hpp"
#include "ICommunication.h"
#include <unistd.h>

using namespace std;

class GateController {
private:
    ICommunication& port;
    uint8_t deviceId;
public:
    GateController(ICommunication& channel, uint8_t id) : port(channel), deviceId(id) {}
    
    void openGate();
    void waitForOpen();
    bool isGateOpen();
    
    void closeGate();
    void waitForClose();
    bool isGateClose();
};
#endif /* GateController_hpp */

//
//  main.cpp
//  Parking
//
//  Created by Михаил Конюхов on 01.12.2025.
//

#include "SerialPort.hpp"
#include "GateController.hpp"
#include "ModbusUtils.hpp"
#include "ConfigLoader.hpp"
#include "Database.hpp"
#include <iostream>
#include <unistd.h>
#include "Middleware.hpp"
#include "RfidReader.hpp"

// HTTP сервер
#include "HTTPServer.hpp"
#include "httplib.h"
// Парсим json
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

// Путь до файла с конфигурацией
string pathConfig = "/Users/mvc/Documents/C++/SysCalls/Parking/config.txt";
ConfigLoader config;

// cmake -G Xcode -B build_xcode

int main(int argc, const char * argv[]) {
    // Загружаем конфиг
    if (!config.load(pathConfig)) {
        cout << "Файл не найден\n";
    }
    
    int deviceId = config.getInt("barrier_id");
    string portName = config.getString("serial_port");
    
    SerialPort port;
    
    if(!port.connect(portName)) {
        cerr << "Ошибка инициализации контроллера управления шлагбаумом.\n";
        return 1;
    }
    
    port.flush();
    
    GateController gateController(port, static_cast<uint8_t>(deviceId));
    
    Database db("parking_01.db");
    
    // Настраиваем логирование
    gateController.setLogger([&db, deviceId](string type, string msg) {
        db.logEvent(type, msg, deviceId);
        cout << "[" << type << "] " << msg << "\n";
    });
    
    // RFID
    string portRFID = config.getString("rfid_port");
    
    RfidReader rfid;
    
    if (rfid.connect(portRFID)) {
        cout << "[RFID] Подключились к считывателю: " << portRFID << "\n";
    }
    
    rfid.setCallBack([&](string cardCode) {
        cout << "[RFID] Сканируем карту: " << cardCode << "\n";
        
        if (db.checkAccessRFID(cardCode)) {
            cout << "[RFID] Доступ открыт \n";
            db.logEvent("RFID", string("Открыт доступ для: ") + cardCode, 0);
            
            try {
                // true - нужно автозакрытие по таймауту
                gateController.openGate(true);
            } catch (const exception& e) {
                cerr << "Ошибка при открытии ворот: " << e.what() << "\n";
            }
            
        } else {
            cout << "[RFID] Ошибка при сканировании \n";
        }

    });
    
    rfid.start();
    
    // HTTP
    string apiKey = config.getString("api_key");
    int portHTTP = config.getInt("port_http");
    
    HTTPServer httpServer(gateController, db, apiKey);
    httpServer.start(portHTTP);
    
    return 0;
}

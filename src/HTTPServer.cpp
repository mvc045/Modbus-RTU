//
//  HTTPServer.cpp
//  Parking
//
//  Created by Михаил Конюхов on 21.12.2025.
//

#include "HTTPServer.hpp"
#include "ConfigLoader.hpp"
#include <iostream>

using namespace std;
using json = nlohmann::json;


HTTPServer::HTTPServer(GateController& gc, Database& db, const string& apiKey): gateController(gc), db(db), apiKey(apiKey)
{
    setupRoutes();
}

void HTTPServer::start(int port) {
    cout << "HTTP server run ... \n";
    svr.listen("0.0.0.0", port);
}

void HTTPServer::setupRoutes() {
    // Загружаем конфиг
    string pathConfig = "/Users/mvc/Documents/C++/SysCalls/Parking/config.txt";
    ConfigLoader config;
    if (!config.load(pathConfig)) {
        cout << "Файл не найден\n";
    }
    
    int deviceId = config.getInt("barrier_id");
    
    // Публичные ручки
    
    svr.Get("/history", [&](const httplib::Request& req, httplib::Response& res) {
        json history = db.getHistory();
        res.set_content(history.dump(4), "application/json");
    });
    
    svr.Get("/status", [&](const httplib::Request& req, httplib::Response& res) {
        bool isOpen = gateController.isGateOpen();
        
        json response;
        response["device_id"] = deviceId;
        response["status"] = isOpen ? "open" : "closed";
        response["timestamp"] = time(nullptr);
        res.set_content(response.dump(), "application/json");
    });
    
    // Приватные ручки
    
    svr.Post("/users_rfid", Middleware::withAuth(apiKey, [&](const httplib::Request& req, httplib::Response& res) {
        json response;
        
        try {
            if (req.body.empty()) {
                throw runtime_error("Пустой Body");
            }
            
            json body = json::parse(req.body);
            
            // Обязательные поля
            if (!body.contains("username") || !body.contains("card_code")) {
                throw runtime_error("Не заполнены обязательные поля");
            }
            
            string username = body["username"].get<string>();
            string cardCode = body["card_code"].get<string>();
            
            RFIDCardCreationResult result = db.createRFIDCard(username, cardCode);
            
            if (result == RFIDCardCreationResult::ErrorNameExists) {
                throw runtime_error("Такой username уже существует");
            }
            
            if (result == RFIDCardCreationResult::ErrorCodeExists) {
                throw runtime_error("Такой RFID ключ уже сущетсвует");
            }
            
            response["ok"] = true;
            response["timestamp"] = time(nullptr);
            
            res.set_content(response.dump(), "application/json");
        } catch (const exception& e) {
            // Отлавливаем ошибки
            response["ok"] = false;
            response["message"] = e.what();
                        
            res.status = 500;
            res.set_content(response.dump(), "application/json");
        }
    }));
    
    svr.Post("/open", Middleware::withAuth(apiKey, [&](const httplib::Request& req, httplib::Response& res) {
        json response;
        
        try {
            gateController.openGate();
            response["ok"] = true;
            response["timestamp"] = time(nullptr);
            
            res.set_content(response.dump(), "application/json");
        } catch (const exception& e) {
            // Отлавливаем ошибки
            response["ok"] = false;
            response["message"] = e.what();
                        
            res.status = 500;
            res.set_content(response.dump(), "application/json");
        }
    }));
    
    svr.Post("/close", Middleware::withAuth(apiKey, [&](const httplib::Request& req, httplib::Response& res) {
        json response;
        
        try {
            gateController.closeGate();
            
            response["ok"] = true;
            response["timestamp"] = time(nullptr);
                        
            res.set_content(response.dump(), "application/json");
        } catch (const exception& e) {
            response["ok"] = false;
            response["message"] = e.what();
            
            res.status = 500;
            res.set_content(response.dump(), "application/json");
        }
    }));
    
}

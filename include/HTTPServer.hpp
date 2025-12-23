//
//  HTTPServer.hpp
//  Parking
//
//  Created by Михаил Конюхов on 21.12.2025.
//

#ifndef HTTPServer_hpp
#define HTTPServer_hpp

#include <stdio.h>
#include "httplib.h"
#include "json.hpp"
#include "GateController.hpp"
#include "Database.hpp"
#include "Middleware.hpp"

using namespace std;

class HTTPServer {
private:
    httplib::Server svr;
    GateController& gateController;
    Database& db;
    string apiKey;
    void setupRoutes();
public:
    HTTPServer(GateController& gc, Database& db, const string& apiKey);
    void start(int port);
};

#endif /* HTTPServer_hpp */

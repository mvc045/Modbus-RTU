//
//  Database.hpp
//  Parking
//
//  Created by Михаил Конюхов on 08.12.2025.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <sqlite3.h>
#include "json.hpp"

using namespace std;

enum RFIDCardCreationResult {
    Success,
    ErrorNameExists,
    ErrorCodeExists,
    Error
};

class Database {
private:
    sqlite3* db;
    string path;
public:
    Database(const string& pathDb);
    ~Database();
    
    void logEvent(const string& type, const string& message, const int& deviceId);
    string getCurrentTime();
    nlohmann::json getHistory();
    bool checkAccessRFID(const string& cardCode);
    RFIDCardCreationResult createRFIDCard(const string& username, const string& cardCode);
    
};

#endif /* Database_hpp */

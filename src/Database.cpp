//
//  Database.cpp
//  Parking
//
//  Created by Михаил Конюхов on 08.12.2025.
//

#include "Database.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;
using json = nlohmann::json;

Database::Database(const string& path): path(path), db(nullptr) {
    
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        cerr << "[DB] Ошибка подключения";
    } else {
        cout << "[DB] База данных: " << path << "\n";
    }
    
    // Создаем таблицу если ее нет
    // Запрос:
    const char* sql = "CREATE TABLE IF NOT EXISTS history ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "timestamp TEXT, "
            "type TEXT, "
            "message TEXT, "
            "device_id INTEGER);";
    
    char* errMsg = nullptr;
    
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        cerr << "[DB] Не получилось создать таблицу history ...";
        sqlite3_free(errMsg);
    }
    
    // Создаем таблицу с пользователями
    const char* sqlUsers =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "card_code TEXT UNIQUE, " // Уникальный номер карты
        "is_active INTEGER DEFAULT 1);"; // 1 - активен, 0 - заблокирован
    
    char* errMsgUsers = nullptr;
    int rcUsers = sqlite3_exec(db, sqlUsers, 0, 0, &errMsgUsers);
    
    if (rcUsers != SQLITE_OK) {
        cerr << "[DB] Не получилось создать таблицу users ...";
        sqlite3_free(errMsgUsers);
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
        cout << "[DB] Отключились от базы\n";
    }
}

void Database::logEvent(const string& type, const string& message, const int& deviceId) {
    if (!db) return;
    
    stringstream ss;
    ss << 
        "INSERT INTO history (timestamp, type, message, device_id) VALUES ('"
        << getCurrentTime() << "', '"
        << type << "', '"
        << message << "', '"
        << deviceId << "');";
    
    string sql = ss.str();
    char* errMsg = nullptr;
    
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "[DB] Ошибка при записи события";
        sqlite3_free(errMsg);
    }
}

json Database::getHistory() {
    json historyArray = json::array();
    
    if (!db) return historyArray;
    
    string sql = "SELECT id, timestamp, type, message, device_id FROM history ORDER BY id DESC LIMIT 50;";
    
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK) {
        cerr << "[DB] Select Error: " << sqlite3_errmsg(db) << "\n";
        return historyArray;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        json item;
        
        item["id"] = sqlite3_column_int(stmt, 0);
        item["device_id"] = sqlite3_column_int(stmt, 4);
        item["type"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item["message"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item["created_at"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        
        historyArray.push_back(item);
    }
    
    sqlite3_finalize(stmt);
    
    return historyArray;
}

string Database::getCurrentTime() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool Database::checkAccessRFID(const string& cardCode) {
    if (!db) return false;
    
    string sql = "SELECT count(*) FROM users WHERE card_code = '" + cardCode + "' AND is_active = 1;";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    bool access = false;
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        access = (count > 0);
    }
    sqlite3_finalize(stmt);
    return access;
}

bool exists(sqlite3* db, const string& query) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) != SQLITE_OK) {
        return false;
    }
    bool found = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return found;
}


RFIDCardCreationResult Database::createRFIDCard(const string& username, const string& cardCode) {
    if (!db) return RFIDCardCreationResult::Error;
    
    string sqlCheckName = "SELECT 1 FROM users WHERE name = '" + username + "' LIMIT 1;";
    if (exists(db, sqlCheckName)) {
        return RFIDCardCreationResult::ErrorNameExists;
    }
    
    string sqlCheckCard = "SELECT 1 FROM users WHERE card_code = '" + cardCode + "' LIMIT 1;";
    if (exists(db, sqlCheckCard)) {
        return RFIDCardCreationResult::ErrorCodeExists;
    }
    
    stringstream ss;
    ss <<
        "INSERT OR IGNORE INTO users (name, card_code) VALUES ('"
        << username << "', '"
        << cardCode << "');";
    
    string sql = ss.str();
    char* errMsg = nullptr;
    
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "[DB] Ошибка при записи события";
        sqlite3_free(errMsg);
        return RFIDCardCreationResult::Error;
    }
    
    return RFIDCardCreationResult::Success;
}


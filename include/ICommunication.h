//
//  ICommunication.h
//  SysCalls
//
//  Created by Михаил Конюхов on 08.12.2025.
//

#ifndef ICommunication_h
#define ICommunication_h

using namespace std;

class ICommunication {
public:
    virtual ~ICommunication() = default;
    virtual bool connect(const string& address) = 0;
    virtual void disconnect() = 0;
    virtual bool sendBytes(const vector<uint8_t>& data) = 0;
    virtual int readBytes(vector<uint8_t>& buffer, int exprected, int timeout) = 0;
    /// Очистка канала
    virtual void flush() = 0;
};


#endif /* ICommunication_h */

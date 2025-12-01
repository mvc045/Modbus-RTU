//
//  SerialPort.cpp
//  Parking
//
//  Created by Михаил Конюхов on 01.12.2025.
//

#include "SerialPort.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <errno.h>
#include <termios.h> // POSIX terminal control
#include <unistd.h>

using namespace std;

SerialPort::SerialPort() : fileDescriptor(-1), isConnect(false) {}

// Порт закроется когда объект удалится из памяти
SerialPort::~SerialPort() {
    
}

bool SerialPort::connect(const std::string& portName) {
    
    // Открываем порт как обычный файл
    // O_RDWR - чтение и запись
    // O_NOCTTY - порт не станет управляющим терминалом процесса
    // O_NDELAY - не ждать DCD линии (важно для эмуляторов)
    fileDescriptor = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (fileDescriptor == -1) {
        // Не смогли подключиться
        return false;
    }
    
    struct termios tty;
    
    if (tcgetattr(fileDescriptor, &tty) != 0) {
        // Тож ошибка
        return false;
    }
    
    // Скорость input и output
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);
    
    // 8 бит данных
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    
    // Отключаем программное управление потоком
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    // Отключаем обработку спецсимволов (наример Ctrl+C)
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    if (tcsetattr(fileDescriptor, TCSANOW, &tty) != 0) {
        // Ошибка настройки почему то не принялись
        return false;
    }
    
    isConnect = true;
    // Снимаем блокировку
    fcntl(fileDescriptor, F_SETFL, 0);
    
    std::cout << "Успешно подключились к: " << portName << std::endl;
    return true;
}

void SerialPort::disconnect() {
    if (isConnect && fileDescriptor != -1) {
        close(fileDescriptor);
        isConnect = false;
        std::cout << "Отключились." << std::endl;
    }
}

bool SerialPort::sendData(const string& data) {
    if (!isConnect) {
        return false;
    }
    
    int bytesWritten = write(fileDescriptor, data.c_str(), data.length());
    
    if (bytesWritten < 0) {
        cerr << "Ошибка записи: " << strerror(errno) << endl;
        return false;
    }
    
    return true;
}

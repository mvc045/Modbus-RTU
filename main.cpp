//
//  main.cpp
//  Parking
//
//  Created by Михаил Конюхов on 01.12.2025.
//

#include "SerialPort.hpp"
#include <iostream>
#include <unistd.h>

using namespace std;

enum class GateCommand: char {
    Open = 'O',
    Close = 'C'
};

class GateController {
private:
    SerialPort port;
public:
    bool init(const string& devicePath) {
        return port.connect(devicePath);
    }
    
    void openGate() {
        cout << "[Controller] отправили команду на открытие\n";
        string command(1, static_cast<char>(GateCommand::Open));
        port.sendData(command);
    }
    
    void closeGate() {
        cout << "[Controller] отправили команду на закрытие\n";
        string command(1, static_cast<char>(GateCommand::Close));
        port.sendData(command);
    }
};

class Car {
private:
    string number;
    string owner;
    string phone;
public:
    Car(string numberAuto, string ownerAuto, string phoneOwner) {
        number = numberAuto;
        owner = ownerAuto;
        phone = phoneOwner;
    }
    string getNumber() { return number; }
    string getOwner() { return owner; }
};

class ParkingSpot {
private:
    string number;
    bool isOccupied;
    Car* car;
public:
    ParkingSpot(string numberParking) {
        number = numberParking;
        isOccupied = false;
        car = nullptr;
    }
    
    bool getIsOccupied() const { return isOccupied; }
    string getNumber() const { return number; }
    
    Car* getParkedCar() { return car; }
    
    void park(Car* c) {
        car = c;
        isOccupied = true;
        cout << "Место: " << number << " занято, машина: " << car->getNumber() << ". \n";
    }
    
    void leave() {
        if (isOccupied) {
            cout << "Место: " << number << " освободилось.\n";
            car = nullptr;
            isOccupied = false;
        } else {
            cout << "Место: " << number << "свободно.\n";
        }
    }
    
};

class ParkingManager {
private:
    vector<ParkingSpot> spots;
    GateController gateController;
public:
    ParkingManager(int totalSpots, const string& portPath) {
        for (int i = 1; i <= totalSpots; i++) {
            spots.push_back(ParkingSpot(to_string(i)));
        }
        
        if (!gateController.init(portPath)) {
            cout << "Ошибка инициализации контроллера управления шлагбаумом";
        }
    }
    
    // Ищет свободные места, паркует занимает место для машины
    void parkCar(Car& car) {
        
        // Проверим нет ли такой машины уже на праковке
        for (ParkingSpot& spot: spots) {
            if (spot.getIsOccupied()) {
                Car* currentCar = spot.getParkedCar();
                if (currentCar->getNumber() == car.getNumber()) {
                    cout << "Машина с номерами " << car.getNumber() << " уже есть на парковке.\n";
                    return;
                }
            }
        }
        
        // Ищем свободные места, если находим паркуемся
        for (ParkingSpot& spot: spots) {
            if (!spot.getIsOccupied()) {
                // Занимаем место
                spot.park(&car);
                // Открываем шлагбаум
                gateController.openGate();
                sleep(15);
                gateController.closeGate();
                return;
            }
        }
        cout << "Извините, свободных мест нет.\n";
    }
    
    // Освободилось место
    void releaseSpot(string spotNumber) {
        auto it = find_if(spots.begin(), spots.end(), [&spotNumber](ParkingSpot& s) {
            return s.getNumber() == spotNumber;
        });
        
        // Если итератор не равен .end(), значит элемент найден
        if (it != spots.end()) {
            it->leave();
        } else {
            cout << "Парковочное место не найдено.\n";
        }
    }
    
};

int main(int argc, const char * argv[]) {
    string portName = "/dev/ttys322";
    // Парковка на 2 места
    ParkingManager manager(2, portName);
    
    Car car1("A001AA", "Иванов", "+7 996 558 91 96");
    Car car2("A002AA", "Петров", "+7 996 558 91 95");
    Car car3("E777KX", "Developer", "+7 996 558 91 94");
    Car car4("A004AA", "Иванов", "+7 996 558 91 93");
    
    manager.parkCar(car1);
//    manager.parkCar(car2);
//    manager.releaseSpot("1");
//    manager.parkCar(car3);
//    manager.parkCar(car4);
//    manager.parkCar(car1);
    
    return 0;
}

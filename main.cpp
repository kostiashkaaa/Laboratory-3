#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <clocale>

enum class City {
    Minsk = 1,
    Gomel,
    Grodno,
    Brest,
    Mogilev,
    Vitebsk
};

static const std::unordered_map<City, std::string> CITY_TO_NAME = {
    {City::Minsk,   "Минск"},
    {City::Gomel,   "Гомель"},
    {City::Grodno,  "Гродно"},
    {City::Brest,   "Брест"},
    {City::Mogilev, "Могилёв"},
    {City::Vitebsk, "Витебск"}
};


static std::string cityToString(City c) {
    auto it = CITY_TO_NAME.find(c);
    return it == CITY_TO_NAME.end() ? std::string("Неизвестно") : it->second;
}

class TariffCatalog {
private:
    std::unordered_map<City, double> pricePerMinuteByCity;

    static TariffCatalog* instance_ptr;

    TariffCatalog() {
    }
    TariffCatalog(const TariffCatalog&) = delete;
    TariffCatalog& operator=(const TariffCatalog&) = delete;

public:
    ~TariffCatalog() {
    }


    static TariffCatalog* getInstance() {
        if (instance_ptr == nullptr) {
            instance_ptr = new TariffCatalog();
        }
        return instance_ptr;
    }


    static void setInstance(TariffCatalog* newInstance) {
        if (instance_ptr != nullptr && instance_ptr != newInstance) {
            delete instance_ptr;
        }
        instance_ptr = newInstance;
    }


    static bool hasInstance() {
        return instance_ptr != nullptr;
    }


    static void destroyInstance() {
        if (instance_ptr != nullptr) {
            delete instance_ptr;
            instance_ptr = nullptr;
        }
    }

    void setTariff(City city, double pricePerMinute) {
        pricePerMinuteByCity[city] = pricePerMinute;
    }

    double getTariffOrThrow(City city) const {
        auto it = pricePerMinuteByCity.find(city);
        if (it == pricePerMinuteByCity.end()) {
            throw std::runtime_error("TARIFF_NOT_SET");
        }
        return it->second;
    }


    bool hasTariff(City city) const {
        return pricePerMinuteByCity.find(city) != pricePerMinuteByCity.end();
    }


    void printAll() const {
        if (pricePerMinuteByCity.empty()) {
            std::cout << "Тарифы не заданы.\n";
            return;
        }
        std::cout << "Тарифы (цена за минуту):\n";
        for (const auto& kv : pricePerMinuteByCity) {
            std::cout << "  " << cityToString(kv.first) << ": " << kv.second << " руб.\n";
        }
    }
};

TariffCatalog* TariffCatalog::instance_ptr = nullptr;


struct CallRecord {
    City city;
    unsigned int durationMinutes{ 0 };

    ~CallRecord() {
    }
};

class Client {
private:
    std::string lastName;
    std::vector<CallRecord> calls;

public:
    explicit Client(std::string lastName_)
        : lastName(std::move(lastName_)) {
    }


    ~Client() {
    }


    const std::string& getLastName() const { return lastName; }


    void addCall(City city, unsigned int minutes) {
        calls.push_back(CallRecord{ city, minutes });
    }
    double computeTotalCost() const {
        double total = 0.0;
        TariffCatalog* t = TariffCatalog::getInstance();
        for (const CallRecord& c : calls) {
            if (!t->hasTariff(c.city)) {
                continue;
            }
            double pricePerMinute = t->getTariffOrThrow(c.city);
            total += pricePerMinute * static_cast<double>(c.durationMinutes);
        }
        return total;
    }

    const std::vector<CallRecord>& getCalls() const { return calls; }
};

class ATC {
private:
    std::vector<Client> clients;
    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return s;
    }

public:
    ~ATC() {

    }

    bool addClient(const std::string& lastName) {
        if (lastName.empty()) return false;
        if (findClientIndex(lastName) != -1) return false;
        clients.emplace_back(lastName);
        return true;
    }

    bool registerCall(const std::string& lastName, City city, unsigned int minutes) {
        int idx = findClientIndex(lastName);
        if (idx == -1) return false;
        clients[static_cast<size_t>(idx)].addCall(city, minutes);
        return true;
    }


    double costForClient(const std::string& lastName) const {
        int idx = findClientIndex(lastName);
        if (idx == -1) throw std::runtime_error("Client not found");
        return clients[static_cast<size_t>(idx)].computeTotalCost();
    }

    double totalCostAllCalls() const {
        double total = 0.0;
        for (const Client& c : clients) {
            total += c.computeTotalCost();
        }
        return total;
    }

    void printClients() const {
        if (clients.empty()) {
            std::cout << "Клиенты не зарегистрированы.\n";
            return;
        }
        std::cout << "Клиенты (фамилии):\n";
        for (const Client& c : clients) {
            std::cout << "  - " << c.getLastName() << "\n";
        }
    }

private:
    int findClientIndex(const std::string& lastName) const {
        std::string needle = toLower(lastName);
        for (size_t i = 0; i < clients.size(); ++i) {
            if (toLower(clients[i].getLastName()) == needle) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};


static void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


static bool isValidInteger(const std::string& str, int& result) {
    if (str.empty() || str.find_first_not_of(" \t\r\n") == std::string::npos) {
        return false;
    }
    std::istringstream iss(str);
    iss >> result;
    std::string remainder;
    iss >> remainder;
    return iss.eof() && remainder.empty();
}

static bool isValidDouble(const std::string& str, double& result) {
    if (str.empty() || str.find_first_not_of(" \t\r\n") == std::string::npos) {
        return false;
    }
    for (char c : str) {
        if (!std::isdigit(static_cast<unsigned char>(c)) && c != '.' && c != ',' && 
            c != '-' && c != '+' && c != 'e' && c != 'E' && !std::isspace(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    std::istringstream iss(str);
    iss >> result;
    std::string remainder;
    iss >> remainder;
    return iss.eof() && remainder.empty();
}


static std::string readNonEmptyString(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        if (std::cin.fail()) {
            std::cout << "Ошибка чтения. Повторите ввод.\n";
            clearInputBuffer();
            continue;
        }
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
        if (!s.empty()) {
            return s;
        }
        else {
            std::cout << "Пустая строка недопустима. Повторите ввод.\n";
        }
    }
}


static int readIntInRange(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        std::string input = readNonEmptyString(prompt);
        int value;
        if (isValidInteger(input, value)) {
            if (value >= minValue && value <= maxValue) {
                return value;
            }
            else {
                std::cout << "Значение должно быть в диапазоне [" << minValue << ", " << maxValue << "].\n";
            }
        }
        else {
            std::cout << "Некорректный ввод. Введите целое число от " << minValue << " до " << maxValue << ".\n";
        }
    }
}


static unsigned int readPositiveUInt(const std::string& prompt, unsigned int minValue = 1, unsigned int maxValue = 1440) {
    while (true) {
        std::string input = readNonEmptyString(prompt);
        int value;
        if (isValidInteger(input, value)) {
            if (value >= static_cast<int>(minValue) && value <= static_cast<int>(maxValue)) {
                return static_cast<unsigned int>(value);
            }
            else {
                std::cout << "Значение должно быть в диапазоне [" << minValue << ", " << maxValue << "] минут.\n";
            }
        }
        else {
            std::cout << "Некорректный ввод. Введите положительное целое число от " << minValue << " до " << maxValue << ".\n";
        }
    }
}


static double readPositiveDouble(const std::string& prompt, double minValue = 0.01, double maxValue = 1000.0) {
    while (true) {
        std::string input = readNonEmptyString(prompt);
        double value;
        if (isValidDouble(input, value)) {
            if (value >= minValue && value <= maxValue) {
                return value;
            }
            else {
                std::cout << "Значение должно быть в диапазоне [" << minValue << ", " << maxValue << "].\n";
            }
        }
        else {
            std::cout << "Некорректный ввод. Введите число от " << minValue << " до " << maxValue << ".\n";
        }
    }
}


static City readCity() {
    std::cout << "Выберите город:\n";
    for (const auto& kv : CITY_TO_NAME) {
        std::cout << "  " << static_cast<int>(kv.first) << ") " << kv.second << "\n";
    }
    int choice = readIntInRange("Введите номер города: ", 1, static_cast<int>(CITY_TO_NAME.size()));
    return static_cast<City>(choice);
}


static void actionAddTariff() {
    City city = readCity();
    double price = readPositiveDouble("Введите цену за минуту (руб.): ");
    TariffCatalog::getInstance()->setTariff(city, price);
    std::cout << "Тариф обновлён для города " << cityToString(city) << "\n";
}


static void actionAddClient(ATC& atc) {
    std::string lastName = readNonEmptyString("Введите фамилию клиента: ");
    if (atc.addClient(lastName)) {
        std::cout << "Клиент добавлен.\n";
    }
    else {
        std::cout << "Не удалось добавить клиента (дубликат или пустая строка).\n";
    }
}


static void actionRegisterCall(ATC& atc) {
    std::string lastName = readNonEmptyString("Введите фамилию клиента: ");
    City city = readCity();
    if (!TariffCatalog::getInstance()->hasTariff(city)) {
        std::cout << "Для этого города не задан тариф. Сначала задайте тариф.\n";
        return;
    }
    unsigned int minutes = readPositiveUInt("Введите длительность звонка в минутах (1..1440): ");
    if (atc.registerCall(lastName, city, minutes)) {
        std::cout << "Звонок зарегистрирован.\n";
    }
    else {
        std::cout << "Клиент не найден.\n";
    }
}


static void actionCostByClient(const ATC& atc) {
    try {
        std::string lastName = readNonEmptyString("Введите фамилию клиента: ");
        double cost = atc.costForClient(lastName);
        std::cout.setf(std::ios::fixed);
        std::cout.precision(2);
        std::cout << "Общая стоимость звонков клиента " << lastName << ": " << cost << " руб.\n";
    }
    catch (const std::runtime_error&) {
        std::cout << "Клиент не найден или не заданы тарифы.\n";
    }
}


static void actionTotalCostAll(const ATC& atc) {
    double cost = atc.totalCostAllCalls();
    std::cout.setf(std::ios::fixed);
    std::cout.precision(2);
    std::cout << "Общая стоимость всех звонков: " << cost << " руб.\n";
}

static void printMenu() {
    std::cout << "\n==== Меню АТС ====\n";
    std::cout << "1) Добавить/обновить тариф\n";
    std::cout << "2) Показать тарифы\n";
    std::cout << "3) Добавить клиента\n";
    std::cout << "4) Показать клиентов\n";
    std::cout << "5) Зарегистрировать звонок\n";
    std::cout << "6) Стоимость по клиенту\n";
    std::cout << "7) Общая стоимость всех звонков\n";
    std::cout << "0) Выход\n";
}


int main() {
    setlocale(LC_ALL, "");

    ATC atc;

    TariffCatalog::getInstance()->setTariff(City::Minsk, 0.5);
    TariffCatalog::getInstance()->setTariff(City::Gomel, 0.4);

    std::cout << "Система АТС запущена. Начальные тарифы загружены.\n";

    while (true) {
        try {
            printMenu();
            int choice = readIntInRange("Выберите пункт меню: ", 0, 7);
            switch (choice) {
            case 1: actionAddTariff(); break;
            case 2: TariffCatalog::getInstance()->printAll(); break;
            case 3: actionAddClient(atc); break;
            case 4: atc.printClients(); break;
            case 5: actionRegisterCall(atc); break;
            case 6: actionCostByClient(atc); break;
            case 7: actionTotalCostAll(atc); break;
            case 0:
                std::cout << "До свидания!\n";
                TariffCatalog::destroyInstance();
                return 0;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Критическая ошибка: " << e.what() << "\n";
        }
    }
}

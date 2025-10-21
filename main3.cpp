#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <sstream>
#include <iomanip>
#include <clocale>

class ITariffStrategy {
protected:
    std::string destination;
    double basePrice;

public:
    ITariffStrategy(std::string dest, double price)
        : destination(std::move(dest)), basePrice(price) {
        if (basePrice < 0) {
            throw std::invalid_argument("Цена не может быть отрицательной");
        }
    }

    virtual ~ITariffStrategy() = default;

    virtual double calculateCost(unsigned int minutes) const = 0;

    virtual double getFinalPricePerMinute() const = 0;

    virtual std::string getTariffType() const = 0;

    const std::string& getDestination() const { return destination; }
    double getBasePrice() const { return basePrice; }

    virtual void printInfo() const {
        std::cout << "Направление: " << destination
            << ", Базовая цена: " << std::fixed << std::setprecision(2)
            << basePrice << " руб/мин";
    }
};


class RegularTariff : public ITariffStrategy {
public:
    RegularTariff(const std::string& dest, double price)
        : ITariffStrategy(dest, price) {
    }

    double calculateCost(unsigned int minutes) const override {
        return basePrice * minutes;
    }

    double getFinalPricePerMinute() const override {
        return basePrice;
    }

    std::string getTariffType() const override {
        return "Обычный";
    }

    void printInfo() const override {
        ITariffStrategy::printInfo();
        std::cout << ", Тип: Обычный тариф" << std::endl;
    }
};

class BenefitTariff : public ITariffStrategy {
private:
    double discountPercent;

public:
    BenefitTariff(const std::string& dest, double price, double discount)
        : ITariffStrategy(dest, price), discountPercent(discount) {
        if (discount < 1 || discount > 99) {
            throw std::invalid_argument("Скидка должна быть в диапазоне 1-99%");
        }
    }

    double calculateCost(unsigned int minutes) const override {
        double discountMultiplier = 1.0 - (discountPercent / 100.0);
        return basePrice * minutes * discountMultiplier;
    }

    double getFinalPricePerMinute() const override {
        return basePrice * (1.0 - discountPercent / 100.0);
    }

    std::string getTariffType() const override {
        return "Льготный";
    }

    double getDiscountPercent() const { return discountPercent; }

    void printInfo() const override {
        ITariffStrategy::printInfo();
        std::cout << ", Скидка: " << std::fixed << std::setprecision(1)
            << discountPercent << "%, Цена со скидкой: "
            << std::setprecision(2) << getFinalPricePerMinute()
            << " руб/мин, Тип: Льготный тариф" << std::endl;
    }
};

class ATC {
private:
    std::vector<std::shared_ptr<ITariffStrategy>> tariffs;

public:
    ~ATC() = default;

    void addRegularTariff(const std::string& destination, double price) {
        if (destination.empty()) {
            throw std::invalid_argument("Направление не может быть пустым");
        }
        if (price <= 0) {
            throw std::invalid_argument("Цена должна быть положительной");
        }

        tariffs.push_back(std::make_shared<RegularTariff>(destination, price));
        std::cout << "Обычный тариф добавлен успешно" << std::endl;
    }

    void addBenefitTariff(const std::string& destination, double price, double discount) {
        if (destination.empty()) {
            throw std::invalid_argument("Направление не может быть пустым");
        }
        if (price <= 0) {
            throw std::invalid_argument("Цена должна быть положительной");
        }
        if (discount < 1 || discount > 99) {
            throw std::invalid_argument("Скидка должна быть в диапазоне 1-99%");
        }

        tariffs.push_back(std::make_shared<BenefitTariff>(destination, price, discount));
        std::cout << "Льготный тариф добавлен успешно" << std::endl;
    }

    double calculateAverageCost() const {
        if (tariffs.empty()) {
            throw std::runtime_error("Нет тарифов для расчета");
        }

        double totalPrice = 0.0;
        for (const auto& tariff : tariffs) {
            totalPrice += tariff->getFinalPricePerMinute();
        }

        return totalPrice / tariffs.size();
    }

    void printAllTariffs() const {
        if (tariffs.empty()) {
            std::cout << "Тарифы отсутствуют" << std::endl;
            return;
        }

        std::cout << "\nСписок тарифов:" << std::endl;
        for (size_t i = 0; i < tariffs.size(); ++i) {
            std::cout << (i + 1) << ". ";
            tariffs[i]->printInfo();
        }
    }

    size_t getTariffCount() const {
        return tariffs.size();
    }

    void printStatistics() const {
        if (tariffs.empty()) {
            std::cout << "Нет данных для статистики" << std::endl;
            return;
        }

        int regularCount = 0;
        int benefitCount = 0;

        for (const auto& tariff : tariffs) {
            if (tariff->getTariffType() == "Обычный") {
                regularCount++;
            }
            else {
                benefitCount++;
            }
        }

        std::cout << "\nСтатистика:" << std::endl;
        std::cout << "Всего тарифов: " << tariffs.size() << std::endl;
        std::cout << "Обычных: " << regularCount << std::endl;
        std::cout << "Льготных: " << benefitCount << std::endl;
        std::cout << "Средняя стоимость (с учётом скидок): "
            << std::fixed << std::setprecision(2)
            << calculateAverageCost() << " руб/мин" << std::endl;
    }

    void clearAllTariffs() {
        tariffs.clear();
        std::cout << "Все тарифы удалены" << std::endl;
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
    iss >> std::ws >> remainder;
    return iss.eof() && remainder.empty();
}

static bool isValidDouble(const std::string& str, double& result) {
    if (str.empty() || str.find_first_not_of(" \t\r\n") == std::string::npos) {
        return false;
    }

    std::string cleaned = str;
    std::replace(cleaned.begin(), cleaned.end(), ',', '.');

    std::istringstream iss(cleaned);
    iss >> result;
    std::string remainder;
    iss >> std::ws >> remainder;
    return iss.eof() && remainder.empty();
}

static std::string readNonEmptyString(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);

        if (std::cin.fail()) {
            std::cout << "Ошибка чтения. Повторите ввод." << std::endl;
            clearInputBuffer();
            continue;
        }

        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);

        if (!s.empty()) {
            return s;
        }
        else {
            std::cout << "Пустая строка недопустима. Повторите ввод." << std::endl;
        }
    }
}

static bool isValidCityName(const std::string& cityName) {
    if (cityName.empty() || cityName.length() > 50) {
        return false;
    }

    bool hasLetter = false;

    for (size_t i = 0; i < cityName.length(); ++i) {
        unsigned char ch = static_cast<unsigned char>(cityName[i]);

        if (ch >= '0' && ch <= '9') {
            return false;
        }

        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            hasLetter = true;
        }

        if (ch >= 128) {
            hasLetter = true;
        }
    }

    return hasLetter;
}

static std::string readCityName(const std::string& prompt) {
    while (true) {
        std::string cityName = readNonEmptyString(prompt);

        if (isValidCityName(cityName)) {
            return cityName;
        }
        else {
            std::cout << "Некорректное название города. Используйте только буквы, пробелы и дефисы." << std::endl;
            std::cout << "Длина должна быть от 1 до 50 символов." << std::endl;
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
                std::cout << "Значение должно быть в диапазоне ["
                    << minValue << ", " << maxValue << "]" << std::endl;
            }
        }
        else {
            std::cout << "Некорректный ввод. Введите целое число от "
                << minValue << " до " << maxValue << std::endl;
        }
    }
}

static double readPositiveDouble(const std::string& prompt, double minValue = 0.01, double maxValue = 10000.0) {
    while (true) {
        std::string input = readNonEmptyString(prompt);
        double value;

        if (isValidDouble(input, value)) {
            if (value >= minValue && value <= maxValue) {
                return value;
            }
            else {
                std::cout << "Значение должно быть в диапазоне ["
                    << minValue << ", " << maxValue << "]" << std::endl;
            }
        }
        else {
            std::cout << "Некорректный ввод. Введите число от "
                << minValue << " до " << maxValue << std::endl;
        }
    }
}

static void actionAddRegularTariff(ATC& atc) {
    try {
        std::string destination = readCityName("Введите направление (город): ");
        double price = readPositiveDouble("Введите цену за минуту (руб.): ", 0.01, 1000.0);
        atc.addRegularTariff(destination, price);
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << std::endl;
    }
}

static void actionAddBenefitTariff(ATC& atc) {
    try {
        std::string destination = readCityName("Введите направление (город): ");
        double price = readPositiveDouble("Введите базовую цену за минуту (руб.): ", 0.01, 1000.0);
        double discount = readPositiveDouble("Введите размер скидки (%): ", 1.0, 99.0);
        atc.addBenefitTariff(destination, price, discount);
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << std::endl;
    }
}

static void printMenu() {
    std::cout << "\nМеню АТС:" << std::endl;
    std::cout << "1. Добавить обычный тариф" << std::endl;
    std::cout << "2. Добавить льготный тариф" << std::endl;
    std::cout << "3. Показать все тарифы" << std::endl;
    std::cout << "4. Показать среднюю стоимость" << std::endl;
    std::cout << "5. Показать статистику" << std::endl;
    std::cout << "6. Очистить все тарифы" << std::endl;
    std::cout << "0. Выход" << std::endl;
}

int main() {
    setlocale(LC_ALL, "");
    ATC atc;
    // Добавление тестовых данных
    std::cout << "\nИнициализация тестовых данных..." << std::endl;
    try {
        atc.addRegularTariff("Москва", 2.50);
        atc.addRegularTariff("Санкт-Петербург", 2.30);
        atc.addBenefitTariff("Минск", 1.80, 15.0);
        atc.addBenefitTariff("Киев", 2.00, 20.0);
        std::cout << "Тестовые данные загружены\n" << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка при загрузке тестовых данных: " << e.what() << std::endl;
    }

    while (true) {
        try {
            printMenu();
            int choice = readIntInRange("Выберите пункт меню: ", 0, 6);

            switch (choice) {
            case 1:
                actionAddRegularTariff(atc);
                break;
            case 2:
                actionAddBenefitTariff(atc);
                break;
            case 3:
                atc.printAllTariffs();
                break;
            case 4:
                try {
                    std::cout << "\nСредняя стоимость тарифов с учётом скидок: "
                        << std::fixed << std::setprecision(2)
                        << atc.calculateAverageCost() << " руб/мин" << std::endl;
                }
                catch (const std::exception& e) {
                    std::cout << "Ошибка: " << e.what() << std::endl;
                }
                break;
            case 5:
                atc.printStatistics();
                break;
            case 6:
                atc.clearAllTariffs();
                break;
            case 0:
                std::cout << "\nРабота завершена. До свидания!" << std::endl;
                return 0;
            }
        }
        catch (const std::exception& e) {
            std::cout << "Критическая ошибка: " << e.what() << std::endl;
        }
    }

    return 0;
}

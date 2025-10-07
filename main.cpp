#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

// ===================== Part 1: Class design only =====================

// Enum with several cities (required by task)
enum class City {
	Minsk = 1,
	Gomel,
	Grodno,
	Brest,
	Mogilev,
	Vitebsk
};

static const std::unordered_map<City, std::wstring> CITY_TO_NAME = {
	{City::Minsk,   L"Минск"},
	{City::Gomel,   L"Гомель"},
	{City::Grodno,  L"Гродно"},
	{City::Brest,   L"Брест"},
	{City::Mogilev, L"Могилёв"},
	{City::Vitebsk, L"Витебск"}
};

static std::wstring cityToString(City c) {
	auto it = CITY_TO_NAME.find(c);
	return it == CITY_TO_NAME.end() ? std::wstring(L"Неизвестно") : it->second;
}

// Singleton: TariffCatalog stores per-minute price for each City
class TariffCatalog {
private:
	std::unordered_map<City, double> pricePerMinuteByCity;
	TariffCatalog() = default;
	TariffCatalog(const TariffCatalog&) = delete;
	TariffCatalog& operator=(const TariffCatalog&) = delete;
public:
	~TariffCatalog() = default;
	static TariffCatalog& instance() {
		static TariffCatalog catalog;
		return catalog;
	}
	void setTariff(City city, double pricePerMinute) {
		pricePerMinuteByCity[city] = pricePerMinute;
	}
	double getTariffOrThrow(City city) const {
		auto it = pricePerMinuteByCity.find(city);
		if (it == pricePerMinuteByCity.end()) {
			throw std::runtime_error("Нет тарифа для города");
		}
		return it->second;
	}
};

// Call record stores destination and duration
struct CallRecord {
	City city;
	unsigned int durationMinutes{0};
	~CallRecord() = default;
};

// Client stores last name and its calls; calculates total using TariffCatalog
class Client {
private:
	std::string lastName;
	std::vector<CallRecord> calls;
public:
	explicit Client(std::string lastName_) : lastName(std::move(lastName_)) {}
	~Client() = default;
	const std::string& getLastName() const { return lastName; }
	void addCall(City city, unsigned int minutes) { calls.push_back({city, minutes}); }
	double computeTotalCost() const {
		double total = 0.0;
		const TariffCatalog& t = TariffCatalog::instance();
		for (const CallRecord& c : calls) total += t.getTariffOrThrow(c.city) * c.durationMinutes;
		return total;
	}
};

// ATC stores clients; minimal version for Part 1
class ATC {
private:
	std::vector<Client> clients;
public:
	~ATC() = default;
	void addClient(const std::string& lastName) { clients.emplace_back(lastName); }
	Client& getClientRef(size_t index) { return clients.at(index); }
};

int main() {
	_setmode(_fileno(stdout), _O_U16TEXT);
	// Небольшая демонстрация (без меню):
	// Настроим пару тарифов через Singleton
	TariffCatalog::instance().setTariff(City::Minsk, 0.5);
	TariffCatalog::instance().setTariff(City::Gomel, 0.4);

	ATC atc;
	atc.addClient("Ivanov");
	Client& ivanov = atc.getClientRef(0);
	ivanov.addCall(City::Minsk, 10);  // 10 минут в Минск
	ivanov.addCall(City::Gomel, 5);   // 5 минут в Гомель

	std::wcout.setf(std::ios::fixed);
	std::wcout.precision(2);
	std::wcout << L"Клиент: " << std::wstring(ivanov.getLastName().begin(), ivanov.getLastName().end())
	          << L", общая стоимость = " << ivanov.computeTotalCost() << L"\n";
	return 0;
}



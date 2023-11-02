#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

class Date {
public:
	Date(const int& new_year, const int& new_month, const int& new_day) {
		year = new_year;
		if (ValidateMonth(new_month)) {
			month = new_month;
		}
		else {
			throw std::invalid_argument("Month value is invalid: " + std::to_string(new_month));
		}

		if (ValidateDay(new_day)) {
			day = new_day;
		}
		else {
			throw std::invalid_argument("Day value is invalid: " + std::to_string(new_day));
		}
	}

	int GetYear() const {
		return year;
	}

	int GetMonth() const {
		return month;
	}

	int GetDay() const {
		return day;
	}

	bool operator==(const Date& rhs) const {
		return year == rhs.year && month == rhs.month && day == rhs.day;
	}

	bool operator<(const Date& rhs) const {
		return (year * 365 + month * 12 + day * 31) < (rhs.year * 365 + rhs.month * 12 + rhs.day * 31);
	}

	friend std::ostream& operator<<(std::ostream& stream, const Date& date) {
		stream << date.Print();
		return stream;
	}

	std::string Print() const {
		std::stringstream ss;
		const int year_width = 4;
		const int month_width = 2;
		const int day_width = 2;
		if (year < 0) {
			ss << "-";
		}
		ss << std::setw(year_width) << std::setfill('0') << std::abs(year) << "-" << std::setw(month_width) << std::setfill('0') << month << "-" << std::setw(day_width) << std::setfill('0') << day;
		return ss.str();
	}

	static bool IsAllowedSymbol(const char& c) {
		const std::set<char> allowed_symbols =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '+' };
		if (!allowed_symbols.count(c)) {
			return false;
		}

		return true;
	}

	static bool CheckAndSkipDelimiter(std::stringstream& ss) {
		char c = ss.peek();
		if (c == '-') {
			ss.ignore(1);
			return true;
		}

		return false;
	}

	static void ThrowWrongDateFormatException(const std::string& source) {
		throw std::invalid_argument("Wrong date format: " + source);
	}

	static Date ParseDate(const std::string& source) {
		const size_t min_length = 5;
		size_t len = source.length();
		if (len < min_length) {
			ThrowWrongDateFormatException(source);
		}

		for (const auto& c : source) {
			if (!IsAllowedSymbol(c)) {
				ThrowWrongDateFormatException(source);
			}
		}

		int new_year, new_month, new_day;
		std::stringstream ss(source);
		ss >> new_year;
		if (ss.fail() || !CheckAndSkipDelimiter(ss)) {
			ThrowWrongDateFormatException(source);
		}
		ss >> new_month;
		if (ss.fail() || !CheckAndSkipDelimiter(ss)) {
			ThrowWrongDateFormatException(source);
		}
		ss >> new_day;
		if (ss.fail() || ss.peek() != EOF) {
			ThrowWrongDateFormatException(source);
		}

		return Date(new_year,new_month,new_day);
	}

private:
	int year;
	int month;
	int day;

	bool ValidateMonth(const int& month) const {
		return month >= 1 && month <= 12;
	}

	bool ValidateDay(const int& day) const {
		return day >= 1 && day <= 31;
	}
};

class Database {
public:
	void AddEvent(const Date& date, const std::string& event) {
		entries[date].insert(event);
	}

	bool DeleteEvent(const Date& date, const std::string& event) {
		bool result = false;
		if (entries.count(date)) {
			std::set<std::string>& events = entries.at(date);
			if (events.count(event)) {
				events.erase(event);
				if (events.empty()) {
					DeleteDate(date);
				}
				result = true;
			}
		}

		return result;
	}

	int DeleteDate(const Date& date) {
		int deleted = 0;
		if (entries.count(date)) {
			deleted = static_cast<int>(entries.at(date).size());
			entries.erase(date);
		}

		return deleted;
	}

	std::set<std::string> Find(const Date& date) const {
		if (entries.count(date)) {
			return entries.at(date);
		}
		else {
			throw std::invalid_argument("Date not found");
		}
	}

	void Print() const {
		for (const auto& entry : entries) {
			for (const auto& event : entry.second) {
				std::cout << entry.first << " " << event << std::endl;
			}
		}
	}

	void ExecuteCommand(std::string& command) {
		std::string arg0 = "";
		std::string arg1 = "";
		std::string arg2 = "";

		std::stringstream ss(command);
		ss >> arg0 >> arg1 >> arg2;

		if (arg0.empty()) {
			return;
		}

		std::string original = arg0;
		for (auto& c : arg0)
		{
			c = tolower(c);
		}

		if (!commands_map.count(arg0)) {
			throw std::invalid_argument("Unknown command: " + original);
		}

		switch (commands_map.at(arg0)) {
			// ADD
			case 0: {
				if (arg1.empty() || arg2.empty()) {
					throw std::invalid_argument("Add requires 2 arguments (date event)");
					return;
				}
				AddEvent(Date::ParseDate(arg1), arg2);
				break;
			}
			// DEL
			case 1: {
				if (arg1.empty() && arg2.empty()) {
					throw std::invalid_argument("Del requires 1 (date) or 2 arguments (date event)");
					return;
				}
				if (!arg1.empty() && arg2.empty()) {
					int deleted = DeleteDate(Date::ParseDate(arg1));
					std::cout << "Deleted " << deleted << " events" << std::endl;
				}
				else {
					std::string result = DeleteEvent(Date::ParseDate(arg1), arg2)
						? "Deleted successfully" : "Event not found";
					std::cout << result << std::endl;
				}
				break;
			}
			// FIND
			case 2: {
				if (arg1.empty() || !arg2.empty()) {
					throw std::invalid_argument("Find requires 1 argument (date)");
					return;
				}
				std::set<std::string> events = Find(Date::ParseDate(arg1));
				for (const auto& e : events) {
					std::cout << e << std::endl;
				}
				break;
			}
			// PRINT
			case 3: {
				if (!arg1.empty() || !arg2.empty()) {
					throw std::invalid_argument("Print doesn't accept any argument");
					return;
				}
				Print();
				break;
			}
		}
	}

private:
	const std::map<std::string, int> commands_map = {
		{"add", 0}, {"del", 1}, {"find", 2}, {"print", 3}
	};

	std::map<Date, std::set<std::string>> entries;
};

int main() {

	Database db;
	std::string command;
	while (getline(std::cin, command)) {
		try {
			db.ExecuteCommand(command);
		}
		catch (const std::exception& exc) {
			std::cout << exc.what() << std::endl;
		}
	}

	return 0;
}
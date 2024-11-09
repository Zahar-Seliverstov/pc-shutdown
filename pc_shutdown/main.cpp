#include <iostream>
#include <cstdlib> 
#include <thread>  
#include <chrono>  
#include <fstream>
#include <string>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <regex>
#include <sstream>

std::string pathToFile = "data_time.txt";
std::atomic<bool> cancelTimer(false);

enum keys
{
	upArrow = 72,
	downArrow = 80,
	enter = 13,
	escape = 27,
	del = 46
};

void shutdownTimer(int seconds) {
	for (int i = seconds; i > 0; --i) {
		if (cancelTimer) {
			std::cout << "\nОжидание отменено.\n";
			return;
		}
		printf("\x1b[%d;%dH", 0, 0);
		std::cout << "\x1b[90m[X] \x1b[0mОтменить { \x1b[93mesc\x1b[0m }\n\n";
		std::cout << "Компьютер будет выключен через " << i << " секунд(ы)                                                                                                        ";
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	system("shutdown -s -t 0");
}

void listenForEsc() {
	while (true) {
		if (_kbhit()) {
			char ch = _getch();
			if (ch == 27) {
				cancelTimer = true;
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

int convertTimeToSeconds(const std::string& timeStr) {
	int hours, minutes, seconds;
	char colon;
	std::istringstream iss(timeStr);

	// Разбор строки на часы, минуты и секунды
	if (!(iss >> hours >> colon >> minutes >> colon >> seconds)) {
		throw std::invalid_argument("Invalid time format");
	}

	// Проверка диапазонов
	if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59) {
		throw std::out_of_range("Time values out of range");
	}

	// Вычисление общего количества секунд
	return hours * 3600 + minutes * 60 + seconds;
}

void hideCursor()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void writeToFile(const std::string& time) {
	try {
		std::fstream file;
		file.open(pathToFile, std::ios::out | std::ios::app);

		if (file.is_open()) {
			file << time << '\n';
			file.close();
		}
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

void overwriteAllTimers(std::vector<std::string> arr) {
	std::fstream file;
	file.open(pathToFile, std::ios::out | std::ios::trunc);

	if (file.is_open()) {
		for (const auto& item : arr) {
			file << item;
		}
		file.close();
	}
}

std::vector<std::string> readFile(std::string& pathToFile) {
	try {
		if (pathToFile.length() == 0) {
			std::cout << "Ошибка: путь к файлу пуст (path to file is empty)" << std::endl;
		}

		std::fstream file(pathToFile);
		std::vector<std::string> allTimers;

		if (file.is_open()) {
			std::string line;
			while (std::getline(file, line)) {
				allTimers.push_back(line + '\n');
			}
			file.close();
		}
		else {
			std::cout << "";
		}
		return allTimers;
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

void displayMenu(std::vector<std::string> arr, int index) {
	for (size_t i = 0; i < arr.size(); i++) {
		std::cout << (i == index ? index == 2 ? "\x1b[91m" : "\x1b[97m" : "\x1b[90m");
		std::cout << arr[i] << std::endl;
	}

}

int handleStartMenu() {
	int mode = 0;
	std::vector<std::string> modes = { "Запустить таймер", "Добавить таймер", "Выйти" };
	displayMenu(modes, mode);
	while (true) {
		if (_kbhit()) {
			switch (_getch())
			{
			case upArrow:
				mode = mode - 1 < 0 ? mode = modes.size() - 1 : mode -= 1;
				break;
			case downArrow:
				mode = mode + 1 > 2 ? mode = 0 : mode += 1;
				break;
			case enter:
				return mode;
			}
			printf("\x1b[%d;%dH", 5, 0);
			displayMenu(modes, mode);
		}
	}
	return 0;
}

void displayTimersMenu(std::vector<std::string> arr, int index) {
	for (size_t i = 0; i < arr.size(); i++) {
		std::cout << "\x1b[90m - " << (i == index ? "\x1b[97m" : "\x1b[90m");
		std::cout << arr[i];
	}

}

std::vector<std::string> deleteTimer(std::vector<std::string>& arr, int index) {
	arr.erase(arr.begin() + index);
	return arr;
}

int handleSelectTimerMenu(std::vector<std::string>& allTimers) {
	int timerIndex = 0;
	std::cout << "\x1b[90m<== \x1b[0mНазад \x1b[0m{ \x1b[93mesc\x1b[0m }\n";
	std::cout << "\x1b[90m[X] \x1b[0mУдалить таймер \x1b[0m{ \x1b[93mdel\x1b[0m }\n\n";
	std::cout << "Выберите таймер:\n\n";
	displayTimersMenu(allTimers, timerIndex);

	while (true) {
		if (_kbhit()) {
			switch (_getch())
			{
			case upArrow:
				timerIndex = timerIndex - 1 < 0 ? timerIndex = allTimers.size() - 1 : timerIndex -= 1;
				break;
			case downArrow:
				timerIndex = timerIndex + 1 > allTimers.size() - 1 ? timerIndex = 0 : timerIndex += 1;
				break;
			case enter:
				return timerIndex;
			case escape:
				return -1;
			case del:
				if (allTimers.size() != 0) {
					system("cls");
					overwriteAllTimers(deleteTimer(allTimers, timerIndex));
					std::cout << "\x1b[90m<== \x1b[0mНазад \x1b[0m{ \x1b[93mesc\x1b[0m }\n";
					std::cout << "\x1b[90m[X] \x1b[0mУдалить таймер \x1b[0m{ \x1b[93mdel\x1b[0m }\n\n";
					std::cout << "Выберите таймер:\n\n";
					timerIndex = 0;
				}
				break;
			}
		}
		printf("\x1b[%d;%dH", 6, 0);
		displayTimersMenu(allTimers, timerIndex);
	}
}

bool validateTimeFormat(const std::string& timeStr) {
	std::regex timePattern("^([0-1][0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]$");
	return std::regex_match(timeStr, timePattern);
}

std::string handleAddTimerMenu(std::vector<std::string>& allTimers) {
	std::cout << "\x1b[90m<== \x1b[0mНазад \x1b[0m{ \x1b[93mesc\x1b[0m }\n";
	std::cout << "\x1b[0mВведите время в формате [\x1b[93m hh:mm:ss \x1b[0m]\n\n";

	while (true) {
		std::cout << "=> ";

		std::string newTimer = "";

		while (true) {
			if (_kbhit()) {
				int ch = _getch();
				if (ch == escape) {
					return ""; // Возвращаем пустую строку при нажатии ESC
				}
				else if (ch == '\r') { // Enter
					std::cout << std::endl;

					if (!validateTimeFormat(newTimer)) {
						system("cls");
						std::cout << "\x1b[90m<== \x1b[0mНазад \x1b[0m{ \x1b[93mesc\x1b[0m }\n";
						std::cout << "Неправильный формат времени - Правильный формат [\x1b[93m hh:mm:ss \x1b[0m]\n\n";
						break;
					}

					bool flag = false;

					for (const auto& timer : allTimers) {
						if (timer == newTimer + '\n') {
							system("cls");
							std::cout << "\x1b[90m<== \x1b[0mНазад \x1b[0m{ \x1b[93mesc\x1b[0m }\n";
							std::cout << "Таймер уже существует, попробуйте еще раз\n\n";
							flag = true;
							break;
						}
					}
					if (!flag) return newTimer;
					else break;
				}
				else if (ch == '\b') { // Backspace
					if (!newTimer.empty()) {
						newTimer.pop_back();
						std::cout << "\b \b";
					}
				}
				else if ((ch >= '0' && ch <= '9') || ch == ':') { // Только цифры и двоеточие
					if (newTimer.length() < 8) { // Ограничение длины ввода
						newTimer += static_cast<char>(ch);
						std::cout << static_cast<char>(ch);
					}
				}
			}
		}
	}
}


int main() {

	system("chcp 1251");
	hideCursor();

	int newUserTimer;

	while (true) {

		system("cls");
		std::cout << "\x1b[0mДобро пожаловать в PC Shutdown\n\n";
		std::cout << "\x1b[90m~ \x1b[0mВыберите действие:\n\n";

		int action = handleStartMenu();
		int indexTimer = 0;
		std::string newTimer = "";

		std::vector<std::string> allTimers = readFile(pathToFile);

		switch (action)
		{
		case 0:
			system("cls");
			indexTimer = handleSelectTimerMenu(allTimers);
			if (indexTimer != -1) {
				system("cls");
				int seconds = convertTimeToSeconds(allTimers[indexTimer]);
				cancelTimer = false; // Сбрасываем флаг отмены

				// Запускаем поток для ожидания
				std::thread timerThread(shutdownTimer, seconds);
				std::thread escThread(listenForEsc);

				// Ждем завершения потока таймера
				timerThread.join();
				// Завершаем поток прослушивания клавиши Esc
				escThread.join();
			}
			break;
		case 1:
			system("cls");
			newTimer = handleAddTimerMenu(allTimers);

			if (newTimer != "") {
				writeToFile(newTimer);
			}
			break;
		case 2:
			std::cout << "\x1b[0m";
			return 0;
		}
	}

	return 0;
}
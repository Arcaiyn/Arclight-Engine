#include "util/log.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "config.h"

#define LOG_DEBUG "D"
#define LOG_INFO  "I"
#define LOG_WARN  "W"
#define LOG_ERROR "E"

namespace fs = std::filesystem;


namespace Log {

	std::ofstream logFile;
	bool logfileValid = false;


	void openLogFile() {

		if (logfileValid) {
			return;
		}

		fs::path logfileDir(Config::getLogDirectoryName());

		if (!fs::exists(logfileDir)) {

			bool created = false;

			try {
				created = fs::create_directory(logfileDir);
			} catch (std::exception& e) {
				Raw::error("Logger", "Failed to create log directory");
			}

			if (!created) {
				Raw::error("Logger", "Failed to create log directory");
			}

		}

		fs::path logfilePath(std::string(Config::getLogDirectoryName()) + "/" + Config::getLogFileName());

		logFile.open(logfilePath.string(), std::ios::out);

		if (!logFile.is_open()) {
			Raw::error("Logger", "Failed to open log file");
		} else {
			logfileValid = true;
		}

	}


	void closeLogFile() {

		if (logfileValid) {
			logfileValid = false;
			logFile.close();
		}

	}


	namespace Raw {

		void print(const std::string& level, const std::string& subsystem, const std::string& message) {

			std::stringstream ss;
			ss << "[" << level << "] <" << subsystem << "> " << message;
			std::cout << ss.str() << std::endl;

			if (logfileValid) {
				logFile << ss.str() << std::endl;
			}

		}

		void debug(const std::string& subsystem, const std::string& message) {
			print(LOG_DEBUG, subsystem, message);
		}


		void info(const std::string& subsystem, const std::string& message) {
			print(LOG_INFO, subsystem, message);
		}


		void warn(const std::string& subsystem, const std::string& message) {
			print(LOG_WARN, subsystem, message);
		}


		void error(const std::string& subsystem, const std::string& message) {
			print(LOG_ERROR, subsystem, message);
		}

	}

}
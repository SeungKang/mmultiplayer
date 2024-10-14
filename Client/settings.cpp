#include <fstream>
#include <utility>
#include <Windows.h>

#include "settings.h"

static json settings;

static std::string GetSettingsPath() {
	char path[MAX_PATH];
	GetTempPathA(sizeof(path), path);

	return std::string(path) + "mmultiplayer.settings";
}

void Settings::SetSetting(const char *menu, const char *key, json value) {
	if (settings[menu].is_null()) {
		settings[menu] = json::object();
	}

	settings[menu][key] = std::move(value);
        Settings::Save();
}

json Settings::GetSetting(const char *menu, const char *key, json defaultValue) {
	if (settings[menu].is_null()) {
		settings[menu] = json::object();
	}

	auto &v = settings[menu][key];
	if (v.is_null() || (v.type() != defaultValue.type() && v.is_number() != defaultValue.is_number())) {
		v = std::move(defaultValue);
		Settings::Save(); // This is not needed due to ADL (https://en.wikipedia.org/wiki/Argument-dependent_name_lookup).
	}

	return v;
}

void Settings::Load() {
	auto reset = true;

	auto f = std::ifstream(GetSettingsPath());
	if (f) {
		try {
			settings = json::parse(f);
			reset = false;
		} catch (json::parse_error& e) {
		    MessageBoxA(nullptr, e.what(), "Error", 0);
		}

		f.close(); // If this is not called this would be automatically closed when the function ends due to RAII (https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization), but it's good practice to close it manually
	}

	if (reset) {
		Reset();
	}
}

void Settings::Reset() {
	settings = json::object();
	Settings::Save();
}

void Settings::Save() {
	std::ofstream file(GetSettingsPath(), std::ios::out);
	if (!file) {
		printf("settings: failed to save %s\n", GetSettingsPath().c_str());
		return;
	}

	auto dump = settings.dump();
	file.write(dump.c_str(), dump.size());
	file.close();
}
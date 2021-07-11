#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <Windows.h>
#include <filesystem>
#include <ctime>    
#include <iomanip>
#include <chrono>       
#include "concol.h" // Custom library for terminal color customization: fonts, background, etc

#pragma comment(lib, "urlmon.lib")

void CheckDiscordRunning(); // Checks if Discord Canary is running in the background, not implemented
void GetDiscordPath(std::string& app_path); // Gets discord path if it exists
void GetAppPath(std::string& app_path); // Gets latest app path inside of discord folder
void GlasscordDownload(std::string app_path); // Download glasscord.asar to discordcanary app directory
void JsonFileIO(std::string& app_path); // Perform checks and I/O operations on app directory's json files
int IsDiscordFound(std::string app_path); // Returns 1 if Discord Canary is installed, returns 0 otherwise
void log(std::string, int message_lvl); //Log message in console with a message level > Level 0: Error; Level 1: Warning; Level 2: Info

int main()
{
    // Initialize console decorations
    eku::concolinit();
    SetConsoleTitle(L"Glasscord Installer for Discord by Rikimbili");
	
    // Check if there's an instance of Discord Running and only continue after it has been closed.
    CheckDiscordRunning();

    // Obtaining Discord app path and downloading glasscord 
    std::string app_path;
    GetDiscordPath(app_path);
    GetAppPath(app_path);
    app_path += "\\resources\\app\\backup";
    if (!std::filesystem::is_directory(app_path))
    {
        log("Empty or non-existent directory. Discord may have changed their folder structure. Check Github for updates", 0);
        exit(EXIT_FAILURE);
    }
    GlasscordDownload(app_path);

    // Operations and checks of the json file in the app dir
    JsonFileIO(app_path);

    // If this line is reached, then it is/was installed successfully 
    log("Successfully Installed!", 2);
    std::cout << "Press Enter to exit...";
    std::cin.ignore();
    exit(EXIT_SUCCESS);
}

void GetDiscordPath(std::string &app_path)
{
    char* appdataPATH = nullptr;
    size_t appdata_buffer = 0;

    // Locate AppData\Local path
    if (_dupenv_s(&appdataPATH, &appdata_buffer, "LOCALAPPDATA") == 0 && appdataPATH != nullptr)
    {
        app_path = (std::string)appdataPATH;
        free(appdataPATH);
    }
    else
    {
        log("Failed to locate variable pointing to AppData\\Local", 0);
        exit(EXIT_FAILURE);
    }
	
    // Check if discord canary folder is present
    if (IsDiscordFound(app_path) == 1)
    {
        app_path += "\\DiscordCanary";
    }
    else 
        exit(EXIT_FAILURE);
}

void GetAppPath(std::string& app_path) 
{
    log("Searching for the latest app directory", 2);

    // Iterate through all subdir within app_path and assign the one with the latest app version to app_path
    std::string found_path = app_path;
    for (auto& entry : std::filesystem::directory_iterator(app_path)) 
    {
        std::string temp_app_path = entry.path().string();
		if (strstr(temp_app_path.c_str(), "app-") && strcmp(temp_app_path.c_str(), found_path.c_str()) == 1)
		{
			found_path = temp_app_path;
		}
    }
    app_path = found_path;
}

void GlasscordDownload(std::string app_path)
{
    if (std::filesystem::exists(app_path + "\\glasscord.asar"))
    {
        log("glasscord.asar found in app dir and won't be downloaded", 2);
        return;
    }

    log("Downloading glasscord.asar from github repository...", 2);
    // Convert path string and download
    std::wstring stemp = std::wstring(app_path.begin(), app_path.end()) + L"\\glasscord.asar";
    LPCWSTR download_url = L"https://github.com/AryToNeX/Glasscord/releases/latest/download/glasscord.asar";
    LPCWSTR download_path = stemp.c_str();
    HRESULT hr = URLDownloadToFile(NULL, download_url, download_path, 0, NULL);
    // Check if download was successful
    if (FAILED(hr))
    {
        log("Could not download glasscord.asar to app directory. Make sure Discord is fully closed and try again", 0);
        exit(EXIT_FAILURE);
    }
    else
        log("glasscord.asar successfully downloaded", 2);
}

void JsonFileIO(std::string& app_path)
{
	std::fstream f_existing;
	std::ofstream f_new;
	std::string json_data;

	// Copy package.json content to json_data if file exists
	if (!std::filesystem::exists(app_path + "\\package.json"))
	{
		log("No package.json file exists in app directory. A Discord reinstall/update might fix this issue", 0);
		exit(EXIT_FAILURE);
	}
	f_existing.open(app_path + "\\package.json", std::ios::in);
	f_existing >> json_data;
	f_existing.close();
	// Copy json_data to the created package.original.json file if it doesn't exist
	if (!std::filesystem::exists(app_path + "\\package.original.json"))
	{
		f_new.open(app_path + "\\package.original.json", std::ios::out);
		f_new << json_data;
		f_new.close();
	}
	else
		log("package.original.json already exists in app directory", 2);

	// Replace index.js for glasscord.asar in package.json
	if (strstr(json_data.c_str(), "index.js"))
	{
		json_data = std::regex_replace(json_data, std::regex("index.js"), "glasscord.asar");
		f_existing.open(app_path + "\\package.json", std::ios::out);
		f_existing << json_data;
		f_existing.close();
	}
	else
		log("package.json already has glasscord.asar as main argument", 2);
}

int IsDiscordFound(std::string app_path)
{
    // Display appropiate message based on the folders found
    if (std::filesystem::is_directory(app_path + "\\DiscordCanary"))
    {
        log("Discord Canary found", 2);
        return 1;
    }
    else if (std::filesystem::is_directory(app_path + "\\Discord"))
        log("Discord folder found but not Discord Canary. Make sure Canary is installed before installing Glasscord", 1);
    else
        log("Discord Canary folder not found. Make sure Canary is installed before installing Glasscord", 1);
        
    return 0;
}

void log(std::string msg, int msg_lvl)
{
	// Get local time in HH:MM:SS 24 hour format
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    struct std::tm* ptm = new struct std::tm;
    localtime_s(ptm, &tt);
    std::cout << "[" << std::put_time(ptm, "%X") << "]";

	// Display message based on msg_lvl code
	switch(msg_lvl)
	{
		case 0:
            eku::setcolor(eku::red, eku::defbackcol);
            std::cout << "[ERROR]: " << msg << std::endl;
            break;
		case 1:
            eku::setcolor(eku::yellow, eku::defbackcol);
            std::cout << "[WARNING]: " << msg << std::endl;
            break;
		case 2:
            std::cout << "[INFO]: " << msg << std::endl;
            break;
	}
    eku::setcolor(eku::deftextcol, eku::defbackcol);
}
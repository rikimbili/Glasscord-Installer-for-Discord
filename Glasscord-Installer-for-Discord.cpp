#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <Windows.h>
#include <filesystem>
#include <ctime>    
#include <iomanip>
#include <chrono>
#include <tlhelp32.h>
#include <comdef.h>
#include "concol.h" // Custom library for terminal color customization: fonts, background, etc

#pragma comment(lib, "urlmon.lib")

void CheckDiscordRunning(); // Checks if Discord Canary is running in the background, not implemented
bool FindProcessIDByName(const std::wstring& processName); // Find process by passing file name string, case sensitive
void GetDiscordPath(std::string& app_path); // Gets discord path if it exists
void GetAppPath(std::string& app_path); // Gets latest app path inside of discord folder
void GlasscordDownload(std::string app_path); // Download glasscord.asar to discordcanary app directory
void JsonFileIO(std::string& app_path); // Perform checks and I/O operations on app directory's json files
void ExitDialog(std::string msg, int message_lvl); // Logs a message for program exit
void log(std::string msg, int message_lvl); // Log message in console with a message level > Level 0: Error; Level 1: Warning; Level 2: Info
void ShowConsoleCursor(bool showFlag); // Show/Hide console cursor

int main()
{
    // Initialize console decorations
    eku::concolinit();
    SetConsoleTitle(L"Glasscord Installer for Discord by Rikimbili");
    ShowConsoleCursor(0);
	
    // Check if there's an instance of Discord Running and only continue after it has been closed.
    CheckDiscordRunning();

    // Obtaining Discord app path and downloading glasscord 
    std::string app_path;
    GetDiscordPath(app_path);
    GetAppPath(app_path);
    app_path += "\\resources\\app";
    if (!std::filesystem::is_directory(app_path))
        ExitDialog("Non-existent app directory. Make sure Powercord is fully installed and try again", 0);

    GlasscordDownload(app_path);
    
    // I/O operations and checks of the json file in the app dir
    JsonFileIO(app_path);

    // If this line is reached, then glasscord was installed successfully 
    ExitDialog("Successfully Installed!", 2);
}

void CheckDiscordRunning() 
{
    if (FindProcessIDByName(L"DiscordCanary.exe"))
    {
        std::cout << "Discord Canary is running. Close it to continue";
        while (FindProcessIDByName(L"DiscordCanary.exe"))
        {
            for (int i = 0; i < 3; i++)
            {
				std::cout << '.';
				Sleep(1000);
            }
            for (int i = 0; i < 3; i++)
            {
                std::cout << '\b' << ' ' << '\b';
            }
        }
    }
    std::cout << std::endl;
}

bool FindProcessIDByName(const std::wstring& processName)
{
	PROCESSENTRY32 pe32;
	// Before using this structure, set its size
	pe32.dwSize = sizeof(pe32);
	//Take a snapshot of all processes in the system
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
        return 0;

	//Traverse the process snapshots until processName is found or end is reached
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		if (pe32.szExeFile == processName)
		{
			::CloseHandle(hProcessSnap);
            return 1;
		}
		bMore = ::Process32Next(hProcessSnap, &pe32);
	}
	::CloseHandle(hProcessSnap);
	return 0;
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
        ExitDialog("Failed to locate variable pointing to AppData\\Local", 0);
	
    // Check if discord canary folder is present
    if (std::filesystem::is_directory(app_path + "\\DiscordCanary"))
    {
        log("Discord Canary found", 2);
        app_path += "\\DiscordCanary";
    }
    else
        ExitDialog("Discord Canary folder not found. Make sure Canary is installed", 1);
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
        ExitDialog("Could not download glasscord.asar to app directory. Make sure you have a stable internet connection and try again", 0);
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
        ExitDialog("No package.json file exists in app directory. A Discord reinstall/update might fix this issue", 0);

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

void ExitDialog(std::string msg, int message_lvl)
{
	log(msg, message_lvl);
	std::cout << "Press Enter to exit...";
	std::cin.ignore();

    if (message_lvl == 2)
        exit(EXIT_SUCCESS);
	exit(EXIT_FAILURE);
}

void log(std::string msg, int msg_lvl)
{
	// Get local time in HH:MM:SS 24 hour format
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    struct std::tm* ptm = new struct std::tm;
    localtime_s(ptm, &tt);
    std::cout << "[" << std::put_time(ptm, "%X") << "]";

	// Display formatted messages based on msg_lvl code
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

void ShowConsoleCursor(bool showFlag)
{
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = showFlag; // set the cursor visibility
	SetConsoleCursorInfo(out, &cursorInfo);
}
/*
 * TODO:
 * 
 *
 */ 
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <filesystem>
#include <iomanip>    
#include <ctime>       
#include <chrono>       
#include "concol.h"

#pragma comment(lib, "urlmon.lib")

void GetDiscordPath(std::string& app_path); // Gets discord app path if it exists
int IsDiscordInstalled(std::string app_path); // Returns 1 if Discord Canary is installed, returns 0 otherwise
void GlasscordDownload(std::string app_path); // Download glasscord.asar to discordcanary app directory
void log(std::string, int message_lvl); //Log message in console with a message level > Level 0: Error; Level 1: Warning; Level 2: Info

int main()
{
    // Initialize console decorations
    eku::concolinit();
    SetConsoleTitle(L"Glasscord Installer for Discord by Rikimbili");
	
    std::string app_path;

    GetDiscordPath(app_path);

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
    if (IsDiscordInstalled(app_path) == 1)
    {
        app_path += "\\DiscordCanary";
    }
    else 
        exit(EXIT_FAILURE);
}

void GlasscordDownload(std::string &app_path)
{
    log("Downloading glasscord.asar from github repository...", 2);

    LPCWSTR download_url = L"https://github.com/AryToNeX/Glasscord/releases/download/v1.0.1/glasscord.asar";
    LPCWSTR download_path = (LPCWSTR)(app_path + "\\glasscord.asar").c_str();

    URLDownloadToFile(NULL, download_url, download_path, 0, NULL); // TODO: Perform error handling on this part
}

int IsDiscordInstalled(std::string app_path)
{
    // Display appropiate message based on the folders found
    if (std::filesystem::is_directory(app_path + "\\DiscordCanary"))
    {
        log("Discord Canary folder present", 2);
        return 1;
    }
    else if (std::filesystem::is_directory(app_path + "\\Discord"))
        log("Discord folder found but not Discord Canary. Ensure Canary is installed before installing Glasscord", 1);
    else
        log("Discord Canary folder not found. Ensure Canary is installed before installing Glasscord", 1);
        
    return 0;
}

void log(std::string msg, int msg_lvl)
{
	// Get local time in HH:MM:SS 24 hour format
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    struct std::tm* ptm = std::localtime(&tt);
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
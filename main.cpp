#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

void show_banner() {
    std::cout << "-----------------------------------\n";
    std::cout << "Author: Yoxi1\n";
    std::cout << "GitHub: https://github.com/Yoxi228\n";
    std::cout << "-----------------------------------\n\n";
}

std::vector<std::wstring> get_available_drives() {
    std::vector<std::wstring> drives;
    DWORD drive_mask = GetLogicalDrives();

    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        if (drive_mask & (1 << (drive - 'A'))) {
            drives.push_back(std::wstring(1, drive) + L":\\");
        }
    }

    return drives;
}

bool set_pagefile_size(const std::wstring& drive, int min_size, int max_size) {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        0, KEY_SET_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open the registry. Try running as administrator.\n";
        return false;
    }

    std::wstring pagefile_value = drive + L"pagefile.sys " +
                                  std::to_wstring(min_size) + L" " +
                                  std::to_wstring(max_size);

    result = RegSetValueEx(hKey, L"PagingFiles", 0, REG_SZ,
                           (const BYTE*)pagefile_value.c_str(),
                           (pagefile_value.size() + 1) * sizeof(wchar_t));

    RegCloseKey(hKey);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to set the pagefile size.\n";
        return false;
    }
    return true;
}

bool ask_for_reboot() {
    char choice;
    std::cout << "\nDo you want to restart now to apply changes? (y/n): ";
    std::cin >> choice;
    return (choice == 'y' || choice == 'Y');
}

// Ram func 
DWORD get_installed_ram_gb() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (GlobalMemoryStatusEx(&statex)) {
        DWORD ram_gb = static_cast<DWORD>(statex.ullTotalPhys / (1024ull * 1024ull * 1024ull));
        return ram_gb;
    } else {
        std::cerr << "Failed to detect RAM size.\n";
        return 0;
    }
}

int wmain() {
    show_banner();

    // Step 1: Detect RAM
    DWORD ram_gb = get_installed_ram_gb();
    if (ram_gb == 0) {
        std::cout << "Cannot continue without knowing the RAM size.\n";
        return 1;
    }

    std::cout << "Detected RAM size: " << ram_gb << " GB\n";

    int min_size = 0;
    int max_size = 0;

    if (ram_gb <= 16) {
        min_size = max_size = 30000;
        std::cout << "Using preset for <=16GB: 30000MB pagefile size.\n";
    } else {
        min_size = max_size = 20000;
        std::cout << "Using preset for >16GB: 20000MB pagefile size.\n";
    }

    // Step 2: Choose drive
    auto drives = get_available_drives();
    if (drives.empty()) {
        std::cout << "No available drives found.\n";
        return 1;
    }

    std::cout << "\nAvailable drives:\n";
    for (size_t i = 0; i < drives.size(); ++i) {
        std::wcout << i + 1 << ". " << drives[i] << "\n";
    }

    int drive_choice = 0;
    std::cout << "\nSelect a drive (number): ";
    std::cin >> drive_choice;

    if (drive_choice < 1 || drive_choice > drives.size()) {
        std::cout << "Invalid drive choice.\n";
        return 1;
    }

    std::wstring selected_drive = drives[drive_choice - 1];

    // Step 3: Set pagefile
    if (set_pagefile_size(selected_drive, min_size, max_size)) {
        std::cout << "\nPagefile size updated successfully!\n";
    } else {
        std::cout << "\nFailed to update pagefile size.\n";
        return 1;
    }

    // Step 4: Reboot option
    if (ask_for_reboot()) {
        system("shutdown /r /t 0");
    } else {
        std::cout << "Please restart your computer manually to apply changes.\n";
    }

    return 0;
}

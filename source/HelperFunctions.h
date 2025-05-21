#ifndef HelperFunctions_h
#define HelperFunctions_h

#include <windows.h>
#include <string>
#include <format>

typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

inline std::wstring GetWindowsVersionString() {
    // First, try RtlGetVersion (most accurate)
    if (HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll")) {
        if (auto fn = reinterpret_cast<RtlGetVersionPtr>(::GetProcAddress(hMod, "RtlGetVersion"))) {
            RTL_OSVERSIONINFOW rovi = {};
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fn(&rovi) == 0) {
                std::wstring name;

                if (rovi.dwMajorVersion == 10 && rovi.dwBuildNumber >= 22000) {
                    name = L"Windows 11";
                } else if (rovi.dwMajorVersion == 10) {
                    name = L"Windows 10";
                } else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 3) {
                    name = L"Windows 8.1";
                } else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 2) {
                    name = L"Windows 8";
                } else if (rovi.dwMajorVersion == 6 && rovi.dwMinorVersion == 1) {
                    name = L"Windows 7";
                } else {
                    name = L"Unknown Windows";
                }

                return std::format(L"{} Build {}", name, rovi.dwBuildNumber);
            }
        }
    }

    OSVERSIONINFOEXW osVersionInfoW = {};
    osVersionInfoW.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    if (GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osVersionInfoW))) {
        std::wstring versionString;
        switch (osVersionInfoW.dwMajorVersion) {
            case 5:
                if (osVersionInfoW.dwMinorVersion == 0) {
                    versionString = L"Windows 2000";
                } else if (osVersionInfoW.dwMinorVersion == 1) {
                    versionString = L"Windows XP";
                } else if (osVersionInfoW.dwMinorVersion == 2) {
                    versionString = L"Windows Server 2003";
                }
                break;
            case 6:
                if (osVersionInfoW.dwMinorVersion == 0) {
                    if (osVersionInfoW.wProductType == VER_NT_WORKSTATION) {
                        versionString = L"Windows Vista";
                    } else {
                        versionString = L"Windows Server 2008";
                    }
                } else if (osVersionInfoW.dwMinorVersion == 1) {
                    if (osVersionInfoW.wProductType == VER_NT_WORKSTATION) {
                        versionString = L"Windows 7";
                    } else {
                        versionString = L"Windows Server 2008 R2";
                    }
                } else if (osVersionInfoW.dwMinorVersion == 2) {
                    if (osVersionInfoW.wProductType == VER_NT_WORKSTATION) {
                        versionString = L"Windows 8";
                    } else {
                        versionString = L"Windows Server 2012";
                    }
                } else if (osVersionInfoW.dwMinorVersion == 3) {
                    if (osVersionInfoW.wProductType == VER_NT_WORKSTATION) {
                        versionString = L"Windows 8.1";
                    } else {
                        versionString = L"Windows Server 2012 R2";
                    }
                }
                break;
            case 10:
                if (osVersionInfoW.dwMinorVersion == 0) {
                    if (osVersionInfoW.wProductType == VER_NT_WORKSTATION) {
                        versionString = L"Windows 10";
                    } else {
                        versionString = L"Windows Server 2016";
                    }
                } else if (osVersionInfoW.dwMinorVersion == 1) {
                    versionString = L"Windows Server 2019";
                } else if (osVersionInfoW.dwMinorVersion == 2) {
                    versionString = L"Windows 11";
                } else if (osVersionInfoW.dwMinorVersion == 3) {
                    versionString = L"Windows Server 2022";
                }
                break;
            default:
                versionString = L"Unknown Windows Version";
                break;
        }

        versionString += std::format(L" Build {}", osVersionInfoW.dwBuildNumber);
        return versionString;
    }

    return L"Failed to retrieve version information";
}

inline std::wstring GetSystemInfoString() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::wstring archString;
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            archString = L"x64 (AMD or Intel)";
        break;
        case PROCESSOR_ARCHITECTURE_ARM:
            archString = L"ARM";
        break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            archString = L"ARM64";
        break;
        case PROCESSOR_ARCHITECTURE_IA64:
            archString = L"Itanium-based";
        break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            archString = L"x86";
        break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
            default:
                archString = L"Unknown";
        break;
    }

    // Check if running under WOW64
    BOOL isWow64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &isWow64);

    if (isWow64) {
        // Running under WOW64, get native system info
        SYSTEM_INFO nativeSysInfo;
        ZeroMemory(&nativeSysInfo, sizeof(SYSTEM_INFO));
        GetNativeSystemInfo(&nativeSysInfo);

        switch (nativeSysInfo.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
                archString = L"x64 (AMD or Intel)";
            break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                archString = L"ARM64";
            break;
            case PROCESSOR_ARCHITECTURE_IA64:
                archString = L"Itanium-based";
            break;
            case PROCESSOR_ARCHITECTURE_UNKNOWN:
                default:
                    archString = L"Unknown";
            break;
        }
    }

    std::wstring result = std::format(
        L"{}, "
        L"{} Cores, "
        L"{} Bytes Page Size",
        archString,
        sysInfo.dwNumberOfProcessors,
        sysInfo.dwPageSize
    );

    return result;
}

inline std::wstring GetTotalPhysicalMemory() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        const ULONGLONG totalPhysicalMemory = memInfo.ullTotalPhys;
        return std::format(L"Total Physical Memory: {:.2f} GB", static_cast<double>(totalPhysicalMemory) / (1024 * 1024 * 1024));
    }
    return L"Failed to retrieve memory information";
}

inline std::wstring GetMemoryLoadPercentage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        DWORD memoryLoad = memInfo.dwMemoryLoad;
        return std::format(
            L"Free Memory: {:.2f}, Memory Load: {}%",
            static_cast<double>(memInfo.ullAvailPhys) / (1024 * 1024 * 1024), memoryLoad
        );
    }
    return L"Failed to retrieve memory information";
}

#endif
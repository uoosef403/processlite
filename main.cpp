#include <chrono>
#include <iostream>
#include <memory>
#include <windows.h>
#include <string>
#include <thread>
#include <unordered_map>

#include "ButtonManager.h"
#include "HelperFunctions.h"
#include "ListViewManager.h"
#include "ProcessInfo.h"
#include "ProcessMonitor.h"
#include "resource.h"
#include "SystemInfoPanel.h"
#include "TasksIDDef.h"
#include "Concurrency/Scheduler.h"
#include "Concurrency/TaskManager.h"
#include "Concurrency/ThreadPool.h"

using namespace std::chrono_literals;

class MainWindow {
public:
    explicit MainWindow(const HINSTANCE hInstance) : hInstance(hInstance), hwnd(nullptr) {
        initializeMessageHandlers();
    }

    ~MainWindow() {
        process_monitor_->stopMonitoring();
        task_manager_->stopAllTasks();
    }

    bool Create(const std::wstring &title, const int width = 800, const int height = 600) {
        constexpr wchar_t CLASS_NAME[] = L"MainWindowClass";

        WNDCLASSW wc = {};
        wc.lpfnWndProc = &MainWindow::WindowProcSetup;
        wc.hInstance = hInstance;
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = CLASS_NAME;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

        RegisterClassW(&wc);

        hwnd = CreateWindowExW(
            0,
            CLASS_NAME,
            title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            nullptr, nullptr, hInstance, this // pass `this` as lpParam
        );

        if (!hwnd) {
            return false;
        }

        return true;
    }

    void Show(const int nCmdShow) const {
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);
    }

    [[nodiscard]] HWND GetHwnd() const { return hwnd; }

private:
    HINSTANCE hInstance;
    HWND hwnd;

    std::unique_ptr<ListViewManager> list_view_manager_;
    std::unique_ptr<ProcessMonitor> process_monitor_;
    std::unique_ptr<ButtonManager> button_manager_;
    std::unique_ptr<SystemInfoPanel> system_info_panel_;
    std::unique_ptr<TaskManager> task_manager_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<Scheduler> scheduler_;
    using MessageHandler = std::function<LRESULT(HWND, WPARAM, LPARAM)>;
    std::unordered_map<UINT, MessageHandler> message_handlers_;

    // Initial WndProc setup to attach instance pointer
    static LRESULT CALLBACK WindowProcSetup(
        const HWND hwnd,
        const UINT msg,
        const WPARAM wParam,
        const LPARAM lParam
    ) {
        if (msg == WM_NCCREATE) {
            const auto createStruct = reinterpret_cast<CREATESTRUCT *>(lParam);
            auto instance = static_cast<MainWindow *>(createStruct->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance));
            SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(
                                  &MainWindow::WindowProcThunk
                              )
            );
            return instance->WndProc(hwnd, msg, wParam, lParam);
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    // Redirect messages to the member WndProc
    static LRESULT CALLBACK WindowProcThunk(
        const HWND hwnd,
        const UINT msg,
        const WPARAM wParam,
        const LPARAM lParam
    ) {
        const auto instance = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        return instance ? instance->WndProc(hwnd, msg, wParam, lParam) : DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    void fetchSystemInfo(HWND hwnd) const {
        const std::wstring osVersion = GetWindowsVersionString();
        PostMessage(hwnd, WM_APP + 100, 0, reinterpret_cast<LPARAM>(new std::wstring(osVersion)));

        // Fetch CPU Info (Example)
        const std::wstring cpuInfo = GetSystemInfoString();
        PostMessage(hwnd, WM_APP + 101, 0, reinterpret_cast<LPARAM>(new std::wstring(cpuInfo)));

        const std::wstring totalRam = GetTotalPhysicalMemory();
        PostMessage(hwnd, WM_APP + 102, 0, reinterpret_cast<LPARAM>(new std::wstring(totalRam)));
        try {
            task_manager_->addTask(
                systemInfoScheduledTaskID,
                "System Info Update", // Task name
                [this, hwnd](const std::stop_token &st) {
                    if (!st.stop_requested()) {
                        const std::wstring availRam = GetMemoryLoadPercentage();
                        // Post messages to update RAM
                        PostMessage(hwnd, WM_APP + 103, 0, reinterpret_cast<LPARAM>(new std::wstring(availRam)));
                    }
                },
                1000ms
            );
        } catch (...) {
            std::cerr << "Something went wrong when creating new task" << std::endl;
        }
    }

    // Initialize message handlers
    void initializeMessageHandlers() {
        message_handlers_[WM_PAINT] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handlePaint(hwnd, wp, lp);
        };
        message_handlers_[WM_CREATE] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleCreate(hwnd, wp, lp);
        };
        message_handlers_[WM_SIZE] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleSize(hwnd, wp, lp);
        };
        message_handlers_[WM_CLOSE] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleClose(hwnd, wp, lp);
        };
        message_handlers_[WM_DESTROY] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleDestroy(hwnd, wp, lp);
        };
        message_handlers_[WM_COMMAND] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleCommand(hwnd, wp, lp);
        };
        message_handlers_[WM_APP + 100] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleOsVersionUpdate(hwnd, wp, lp);
        };
        message_handlers_[WM_APP + 101] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleCpuInfoUpdate(hwnd, wp, lp);
        };
        message_handlers_[WM_APP + 102] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleTotalRamUpdate(hwnd, wp, lp);
        };
        message_handlers_[WM_APP + 103] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleAvailableRamUpdate(hwnd, wp, lp);
        };
        message_handlers_[WM_APP + 104] = [this](const HWND hwnd, const WPARAM wp, const LPARAM lp) {
            return handleProcessListUpdate(hwnd, wp, lp);
        };
    }

    // Message handlers
    static LRESULT handlePaint(const HWND hwnd, WPARAM, LPARAM) {
        PAINTSTRUCT ps;
        const HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
        return 0;
    }

    LRESULT handleCreate(HWND hwnd, WPARAM, LPARAM) {
        // Initialize controls
        thread_pool_ = std::make_unique<ThreadPool>(2);
        task_manager_ = std::make_unique<TaskManager>();
        scheduler_ = std::make_unique<Scheduler>(*task_manager_, *thread_pool_, 100ms);
        list_view_manager_ = std::make_unique<ListViewManager>(hwnd);
        button_manager_ = std::make_unique<ButtonManager>(hwnd);
        system_info_panel_ = std::make_unique<SystemInfoPanel>(hwnd);

        if (
            !list_view_manager_->Create() ||
            !button_manager_->Create() ||
            !system_info_panel_->Create(0, 300, 200, 50)
        ) {
            MessageBoxW(hwnd, L"Something went wrong!", L"ERROR", MB_ICONERROR);
            PostQuitMessage(0);
        }

        fetchSystemInfo(hwnd);
        process_monitor_ = std::make_unique<ProcessMonitor>(*task_manager_, hwnd, list_view_manager_->getHWND());
        return 0;
    }

    LRESULT handleSize(HWND, WPARAM, const LPARAM lParam) const {
        if (list_view_manager_) {
            list_view_manager_->Resize(LOWORD(lParam), HIWORD(lParam));
        }
        if (button_manager_) {
            button_manager_->Resize(LOWORD(lParam), HIWORD(lParam));
        }
        if (system_info_panel_) {
            system_info_panel_->Resize(LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    }

    static LRESULT handleClose(const HWND hwnd, WPARAM, LPARAM) {
        DestroyWindow(hwnd);
        return 0;
    }

    static LRESULT handleDestroy(HWND, WPARAM, LPARAM) {
        PostQuitMessage(0);
        return 0;
    }

    LRESULT handleCommand(HWND, const WPARAM wParam, LPARAM) const {
        this->onCommand(wParam);
        return 0;
    }

    LRESULT handleOsVersionUpdate(HWND, WPARAM, const LPARAM lParam) const {
        if (system_info_panel_) {
            const std::unique_ptr<std::wstring> data(reinterpret_cast<std::wstring *>(lParam));
            system_info_panel_->SetOsVersion(*data);
        }
        return 0;
    }

    LRESULT handleCpuInfoUpdate(HWND, WPARAM, const LPARAM lParam) const {
        if (system_info_panel_) {
            const std::unique_ptr<std::wstring> data(reinterpret_cast<std::wstring *>(lParam));
            system_info_panel_->SetCpuInfo(*data);
        }
        return 0;
    }

    LRESULT handleTotalRamUpdate(HWND, WPARAM, const LPARAM lParam) const {
        if (system_info_panel_) {
            const std::unique_ptr<std::wstring> data(reinterpret_cast<std::wstring *>(lParam));
            system_info_panel_->SetTotalRam(*data);
        }
        return 0;
    }

    LRESULT handleAvailableRamUpdate(HWND, WPARAM, const LPARAM lParam) const {
        if (system_info_panel_) {
            const std::unique_ptr<std::wstring> data(reinterpret_cast<std::wstring *>(lParam));
            system_info_panel_->SetAvailableRam(*data);
        }
        return 0;
    }

    LRESULT handleProcessListUpdate(HWND, WPARAM, const LPARAM lParam) const {
        if (list_view_manager_) {
            const auto data(reinterpret_cast<ProcessUpdateData *>(lParam));
            list_view_manager_->applyListViewUpdates(data);
        }
        return 0;
    }

    // WndProc implementation
    LRESULT WndProc(const HWND hwnd, const UINT msg, const WPARAM wParam, const LPARAM lParam) {
        if (const auto it = message_handlers_.find(msg); it != message_handlers_.end()) {
            // Call the corresponding handler
            return it->second(hwnd, wParam, lParam);
        }
        // Default handling
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    void onCommand(const WPARAM wParam) const {
        switch (LOWORD(wParam)) {
            case 1:
                MessageBoxW(hwnd, L"Button 1 clicked!", L"Info", MB_OK);
                break;
            case 2:
                MessageBoxW(hwnd, L"Button 2 clicked!", L"Info", MB_OK);
                break;
            case 3:
                PostQuitMessage(0); // Close the application
                break;
            default:
                MessageBoxW(hwnd, L"Invalid Command", L"ERROR", MB_ICONERROR);
        }
    }
};

int WINAPI wWinMain(const HINSTANCE hInstance, HINSTANCE, PWSTR, const int nShowCmd) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);

    SetProcessDPIAware(); // DPI awareness

    MainWindow window(hInstance);
    if (!window.Create(L"Process Viewer Lite")) {
        MessageBoxW(nullptr, L"Window creation failed", L"Error", MB_ICONERROR);
        return 0;
    }

    window.Show(nShowCmd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

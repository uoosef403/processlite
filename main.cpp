#include <memory>
#include <windows.h>
#include <string>

#include "ButtonManager.h"
#include "ListViewManager.h"
#include "resource.h"

class MainWindow {
public:
    explicit MainWindow(const HINSTANCE hInstance) : hInstance(hInstance), hwnd(nullptr) {}

    bool Create(const std::wstring& title, const int width = 800, const int height = 600) {
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

        // Initialize controls
        listViewManager = std::make_unique<ListViewManager>(hwnd);
        buttonManager = std::make_unique<ButtonManager>(hwnd);

        if (!listViewManager->Create() || !buttonManager->Create()) {
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

    std::unique_ptr<ListViewManager> listViewManager;
    std::unique_ptr<ButtonManager> buttonManager;

    // Initial WndProc setup to attach instance pointer
    static LRESULT CALLBACK WindowProcSetup(
        const HWND hwnd,
        const UINT msg,
        const WPARAM wParam,
        const LPARAM lParam
    ) {
        if (msg == WM_NCCREATE) {
            const auto createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            auto instance = static_cast<MainWindow*>(createStruct->lpCreateParams);
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
        const auto instance = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        return instance ? instance->WndProc(hwnd, msg, wParam, lParam) : DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    // Actual message handler
    LRESULT WndProc(
        const HWND hwnd,
        const UINT msg,
        const WPARAM wParam,
        const LPARAM lParam
    ) {
        switch (msg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                const HDC hdc = BeginPaint(hwnd, &ps);
                FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_COMMAND:
                this->onCommand(wParam);
            return 0;
            default:
                return DefWindowProcW(hwnd, msg, wParam, lParam);
        }
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

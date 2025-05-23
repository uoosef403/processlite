# Process Viewer Lite

Process Viewer Lite is a Windows desktop application developed in C++20 using the native Win32 API. The primary goal of this project is to gain a deep understanding of low-level Windows programming by implementing a system utility that displays information about running processes and overall system metrics.

This project is being built step-by-step, focusing on different aspects of the Win32 API, including Kernel32, User32, GDI32, and Advapi32.

## Key Features (Implemented so far)

* **Real-time Process Listing:** Displays a list of currently running processes with details such as:
    * Process Name [cite: 24]
    * Process ID (PID) [cite: 24]
    * CPU Usage (%) [cite: 21, 26]
    * Memory Usage (Working Set Size) [cite: 21, 26]
    * Disk I/O Rate (Read/Write Bytes per second) [cite: 21, 26]
    * Executable Path [cite: 24]
* **Process Icons:** Shows the icon associated with each process executable in the list view[cite: 27].
* **System Information Panel:** Displays key system metrics:
    * Operating System Version (e.g., "Windows 11 Build 22621") [cite: 9, 13, 12]
    * CPU Architecture and Core Count [cite: 10, 13]
    * Total Physical Memory (GB) [cite: 11, 13]
    * Current Memory Load (%) and Available Physical Memory (GB) [cite: 12, 13]
* **Dynamic UI Updates:**
    * The process list automatically refreshes to show new processes, terminate old ones, and update metrics for existing ones[cite: 23, 25].
    * System information (especially available RAM/memory load) updates periodically[cite: 7].
    * ListView columns resize proportionally when the main window is resized[cite: 25].
* **Modern C++ & Win32:**
    * Built with C++20, leveraging features like `std::jthread`, `std::stop_token`, smart pointers, and `<format>`.
    * Directly uses Win32 API calls for all core functionalities.
* **Modular Design:** Code is structured into classes for managing different UI components and functionalities (e.g., `MainWindow`, `ListViewManager`, `ButtonManager`, `SystemInfoPanel`, `ProcessMonitor`, `ProcessMetrics`, `HandleWrapper`)[cite: 1, 2, 4, 5, 14, 15, 16, 17, 18, 19, 20, 22].
* **Multi-threaded Backend:** Employs a custom task scheduler and thread pool for background tasks like process monitoring and system info updates, ensuring a responsive UI[cite: 6, 7, 8].
* **Basic UI Controls:** Includes placeholder buttons ("New Process", "Kill Process") and a "Close" button[cite: 2, 3].

## Technology Stack

* **Language:** C++20
* **API:** Native Win32 API
* **Build System:** CMake (assumed, based on project structure)
* **Compiler:** MSVC (Visual Studio) or any C++20 compliant compiler.

## Project Goals & Learning Focus

This project serves as a practical learning exercise to become proficient with a wide array of Windows APIs from `KERNEL32.DLL`, `USER32.DLL`, `GDI32.DLL`, and `ADVAPI32.DLL`. Each step aims to incorporate and understand specific functions within a real-world context.

## Current Status

The application currently provides a functional process viewer with key metrics and a system information panel. Development is ongoing, with further features planned to explore more Win32 APIs.

## Building the Project (Example using CMake)

1.  Ensure you have installed a C++20-compliant compiler (e.g., Visual Studio with MSVC) and CMake.
2.  Clone the repository:
    ```bash
    git clone [https://github.com/uoosef403/processlite.git](https://github.com/uoosef403/processlite.git)
    ```
3.  Navigate to the project directory:
    ```bash
    cd processlite
    ```
4.  Create a build directory and navigate into it:
    ```bash
    mkdir build
    cd build
    ```
5.  Configure the project with CMake:
    ```bash
    cmake .. 
    # Or, for a specific generator, e.g., Visual Studio 2022:
    # cmake .. -G "Visual Studio 17 2022" -A x64
    ```
6.  Build the project:
    ```bash
    cmake --build . --config Release
    # Or open the generated solution file in Visual Studio and build.
    ```
7.  The executable will typically be in `build/Release` or `build/source/Release`.

## Future Enhancements (Planned Learning Steps)

* **Process Module Viewing:** Displaying DLLs loaded by a selected process.
* **Process Termination:** Implementing the "Kill Process" functionality.
* **Thread Listing and Information:** For a selected process.
* **Enhanced Process Details:** More in-depth information (command line, user, etc.).
* **Registry Interaction:** Reading/displaying specific registry values.
* **File System Operations:** Exploring Win32 file APIs.
* **GDI Custom Drawing:** For specific UI elements or data visualization.
* **Privilege Management:** Handling operations that require elevated privileges.

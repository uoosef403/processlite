#ifndef HandleWrapper_h
#define HandleWrapper_h

#include <Windows.h>

class HandleWrapper {
    HANDLE handle_ = INVALID_HANDLE_VALUE;
public:
    explicit HandleWrapper(const HANDLE h = INVALID_HANDLE_VALUE) : handle_(h) {}
    ~HandleWrapper() { if (handle_ != INVALID_HANDLE_VALUE && handle_ != nullptr) CloseHandle(handle_); }
    HandleWrapper(const HandleWrapper&) = delete;
    HandleWrapper& operator=(const HandleWrapper&) = delete;
    HandleWrapper(HandleWrapper&& other) noexcept : handle_(other.handle_) { other.handle_ = INVALID_HANDLE_VALUE; }
    HandleWrapper& operator=(HandleWrapper&& other) noexcept {
        if (this != &other) {
            if (handle_ != INVALID_HANDLE_VALUE && handle_ != nullptr) CloseHandle(handle_);
            handle_ = other.handle_;
            other.handle_ = INVALID_HANDLE_VALUE;
        }
        return *this;
    }
    operator HANDLE() const { return handle_; }
    bool isValid() const { return handle_ != INVALID_HANDLE_VALUE && handle_ != nullptr; }
    HANDLE get() const { return handle_; }
};

#endif
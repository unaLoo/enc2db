#pragma once

#include <cstdio>
#include <iostream>
#include <cerrno>
#include <cstdarg>
#include <cstring>

#ifdef _WIN32
    #include <conio.h>  // Windows 原生 getch()
    #include <string.h> // Windows 下 C 函数可能在 std::
    #define STRPBRK std::strpbrk
#else
    #define STRPBRK strpbrk
    // Linux 下实现 getch() 类似 Windows 的行为
    inline int getch() {
        return std::cin.get();
    }

    // Linux 下模拟 fopen_s
    inline int fopen_s(FILE** pFile, const char* filename, const char* mode) {
        if (!pFile) return EINVAL;
        *pFile = std::fopen(filename, mode);
        return (*pFile) ? 0 : errno;
    }

    // Linux 下模拟 sprintf_s
    //inline int sprintf_s(char* buffer, size_t size, const char* format, ...) {
    //    if (!buffer || size == 0 || !format) return EINVAL;

    //    va_list args;
    //    va_start(args, format);
    //    int ret = vsnprintf(buffer, size, format, args); // 安全版本
    //    va_end(args);

    //    // 如果输出超过 buffer，返回错误
    //    if (ret < 0 || static_cast<size_t>(ret) >= size) return ERANGE;

    //    return ret;
    //}
    template<size_t N>
    inline int sprintf_s(char(&buffer)[N], const char* format, ...) {
        if (!buffer || N == 0 || !format) return EINVAL;

        va_list args;
        va_start(args, format);
        int ret = vsnprintf(buffer, N, format, args);
        va_end(args);

        if (ret < 0 || static_cast<size_t>(ret) >= N) return ERANGE;
        return ret;
    }
#endif
#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN

#define NOMINMAX // Use the STL min/max

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <memory>
#include <cassert>
#include <string>
/*
#include <algorithm>
#include <exception>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstdio>
*/

namespace kdx
{
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) noexcept : result(hr) {}
        const char *what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "FAIL (HRESULT %08X)", static_cast<unsigned int>(result));
            return s_str;
        }
    private:
        HRESULT result;
    };

    inline void dx_assert(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}

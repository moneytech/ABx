/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#if defined(_MSC_VER)
#pragma once
#endif

#include "targetver.h"
#include <sa/PragmaWarning.h>

#include <stdio.h>
#include <cassert>
#include <ctype.h>

// Suppress min/max conflicts with STL. For further information visit: http://support.microsoft.com/kb/143208
#ifndef NOMINMAX
#   define NOMINMAX
#endif

#include <sys/timeb.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif

#define USE_STANDALONE_ASIO

PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4592)
#include <asio.hpp>
PRAGMA_WARNING_POP

#include <base64.h>

#define AB_UNUSED(P) (void)(P)

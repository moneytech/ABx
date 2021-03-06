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

#pragma once

#define PRAGMA_STRINGIFY(a) #a

#if defined(_MSC_VER)
#   define PRAGMA_WARNING_PUSH __pragma(warning(push))
#   define PRAGMA_WARNING_POP __pragma(warning(pop))
#   define PRAGMA_WARNING_DISABLE_MSVC(id) __pragma(warning(disable: id))
#   define PRAGMA_WARNING_DISABLE_GCC(id)
#   define PRAGMA_WARNING_DISABLE_CLANG(id)
#elif defined(__GNUC__)
#   define PRAGMA_WARNING_PUSH _Pragma("GCC diagnostic push")
#   define PRAGMA_WARNING_POP _Pragma("GCC diagnostic pop")
#   define PRAGMA_WARNING_DISABLE_MSVC(id)
#   define PRAGMA_WARNING_DISABLE_GCC(id) _Pragma(PRAGMA_STRINGIFY(GCC diagnostic ignored id))
#   define PRAGMA_WARNING_DISABLE_CLANG(id)
#elif defined(__clang__)
#   define PRAGMA_WARNING_PUSH _Pragma("clang diagnostic push")
#   define PRAGMA_WARNING_POP _Pragma("clang diagnostic pop")
#   define PRAGMA_WARNING_DISABLE_MSVC(id)
#   define PRAGMA_WARNING_DISABLE_GCC(id)
#   define PRAGMA_WARNING_DISABLE_CLANG(id) _Pragma(PRAGMA_STRINGIFY(clang diagnostic ignored id))
#endif

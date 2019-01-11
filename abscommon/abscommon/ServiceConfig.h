#pragma once

// If WIN_SERVICE is defined it creates a windows service instead of a console application.
#if defined(_WIN32)
#   if !defined(WIN_SERVICE)
//#       define WIN_SERVICE
#   endif
#elif defined(WIN_SERVICE)
#   undef WIN_SERVICE
#endif
#include "stdafx.h"
#include <sstream>
#include <fstream>
#include "SimpleConfigManager.h"
#include "Application.h"
#include "Logger.h"
#include "Version.h"
#include "MiniDump.h"
#include <csignal>

#if defined(_MSC_VER) && defined(_DEBUG)
#   define CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#endif

namespace {
std::function<void(int)> shutdown_handler;
void signal_handler(int signal)
{
    shutdown_handler(signal);
}
} // namespace

static void ShowLogo()
{
    std::cout << "This is " << SERVER_PRODUCT_NAME << std::endl;
    std::cout << "Version " << SERVER_VERSION_MAJOR << "." << SERVER_VERSION_MINOR <<
        " (" << __DATE__ << " " << __TIME__ << ")";
#ifdef _DEBUG
    std::cout << " DEBUG";
#endif
    std::cout << std::endl;
    std::cout << "(C) 2017-" << SERVER_YEAR << std::endl;
    std::cout << std::endl;

    std::cout << "##########  ######  ######" << std::endl;
    std::cout << "    ##          ##  ##" << std::endl;
    std::cout << "    ##  ######  ##  ##" << std::endl;
    std::cout << "    ##  ##      ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;
    std::cout << "    ##  ##  ##  ##  ##" << std::endl;

    std::cout << std::endl;
}

#ifdef _WIN32
static std::mutex gTermLock;
static std::condition_variable termSignal;
#endif

int main(int argc, char** argv)
{
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#if defined(_MSC_VER) && defined(WRITE_MINIBUMP)
    SetUnhandledExceptionFilter(System::UnhandledHandler);
#endif

    std::signal(SIGINT, signal_handler);              // Ctrl+C
    std::signal(SIGTERM, signal_handler);
#ifdef _WIN32
    std::signal(SIGBREAK, signal_handler);            // X clicked
#endif

    ShowLogo();

    {
        std::shared_ptr<Application> app = std::make_shared<Application>();
        if (!app->Initialize(argc, argv))
            return EXIT_FAILURE;

        shutdown_handler = [&](int /*signal*/)
        {
#ifdef _WIN32
            std::unique_lock<std::mutex> lockUnique(gTermLock);
#endif
            app->Stop();
#ifdef _WIN32
            termSignal.wait(lockUnique);
#endif
        };
        app->Run();
    }

#ifdef _WIN32
    termSignal.notify_all();
#endif

    return EXIT_SUCCESS;
}
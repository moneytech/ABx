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

#include "Version.h"
#include <abscommon/ServerApp.h>
#if defined(SCENE_VIEWER)
#include "SceneViewer.h"
#endif
#include <numeric>
#include <sa/CircularQueue.h>
#include <asio.hpp>
#include <mutex>

namespace Net {
class MessageMsg;
class ServiceManager;
}

class MessageDispatcher;
class Maintenance;

class Application final : public ServerApp
{
private:
    asio::io_service ioService_;
    std::mutex lock_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    std::unique_ptr<MessageDispatcher> msgDispatcher_;
    sa::CircularQueue<unsigned, 10> loads_;
    int64_t lastLoadCalc_{ 0 };
    std::unique_ptr<Maintenance> maintenance_;
#if defined(SCENE_VIEWER)
    std::shared_ptr<Debug::SceneViewer> sceneViewer_;
#endif
    bool LoadMain();
    void ShowLogo();
    void PrintServerInfo();
    void HandleMessage(const Net::MessageMsg& msg);
    void HandleCreateInstanceMessage(const Net::MessageMsg& msg);
    unsigned GetAvgLoad() const
    {
        if (loads_.IsEmpty())
            return 0;
        return std::accumulate(loads_.begin(), loads_.end(), 0u) / static_cast<unsigned>(loads_.Size());
    }
protected:
    bool ParseCommandLine() override;
    void ShowVersion() override;
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;

    std::string GetKeysFile() const;
    /// Returns a value between 0..100
    unsigned GetLoad();

    void SpawnServer();

    bool autoTerminate_{ false };
    bool temporary_{ false };

    static Application* Instance;
};

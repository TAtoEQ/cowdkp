#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <map>
#include <thread>
#include <vector>
#include <regex>
#include <fmt/core.h>

#include "detours.h"
#include "logparser.h"
#include "logcallbacks.h"
#include <game.h>
#include "auction.h"
#include "settings.h"

HINSTANCE DllHandle;

using namespace std::chrono_literals;

DWORD __stdcall EjectThread(LPVOID lpParameter)
{
    Sleep(100);
    FreeLibraryAndExitThread(DllHandle, 0);
    return 0;
}

DWORD WINAPI Menu(HINSTANCE hModule)
{
    AllocConsole();
    FILE* fp;
    FILE* fperr;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fperr, "CONERR$", "w", stderr);
    LogParser parser;
    std::thread parserThread;
    auto lastUpdate = std::chrono::high_resolution_clock::now();

    if (!settings::load())
    {
        goto cleanup;
    }

    Game::hook({ "CommandFunc" });

    parserThread = std::thread(&LogParser::start, std::ref(parser), logCallbacks);

    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> timeDiff = now - lastUpdate;
        float dt = timeDiff.count();
        lastUpdate = now;
        
        Auction::updateAuctions(dt);

        if (GetAsyncKeyState(VK_F7) & 1)
        {
            break;
        }
        std::this_thread::sleep_for(.1s);
    }
    parser.stop();
    parserThread.join();

    Game::unhook();

cleanup:
    FreeConsole();
    if (fp) fclose(fp);
    if (fperr) fclose(fperr);
    CreateThread(0, 0, EjectThread, 0, 0, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DllHandle = hModule;
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Menu, NULL, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


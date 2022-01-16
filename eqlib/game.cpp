#include <fmt/core.h>
#include <Windows.h>
#include <map>
#include <string>
#include "detours.h"
#include "game.h"
#include <fstream>


int Game::findPattern(char* addr, int size, const char* pattern) noexcept
{
    std::size_t len = std::strlen(pattern);
    if (len <= 0) return 0;

    for (char* end = addr + size; addr < end; ++addr)
    {
        bool found = true;
        for (std::size_t i = 0; i < len; ++i)
        {
            if (!(pattern[i] == '.' || addr[i] == pattern[i]))
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return (int)addr;
        }
    }
    return 0;
}

void __fastcall Game::hookedCommandFunc(int eq, void* unk, int* p, const char* s)
{
    if (eq == 0 || p == nullptr)
    {
        int base = (int)GetModuleHandle(nullptr);
        int addr = base + Offsets::EQ::INST_ADDR;
        eq = *(int*)addr;
        addr = base + Offsets::EQ::CHAR_ADDR;
        p = *(int**)addr;
    }
    Game::eqInst = eq;
    Game::charInfo = p;
    if (commandFuncCallback)
    {
        std::string r = commandFuncCallback(eq, p, s);
        if (r.length() > 0)
            fnCommandFunc(eq, p, r.c_str());
    }
    else
    {
        fnCommandFunc(eq, p, s);
    }
}

void __fastcall Game::hookedItemLinkFunc(void* item, void* unk, char* buffer, int size, bool unk2)
{
    if (item == nullptr || buffer == nullptr) return;
    fnItemLinkFunc(item, buffer, size, unk2);
}

int __fastcall Game::hookedRaidGroupFunc(void* window, void* unk, int* a, int b, int* c)
{
    if (window == nullptr || a == nullptr)
    {
        return 0;
    }
    return fnRaidGroupFunc((int)window, a, b, c);
}

int __fastcall Game::hookedRaidSelectFunc(void* t, void* unk, int a)
{
    return fnRaidSelectFunc((int)t, a);
}

void Game::hook(const std::vector<std::string>& funcs) noexcept
{
    try
    {
        int base = (int)GetModuleHandle(nullptr);
        for (const auto& s : funcs)
        {
            if (s == "CommandFunc")
            {
                int addr = findPattern((char*)base, Patterns::SEARCH_SIZE, Patterns::COMMAND_FUNC_PATTERN);
                if (addr > 0)
                {
                    fnCommandFunc = (CommandFuncT)DetourFunction((PBYTE)addr, (PBYTE)hookedCommandFunc);
                }
                else
                {
                    fmt::print("Unable to find CommandFunc\n");
                }
            }
            else if (s == "ItemLinkFunc")
            {
                int addr = findPattern((char*)base, Patterns::SEARCH_SIZE, Patterns::ITEMLINK_FUNC_PATTERN);
                if (addr > 0)
                    fnItemLinkFunc = (ItemLinkFuncT)DetourFunction((PBYTE)addr, (PBYTE)hookedItemLinkFunc);
                else
                {
                    fmt::print("Unable to find ItemLinkFunc\n");
                }
            }
            else if (s == "RaidGroupFunc")
            {
                int addr = findPattern((char*)base, Patterns::SEARCH_SIZE, Patterns::RAIDGROUP_FUNC_PATTERN);
                if (addr > 0)
                {
                    fnRaidGroupFunc = (RaidGroupFuncT)DetourFunction((PBYTE)addr, (PBYTE)hookedRaidGroupFunc);
                }
                else
                {
                    fmt::print("Unable to find RaidGroupFunc\n");
                }
            }
        }
    }
    catch (const std::exception&)
    {
    }
}

void Game::unhook() noexcept
{
    if (fnCommandFunc) DetourRemove((PBYTE)fnCommandFunc, (PBYTE)hookedCommandFunc);
    if (fnItemLinkFunc) DetourRemove((PBYTE)fnItemLinkFunc, (PBYTE)hookedItemLinkFunc);
    if (fnRaidGroupFunc) DetourRemove((PBYTE)fnRaidGroupFunc, (PBYTE)hookedRaidGroupFunc);
    if (fnRaidSelectFunc) DetourRemove((PBYTE)fnRaidSelectFunc, (PBYTE)hookedRaidSelectFunc);
}


std::string Game::findLinkForItem(const std::string& item) noexcept
{
    MEMORY_BASIC_INFORMATION info;
    auto access = PAGE_READONLY | PAGE_READWRITE;
    std::vector<unsigned char> memory;
    for (uint64_t addr = 0; addr < 0x6fffffffULL;)
    {
        memset(&info, 0, sizeof(info));
        auto written = VirtualQueryEx((HANDLE)-1, (LPCVOID)addr, &info, sizeof(info));
        if (written == 0)
        {
            break;
        }

        if (info.State == MEM_COMMIT && ((info.Protect & PAGE_GUARD) != PAGE_GUARD) && (info.Protect & access) != 0)
        {   
            if (memory.capacity() < info.RegionSize)
            {
                memory.clear();
                memory.reserve(info.RegionSize);
            }
            SIZE_T read;
            if (ReadProcessMemory((HANDLE)-1, info.BaseAddress, &memory[0], info.RegionSize, &read))
            {
                for (size_t start = 0; start < read - 93; ++start)
                {
                    char c = memory[start];
                    if (c == '\x12')
                    {
                        size_t nameInd = start + 92;
                        size_t x12Ind = nameInd + item.length();
                        if (memory[x12Ind] == '\x12' && item == std::string((char*)&memory[nameInd], item.length()))
                        {
                            return std::string((char*)&memory[start], item.length() + 93);
                        }
                    }
                }
            }
        }
        addr = (uint64_t)info.BaseAddress + info.RegionSize;
    }
    return "";
}
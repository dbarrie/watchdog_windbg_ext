
#include <engextcpp.hpp>

enum LogCategory
{
    LC_CRITICAL,
    LC_ASSERTION,
    LC_ERROR,
    LC_WARNING,
    LC_EVENT,
    LC_TRACE,
    LC_LOWRESOURCE,
    LC_DMMEVENT,
    LC_PRESENTTOKENEVENT,
    LC_POWER,
    LC_DEBUG
};

class EXT_CLASS : public ExtExtension
{
public:
    EXT_COMMAND_METHOD(wdcfg);
    EXT_COMMAND_METHOD(wdlive);
    EXT_COMMAND_METHOD(wdcurr);

    const static size_t kNumCategories = 11;
    const static char* gCategoryNames[kNumCategories];
};

EXT_DECLARE_GLOBALS();

const char* Extension::gCategoryNames[Extension::kNumCategories] =
{
    "Critical",
    "Assertion",
    "Error",
    "Warning",
    "Event",
    "Trace",
    "LowResource",
    "DmmEvent",
    "PresentTokenEvent",
    "Power",
    "Debug"
};

EXT_COMMAND(wdcfg,
    "Configure Watchdog.sys category behavior",
    "{;en=(16),o,d=0;log;Log categories}{;en=(16),o,d=7;dbg;Debug Break categories}")
{
    UINT64 logCategories = GetUnnamedArgU64(0);
    UINT64 dbgCategories = GetUnnamedArgU64(1);

    Out("Configuring Watchdog categories...\n");

    ULONG64 addr;
    GetExpr64("watchdog!WdLogRoot", false, ~0ULL, &addr);

    for (size_t i = 0; i < kNumCategories; ++i)
    {
        bool enableLog = (logCategories & (1ULL << i)) != 0;
        bool enableDbg = (dbgCategories & (1ULL << i)) != 0;

        ULONG64 categoryOffset = addr + 0x18 * i;
        ULONG64 dbgOffset = categoryOffset + 0x10;
        ULONG64 logOffset = categoryOffset + 0x11;

        ExtRemoteData DebugFlag("DebugFlag", dbgOffset, sizeof(BOOLEAN));
        ExtRemoteData LogFlag("LogFlag", logOffset, sizeof(BOOLEAN));

        bool prevEnableLog = LogFlag.GetBoolean();
        bool prevEnableDbg = DebugFlag.GetBoolean();

        DebugFlag.SetBoolean(enableDbg);
        LogFlag.SetBoolean(enableLog);

        Out("  % 18s: %s %s (was %s %s)\n", 
            gCategoryNames[i], 
            enableLog ? "Log" : "   ", enableDbg ? "Break" : "     ",
            prevEnableLog ? "Log" : "   ", prevEnableDbg ? "Break" : "     ");
    }
}

EXT_COMMAND(wdlive,
            "Dump Watchdog.sys live event",
            "")
{
    ULONG64 addr;
    GetExpr64("watchdog!WdLogLiveDumpInfo", false, ~0ULL, &addr);

    ExtRemoteData CategoryMem("Category", addr + 0x00, sizeof(ULONG));
    ExtRemoteData EventOrderMem("EventOrder", addr + 0x08, sizeof(ULONG64));
    ExtRemoteData ThreadMem("Thread", addr + 0x10, sizeof(ULONG64));
    ExtRemoteData AddressMem("Address", addr + 0x18, sizeof(ULONG64));
    ExtRemoteData Data0Mem("Data0", addr + 0x20, sizeof(ULONG64));
    ExtRemoteData Data1Mem("Data1", addr + 0x28, sizeof(ULONG64));
    ExtRemoteData Data2Mem("Data2", addr + 0x30, sizeof(ULONG64));
    ExtRemoteData Data3Mem("Data3", addr + 0x38, sizeof(ULONG64));
    ExtRemoteData Data4Mem("Data4", addr + 0x40, sizeof(ULONG64));

    LogCategory category = (LogCategory)CategoryMem.GetUlong();

    Out("Watchdog Live Event:\n");
    Out("% 18s #% 08d thread=% 016p addr=% 016p\n", 
        gCategoryNames[category],
        EventOrderMem.GetUlong64(),
        ThreadMem.GetUlong64(),
        AddressMem.GetUlong64());

    ULONG64 data[5];
    data[0] = Data0Mem.GetUlong64();
    data[1] = Data1Mem.GetUlong64();
    data[2] = Data2Mem.GetUlong64();
    data[3] = Data3Mem.GetUlong64();
    data[4] = Data4Mem.GetUlong64();

    Out("                       Data: % 016p % 016p % 016p % 016p % 016p\n",
        data[0], data[1], data[2], data[3], data[4]);

    /*
    switch (category)
    {
    LC_ERROR:
        
    default:
        Out("                       Data: % 016p % 016p % 016p % 016p % 016p\n",
            data[0], data[1], data[2], data[3], data[4]);
        break;
    }
    */
}

EXT_COMMAND(wdcurr,
    "Dump Watchdog.sys current event",
    "")
{
    ULONG64 addr;
    GetExpr64("@rbx", false, ~0ULL, &addr);

    ExtRemoteData EventOrderMem("EventOrder", addr + 0x00, sizeof(ULONG64));
    ExtRemoteData ThreadMem("Thread", addr + 0x08, sizeof(ULONG64));
    ExtRemoteData AddressMem("Address", addr + 0x10, sizeof(ULONG64));
    ExtRemoteData Data0Mem("Data0", addr + 0x18, sizeof(ULONG64));
    ExtRemoteData Data1Mem("Data1", addr + 0x20, sizeof(ULONG64));
    ExtRemoteData Data2Mem("Data2", addr + 0x28, sizeof(ULONG64));
    ExtRemoteData Data3Mem("Data3", addr + 0x30, sizeof(ULONG64));
    ExtRemoteData Data4Mem("Data4", addr + 0x38, sizeof(ULONG64));

    Out("Watchdog Current Event:\n");
    Out("#% 08d thread=% 016p addr=% 016p\n",
        EventOrderMem.GetUlong64(),
        ThreadMem.GetUlong64(),
        AddressMem.GetUlong64());

    ULONG64 data[5];
    data[0] = Data0Mem.GetUlong64();
    data[1] = Data1Mem.GetUlong64();
    data[2] = Data2Mem.GetUlong64();
    data[3] = Data3Mem.GetUlong64();
    data[4] = Data4Mem.GetUlong64();

    Out("Data: % 016p % 016p % 016p % 016p % 016p\n",
        data[0], data[1], data[2], data[3], data[4]);
}

// Behaviour tests for the bounded-retry component: Start, StartOrWake (wake
// during the interval, forced attempt after applied, restart after finish,
// coalesced wakes) and StopRequested. Build with Windhawk's clang, -static:
//   clang++ -std=c++20 -O1 -static bounded-retry-tests.cpp -o retry.exe
#include <windows.h>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <memory>
#include <mutex>
#include <utility>

namespace retry {
#include "../components/bounded-retry/body.h"
}

static std::atomic<int> g_attempts{0};
static std::atomic<bool> g_applied{false};
static std::atomic<bool> g_unloading{false};
static std::atomic<int> g_stopSeen{0};

static void Attempt() { ++g_attempts; }
static bool Applied() { return g_applied.load(); }

static void SlowAttempt() {
    ++g_attempts;
    for (int i = 0; i < 100; ++i) {
        if (retry::RetryLoop::StopRequested()) { ++g_stopSeen; return; }
        Sleep(10);
    }
}

int main() {
    // 1. Plain Start: first attempt at once, then one per interval, bounded.
    {
        retry::RetryLoop loop;
        g_attempts = 0; g_applied = false;
        loop.Start(Attempt, Applied, g_unloading, 3, 50);
        Sleep(400);
        assert(g_attempts == 3);
        loop.Stop();
    }
    // 2. Wake during the interval runs an attempt now, not after 2 s.
    {
        retry::RetryLoop loop;
        g_attempts = 0; g_applied = false;
        loop.Start(Attempt, Applied, g_unloading, 5, 2000);
        Sleep(100);
        assert(g_attempts == 1);
        ULONGLONG t = GetTickCount64();
        loop.StartOrWake(Attempt, Applied, g_unloading, 5, 2000);
        assert(GetTickCount64() - t < 50);  // never waits
        Sleep(100);
        assert(g_attempts == 2);
        loop.Stop();
    }
    // 3. A wake after `applied` reports done still runs (forced), then the
    //    loop settles again; and a wake after it FINISHED starts a new run.
    {
        retry::RetryLoop loop;
        g_attempts = 0; g_applied = true;
        loop.Start(Attempt, Applied, g_unloading, 5, 50);
        Sleep(100);
        assert(g_attempts == 0);            // applied: nothing to do
        loop.StartOrWake(Attempt, Applied, g_unloading, 5, 50);
        Sleep(100);
        assert(g_attempts == 1);            // forced first attempt, then settles
        Sleep(200);
        assert(g_attempts == 1);
        loop.Stop();
    }
    // 4. Stop interrupts a long attempt through StopRequested.
    {
        retry::RetryLoop loop;
        g_attempts = 0; g_applied = false; g_stopSeen = 0;
        loop.Start(SlowAttempt, Applied, g_unloading, 5, 50);
        Sleep(50);
        ULONGLONG t = GetTickCount64();
        loop.Stop();
        assert(GetTickCount64() - t < 200);
        assert(g_stopSeen == 1);
    }
    // 5. Repeated wakes while one attempt runs coalesce into at most one more.
    {
        retry::RetryLoop loop;
        g_attempts = 0; g_applied = false;
        loop.Start(SlowAttempt, Applied, g_unloading, 1, 50);
        Sleep(20);
        for (int i = 0; i < 5; ++i)
            loop.StartOrWake(SlowAttempt, Applied, g_unloading, 1, 50);
        Sleep(2500);
        assert(g_attempts == 2);
        loop.Stop();
    }
    std::puts("RETRY_TESTS_OK");
    return 0;
}

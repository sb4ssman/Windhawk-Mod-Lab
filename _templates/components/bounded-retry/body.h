// ---- Bounded retry ----------------------------------------------------------
//
// Stoppable and WAITED during unload. A detached thread that outlives
// Wh_ModUninit runs mod code out of an unloaded DLL.
//
// STOP IS CALLED FROM MORE THAN ONE THREAD. Wh_ModUninit stops the loop from
// Windhawk's thread while an Explorer taskbar rebuild can be starting it from
// the taskbar's UI thread, and Start() stops the previous run before it begins
// a new one. So the handles cannot live in bare members that each caller
// closes: two callers would read the same handle and close it twice, and in
// explorer.exe a double CloseHandle later closes whatever unrelated handle the
// value was recycled into.
//
// One attempt therefore owns its handles through a shared Run, and EVERY
// caller that observes a live Run waits for it. The mutex is held only across
// the handoff, never across the wait: the retry thread marshals onto the UI
// thread with SendMessage, so a UI-thread caller blocked on the mutex while
// another thread waited under it could never service that message.
//
// Start() itself is not serialized against a concurrent Start(), because both
// of this mod's callers run on the taskbar's UI thread.

//! LAB NOTE - stripped on assembly.
//!
//! The shared-Run shape exists because the first extraction of this loop
//! dropped the SRWLOCK that the pre-template per-mod code had, and the AI
//! review of PR #5569 caught it: Stop() from Wh_ModUninit races Start() from
//! the TrayUI::StartTaskbar hook, both read the same handles, both close them.
//!
//! Two shapes were rejected before this one:
//!   - hold a mutex across the wait. Deadlocks: if the Windhawk thread holds
//!     it while pumping, a UI-thread caller blocks on the mutex and can never
//!     service the retry thread's SendMessage.
//!   - swap the handles to a local under the lock and let the winner close
//!     them. The LOSER then returns from Stop() without waiting, so
//!     Wh_ModUninit can return while the worker is still running mod code.

class RetryLoop {
public:
    // applied: has the work finished? unloading: stop immediately.
    using AppliedFn = bool (*)();
    using AttemptFn = void (*)();

    // No destructor on purpose. A namespace-scope loop's destructor would run
    // at DLL detach, inside the loader lock, and Stop() waits on a thread —
    // the owner stops it explicitly from Wh_ModUninit instead.

    void Start(AttemptFn attempt, AppliedFn applied,
               std::atomic<bool> const& unloading, int attempts = 5,
               DWORD intervalMs = 2000, bool forceFirstAttempt = false) {
        Stop();
        if (unloading) return;

        auto run = std::make_shared<Run>();
        run->attempt = attempt;
        run->applied = applied;
        run->unloading = &unloading;
        run->attempts = attempts;
        run->intervalMs = intervalMs;
        run->forceFirstAttempt = forceFirstAttempt;
        run->stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!run->stopEvent) return;  // ~Run closes nothing it did not create

        // The thread carries a reference of its own, so the Run survives until
        // both the loop and the thread are done with it, whichever ends first.
        auto* parameter = new std::shared_ptr<Run>(run);
        run->thread =
            CreateThread(nullptr, 0, ThreadMain, parameter, 0, nullptr);
        if (!run->thread) {
            delete parameter;
            return;
        }

        {
            std::lock_guard<std::mutex> guard(mutex_);
            if (!unloading) {
                run_ = std::move(run);
                return;
            }
        }
        // Unload began while this attempt was being created, so the Stop that
        // would have waited for it saw nothing. Wait for it here instead.
        StopRun(run);
    }

    void Stop() {
        std::shared_ptr<Run> run;
        {
            std::lock_guard<std::mutex> guard(mutex_);
            run = run_;  // shared, not moved: a concurrent Stop must wait too
        }
        if (!run) return;
        StopRun(run);
        std::lock_guard<std::mutex> guard(mutex_);
        if (run_ == run) run_.reset();
    }

private:
    struct Run {
        HANDLE thread = nullptr;
        HANDLE stopEvent = nullptr;
        AttemptFn attempt = nullptr;
        AppliedFn applied = nullptr;
        std::atomic<bool> const* unloading = nullptr;
        int attempts = 5;
        DWORD intervalMs = 2000;
        bool forceFirstAttempt = false;

        // Closed exactly once, when the last of the loop and the thread lets
        // go. Both have already stopped using them by then.
        ~Run() {
            if (thread) CloseHandle(thread);
            if (stopEvent) CloseHandle(stopEvent);
        }
    };

    static DWORD WINAPI ThreadMain(void* parameter) {
        auto* owned = static_cast<std::shared_ptr<Run>*>(parameter);
        std::shared_ptr<Run> run = *owned;
        delete owned;
        for (int i = 0; i < run->attempts && !*run->unloading; ++i) {
            // A settings reload can need one restore/reapply pass even while
            // `applied` truthfully says we still own live XAML. Do not
            // overload that ownership flag merely to wake the retry loop;
            // request a forced first attempt instead.
            if (run->applied && !(run->forceFirstAttempt && i == 0) &&
                run->applied())
                break;
            if (i && WaitForSingleObject(run->stopEvent, run->intervalMs) !=
                         WAIT_TIMEOUT)
                break;
            if (run->attempt) run->attempt();
        }
        return 0;
    }

    // Signal and wait, pumping sent messages: a caller on the taskbar's UI
    // thread would otherwise deadlock against the SendMessage the retry thread
    // is making back to it. Idempotent — the stop event is manual-reset, and
    // waiting on an already-exited thread returns at once.
    static void StopRun(std::shared_ptr<Run> const& run) {
        if (run->stopEvent) SetEvent(run->stopEvent);
        if (!run->thread) return;
        DWORD result;
        do {
            HANDLE thread = run->thread;
            result = MsgWaitForMultipleObjects(1, &thread, FALSE, INFINITE,
                                               QS_SENDMESSAGE);
            if (result == WAIT_OBJECT_0 + 1) {
                MSG message;
                PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
            }
        } while (result == WAIT_OBJECT_0 + 1);
    }

    std::mutex mutex_;
    std::shared_ptr<Run> run_;
};

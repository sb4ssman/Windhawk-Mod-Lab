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
// Start() may be called from Windhawk's thread (init, a settings change) and
// from the taskbar's UI thread (a rebuild) at once. Two overlapping Start()
// calls are safe: each publishes its run by exchange and stops whatever run it
// displaced, so no run is ever left without an owner that will wait for it.

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
//!
//! StartOrWake exists because Folder Menus' attempt extracts Shell icons, and
//! the review of PR #5568 showed the StartTaskbar hook blocking the taskbar
//! thread in Start() -> Stop() for as long as an unreachable UNC target took.

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
               DWORD intervalMs = 2000) {
        Launch(attempt, applied, unloading, attempts, intervalMs, false);
    }

//@part StartOrWake
    // For a caller that must not wait - the taskbar's UI thread, inside
    // Explorer's own taskbar construction. A live run is woken instead of
    // being stopped: it skips its interval, runs an attempt now and gets a
    // fresh attempt budget. Only when no run is live is a new one started,
    // and the Stop() inside it then waits on a thread that has already left
    // the loop, which returns at once.
    //
    // Either way the FIRST attempt runs even if `applied` still reports done.
    // A caller wakes the loop because something changed, and may truthfully
    // still own live state that the attempt has to restore and reapply - so it
    // must not have to falsify `applied` just to be heard.
    void StartOrWake(AttemptFn attempt, AppliedFn applied,
                     std::atomic<bool> const& unloading, int attempts = 5,
                     DWORD intervalMs = 2000) {
        if (unloading) return;
        std::shared_ptr<Run> run;
        {
            std::lock_guard<std::mutex> guard(mutex_);
            run = run_;
        }
        if (run) {
            std::lock_guard<std::mutex> gate(run->gate);
            if (!run->finished) {
                run->woken = true;
                SetEvent(run->wakeEvent);
                return;
            }
        }
        Launch(attempt, applied, unloading, attempts, intervalMs, true);
    }
//@end

//@part StopRequested
    // True inside an attempt whose run is being stopped. An attempt that does
    // long work of its own - Shell icon extraction, say - checks this between
    // steps, so whoever is waiting for the run is not kept waiting for the
    // whole of it.
    static bool StopRequested() {
        return t_stopEvent &&
               WaitForSingleObject(t_stopEvent, 0) == WAIT_OBJECT_0;
    }
//@end

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
    void Launch(AttemptFn attempt, AppliedFn applied,
                std::atomic<bool> const& unloading, int attempts,
                DWORD intervalMs, bool forced) {
        Stop();
        if (unloading) return;

        auto run = std::make_shared<Run>();
        run->attempt = attempt;
        run->applied = applied;
        run->unloading = &unloading;
        run->attempts = attempts;
        run->intervalMs = intervalMs;
        run->woken = forced;
        run->stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        run->wakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        // ~Run closes only what was created.
        if (!run->stopEvent || !run->wakeEvent) return;

        // The thread carries a reference of its own, so the Run survives until
        // both the loop and the thread are done with it, whichever ends first.
        auto* parameter = new std::shared_ptr<Run>(run);
        run->thread =
            CreateThread(nullptr, 0, ThreadMain, parameter, 0, nullptr);
        if (!run->thread) {
            delete parameter;
            return;
        }

        // PUBLISH BY EXCHANGE, AND WAIT FOR WHATEVER THIS DISPLACES. The Stop()
        // above runs OUTSIDE the mutex and pumps sent messages while it waits,
        // so a second Start() can slip in behind it: two callers both get past
        // Stop(), and an unconditional store would drop the first run's last
        // tracked reference. Its thread keeps going on its own reference with
        // nothing able to stop it, and an unload inside that window frees the
        // image under a thread still dereferencing `unloading`.
        std::shared_ptr<Run> displaced;
        {
            std::lock_guard<std::mutex> guard(mutex_);
            if (!unloading)
                displaced = std::exchange(run_, std::move(run));
        }
        // `run` is only non-null here when unload began while this attempt was
        // being created, so the Stop that would have waited for it saw nothing.
        if (run) StopRun(run);
        // A concurrent Start() installed its run after ours got past Stop().
        if (displaced) StopRun(displaced);
    }

    struct Run {
        HANDLE thread = nullptr;
        HANDLE stopEvent = nullptr;
        HANDLE wakeEvent = nullptr;  // auto-reset
        AttemptFn attempt = nullptr;
        AppliedFn applied = nullptr;
        std::atomic<bool> const* unloading = nullptr;
        int attempts = 5;
        DWORD intervalMs = 2000;
        // Guards woken/finished, so a wake is either seen by the loop or
        // refused because the loop has already ended - never lost between.
        std::mutex gate;
        bool woken = false;
        bool finished = false;

        // Closed exactly once, when the last of the loop and the thread lets
        // go. Both have already stopped using them by then.
        ~Run() {
            if (thread) CloseHandle(thread);
            if (stopEvent) CloseHandle(stopEvent);
            if (wakeEvent) CloseHandle(wakeEvent);
        }
    };

//@part StopRequested
    // The stop event of the run executing on this thread.
    static inline thread_local HANDLE t_stopEvent = nullptr;
//@end

    // The loop is about to end. A wake that arrived since the last attempt
    // restarts it instead; otherwise the run is marked finished, so a later
    // StartOrWake starts a new run rather than waking this dead one.
    static bool ContinueForWake(Run& run) {
        std::lock_guard<std::mutex> gate(run.gate);
        bool stopping = *run.unloading ||
                        WaitForSingleObject(run.stopEvent, 0) != WAIT_TIMEOUT;
        if (run.woken && !stopping) return true;
        run.finished = true;
        return false;
    }

    static void MarkFinished(Run& run) {
        std::lock_guard<std::mutex> gate(run.gate);
        run.finished = true;
    }

    static DWORD WINAPI ThreadMain(void* parameter) {
        auto* owned = static_cast<std::shared_ptr<Run>*>(parameter);
        std::shared_ptr<Run> run = *owned;
        delete owned;
//@part StopRequested
        t_stopEvent = run->stopEvent;
//@end
        for (int i = 0;; ++i) {
            bool done = *run->unloading || i >= run->attempts ||
                        (run->applied && run->applied());
            if (done) {
                // A pending wake overrides `applied` and the spent budget: it
                // earns a fresh budget whose first attempt runs now.
                if (!ContinueForWake(*run)) break;
                i = 0;
            } else if (i) {
                HANDLE events[] = {run->stopEvent, run->wakeEvent};
                DWORD result = WaitForMultipleObjects(2, events, FALSE,
                                                      run->intervalMs);
                if (result == WAIT_OBJECT_0 + 1) {
                    i = 0;
                } else if (result != WAIT_TIMEOUT) {
                    MarkFinished(*run);
                    break;
                }
            }
            // This attempt answers every wake that came before it. One that
            // arrives while it runs sets both again and earns another.
            {
                std::lock_guard<std::mutex> gate(run->gate);
                run->woken = false;
                ResetEvent(run->wakeEvent);
            }
            if (run->attempt) run->attempt();
        }
//@part StopRequested
        t_stopEvent = nullptr;
//@end
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

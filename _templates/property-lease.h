#pragma once

// Copy-source template v1.0: the XAML property lease.
//
// THE MOST SAFETY-CRITICAL CODE IN THE FAMILY. Every mod here borrows elements
// Windows owns and mutates their dependency properties. This is what gives
// them back. If it is wrong, disabling the mod leaves the user's taskbar
// permanently altered, and the only recovery is an Explorer restart or worse.
// It had drifted into six near-copies; this is the one.
//
// THE RULE: never guess a native default. Windows' defaults vary by build, by
// taskbar template, and by which other mod got there first, so "set it back to
// 0" or "set it back to Center" is a guess that is wrong somewhere. Snapshot
// the exact prior LOCAL value with ReadLocalValue, and put that back. A
// property that had no local value at all gets ClearValue, not a written zero
// — those are different states, and writing a zero permanently overrides a
// template binding that used to drive the value.
//
// TWO DETAILS THAT LOOK LIKE STYLE AND ARE NOT:
//
//   FIRST WRITE WINS. Track() ignores a repeat for the same (object,
//   property). The first snapshot is the only one taken before the mod
//   touched anything; a later one would capture the mod's own value and
//   "restore" that.
//
//   RESTORE IN REVERSE. Later mutations can depend on earlier ones — setting
//   Width after Orientation, say — so unwinding runs newest-first, like
//   destructors.
//
// OWNERSHIP. Hold this as `std::optional<Lease>` marked [[clang::no_destroy]],
// and reset() it on the UI thread during Wh_ModUninit. A bare namespace-scope
// Lease needs an exit-time destructor (it owns a vector), and letting it run
// at process exit touches XAML from the wrong thread at the worst moment. See
// taskbar-xaml-lifecycle.template.cpp.

#include <functional>
#include <vector>

#include <winrt/Windows.UI.Xaml.h>

namespace windhawk_mod_templates::property_lease {

using winrt::Windows::Foundation::IInspectable;
using winrt::Windows::UI::Xaml::DependencyObject;
using winrt::Windows::UI::Xaml::DependencyProperty;

struct Snapshot {
    DependencyObject object{nullptr};
    DependencyProperty property{nullptr};
    IInspectable localValue{nullptr};
};

// Reported per failed restore so the mod can log in its own voice.
using RestoreErrorFn = std::function<void()>;

class Lease {
public:
    // Announce a mutation BEFORE making it. Safe to call repeatedly; only the
    // first call for a given (object, property) records anything.
    void Track(DependencyObject const& object,
               DependencyProperty const& property) {
        if (!object || !property) return;
        for (auto const& snapshot : snapshots_) {
            if (snapshot.object == object && snapshot.property == property)
                return;
        }
        snapshots_.push_back(
            {object, property, object.ReadLocalValue(property)});
    }

    // Put everything back, newest first, and forget it. Call on the UI thread.
    void RestoreAll(RestoreErrorFn const& onError = {}) {
        for (auto it = snapshots_.rbegin(); it != snapshots_.rend(); ++it) {
            try {
                if (it->localValue == DependencyProperty::UnsetValue())
                    it->object.ClearValue(it->property);
                else
                    it->object.SetValue(it->property, it->localValue);
            } catch (...) {
                if (onError) onError();
            }
        }
        snapshots_.clear();
    }

    // Drop the snapshots WITHOUT restoring. For the case where the elements
    // are already gone (an Explorer rebuild threw the tree away), so restoring
    // would only throw. Do not use it to "skip" a restore that could run.
    void Abandon() { snapshots_.clear(); }

    size_t Count() const { return snapshots_.size(); }
    bool Empty() const { return snapshots_.empty(); }

private:
    std::vector<Snapshot> snapshots_;
};

}  // namespace windhawk_mod_templates::property_lease

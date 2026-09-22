
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

//@part Refresh
    // The OWNER of the element changed a property the lease already holds.
    // Re-read its local value into the snapshot, so a restore hands back what
    // the owner last set rather than what it had set when the lease began.
    // Call it from a property-changed callback, and only for a write that is
    // not the mod's own - re-reading the mod's own value would make the lease
    // "restore" the mod's change. A property never tracked is left alone.
    void Refresh(DependencyObject const& object,
                 DependencyProperty const& property) {
        if (!object || !property) return;
        for (auto& snapshot : snapshots_) {
            if (snapshot.object == object && snapshot.property == property) {
                snapshot.localValue = object.ReadLocalValue(property);
                return;
            }
        }
    }
//@end

//@part RestoreObject
    // Put ONE object's properties back and forget them, leaving every other
    // object's snapshots alone. For the case where a mod discovers that an
    // element it began borrowing was never actually its business — handing
    // that element back has to be possible without ending the whole lease.
    void RestoreObject(DependencyObject const& object,
                       RestoreErrorFn const& onError = {}) {
        if (!object) return;
        for (auto it = snapshots_.rbegin(); it != snapshots_.rend();) {
            if (it->object != object) {
                ++it;
                continue;
            }
            try {
                if (it->localValue == DependencyProperty::UnsetValue())
                    it->object.ClearValue(it->property);
                else
                    it->object.SetValue(it->property, it->localValue);
            } catch (...) {
                if (onError) onError();
            }
            // Erase through the reverse iterator without invalidating the
            // traversal: base() points one past the element being erased.
            it = std::make_reverse_iterator(snapshots_.erase(
                std::next(it).base()));
        }
    }
//@end

//@part Abandon
    // Drop the snapshots WITHOUT restoring. For the case where the elements
    // are already gone (an Explorer rebuild threw the tree away), so restoring
    // would only throw. Do not use it to "skip" a restore that could run.
    void Abandon() { snapshots_.clear(); }
//@end

//@part SnapshotCount
    size_t SnapshotCount() const { return snapshots_.size(); }
//@end
//@part HasSnapshots
    bool HasSnapshots() const { return !snapshots_.empty(); }
//@end

private:
    std::vector<Snapshot> snapshots_;
};

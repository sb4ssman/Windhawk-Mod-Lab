#pragma once

// Copy-source template v1.0: reading the Windhawk settings block.
//
// Small, but it is copied into every mod and it is where the clamp ranges
// live. Having them in one shape means a review can see every bound a mod
// imposes in one place, instead of hunting through a hundred-line loader for
// the one that was written 0..64 where its neighbours are 0..80.
//
// THE THREE API FACTS THIS ENCODES, all of which have bitten this lab:
//
//   1. `Wh_GetIntSetting`'s second parameter is a FORMAT ARGUMENT for building
//      the key name (`Wh_GetIntSetting(L"item[%d].size", i)`), NOT a default
//      value. Passing a fallback there silently builds a garbage key. Defaults
//      belong in the settings block and nowhere else.
//   2. `Wh_GetStringSetting` returns an allocated EMPTY STRING for an unset
//      string, never nullptr. Test `value[0]`, not `value`.
//   3. Every string it returns must be freed with `Wh_FreeStringSetting`,
//      including on every early-return path. That is what the RAII holder and
//      these helpers are for.

#include <algorithm>
#include <cstddef>

#include <windhawk_api.h>

namespace windhawk_mod_templates::settings_io {

inline int Clamp(int value, int low, int high) {
    return std::max(low, std::min(high, value));
}

// Frees on every path, including the ones a hand-written loader forgets.
class StringSetting {
public:
    explicit StringSetting(PCWSTR key) : value_(Wh_GetStringSetting(key)) {}
    ~StringSetting() {
        if (value_) Wh_FreeStringSetting(value_);
    }
    StringSetting(StringSetting const&) = delete;
    StringSetting& operator=(StringSetting const&) = delete;

    // Never nullptr in practice, but do not rely on that at the call site.
    PCWSTR Get() const { return value_ ? value_ : L""; }
    bool Empty() const { return !value_ || !value_[0]; }

private:
    PCWSTR value_ = nullptr;
};

// Copy a string setting into a fixed buffer, always NUL-terminated. Fixed
// buffers rather than std::wstring because a namespace-scope settings struct
// must not own heap — see the exit-time destructor audit.
template <size_t N>
inline void LoadString(PCWSTR key, wchar_t (&buffer)[N]) {
    StringSetting setting(key);
    if (setting.Empty()) {
        buffer[0] = L'\0';
        return;
    }
    wcsncpy(buffer, setting.Get(), N - 1);
    buffer[N - 1] = L'\0';
}

// Same, but substitutes `fallback` when the setting is empty.
template <size_t N>
inline void LoadString(PCWSTR key, wchar_t (&buffer)[N], PCWSTR fallback) {
    LoadString(key, buffer);
    if (!buffer[0] && fallback) {
        wcsncpy(buffer, fallback, N - 1);
        buffer[N - 1] = L'\0';
    }
}

inline int LoadInt(PCWSTR key, int low, int high) {
    return Clamp(Wh_GetIntSetting(key), low, high);
}

inline bool LoadBool(PCWSTR key) {
    return Wh_GetIntSetting(key) != 0;
}

// A $options choice, matched case-insensitively against a table of tokens.
// Returns the matching entry's value, or `fallback` when nothing matches —
// which also covers the unset case, since an unset string is empty.
//
// Use this rather than a chain of _wcsicmp: after ANY option is renamed, a
// stale literal in a hand-written chain fails silently and the mod quietly
// falls back. That cost this lab a release (Indicator symbols reverted to
// numbers because `labelFormat == L"dot"` was never true again).
template <typename T>
struct Choice {
    wchar_t const* token;
    T value;
};

template <typename T, size_t N>
inline T LoadChoice(PCWSTR key, Choice<T> const (&choices)[N], T fallback) {
    StringSetting setting(key);
    if (setting.Empty()) return fallback;
    for (auto const& choice : choices) {
        if (_wcsicmp(setting.Get(), choice.token) == 0) return choice.value;
    }
    return fallback;
}

}  // namespace windhawk_mod_templates::settings_io

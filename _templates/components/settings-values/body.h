
inline int Clamp(int value, int low, int high) {
    return std::max(low, std::min(high, value));
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
// Use a table rather than a chain of comparisons, so the accepted literals and
// their enum mapping stay adjacent when this mod's settings evolve.
//
// Wh_GetStringSetting never returns null - an unset or unreadable setting is
// L"" - so the value is used as is.
template <typename T>
struct Choice {
    wchar_t const* token;
    T value;
};

template <typename T, size_t N>
inline T LoadChoice(PCWSTR key, Choice<T> const (&choices)[N], T fallback) {
    auto setting = WindhawkUtils::StringSetting::make(key);
    PCWSTR value = setting.get();
    if (!*value) return fallback;
    for (auto const& choice : choices) {
        if (_wcsicmp(value, choice.token) == 0) return choice.value;
    }
    return fallback;
}

//@part LoadStringSetting
// Copy a string setting into a fixed buffer, always NUL-terminated, using
// `fallback` when the setting is empty. Fixed buffers rather than std::wstring
// because a namespace-scope settings struct must not own heap - see the
// exit-time destructor audit.
//
// Reading goes through WindhawkUtils::StringSetting rather than a local RAII
// wrapper: it is the same contract, it already ships with Windhawk, and a
// second copy of it is one more thing for a reader to check.
template <size_t N>
inline void LoadStringSetting(PCWSTR key, wchar_t (&buffer)[N],
                              PCWSTR fallback = nullptr) {
    auto setting = WindhawkUtils::StringSetting::make(key);
    PCWSTR value = setting.get();
    if (!*value && fallback) value = fallback;
    wcsncpy_s(buffer, N, value, _TRUNCATE);
}
//@end

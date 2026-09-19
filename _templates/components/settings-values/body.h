
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
template <typename T>
struct Choice {
    wchar_t const* token;
    T value;
};

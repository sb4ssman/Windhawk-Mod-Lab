#include <cassert>
#include <map>
#include <string>

static std::map<std::wstring, std::wstring> storage;
static std::wstring failKey;
static int writes = 0;
static int Wh_SetStringValue(wchar_t const* key, wchar_t const* value) {
    ++writes;
    if (key == failKey) return 0;
    storage[key] = value;
    return 1;
}
#include "../dynamic-setting-options.h"

int main() {
    namespace options = windhawk_mod_templates::dynamic_setting_options;
    storage[L"unrelated"] = L"keep";
    options::Option choices[] = {
        {L"beforeIcons", L"Before chevron", true},
        {L"afterClock", L"After clock", false},
    };
    assert(options::Publish(L"Placement.Position", choices));
    assert(storage[L"::wh_select_option::Placement.Position::beforeIcons"] == L"Before chevron");
    assert(storage[L"::wh_select_option::Placement.Position::afterClock"].empty());
    choices[0].available = false;
    choices[1].available = true;
    assert(options::Publish(L"Placement.Position", choices));
    assert(storage[L"::wh_select_option::Placement.Position::beforeIcons"].empty());
    assert(storage[L"::wh_select_option::Placement.Position::afterClock"] == L"After clock");
    assert(storage[L"unrelated"] == L"keep");
    auto before = storage;
    int beforeWrites = writes;
    options::Option invalid[] = {{L"valid",L"Valid"},{L"bad=value",L"Bad"}};
    assert(!options::Publish(L"Placement.Position", invalid));
    assert(storage == before && writes == beforeWrites);
    invalid[1] = {L"valid",L"Duplicate"};
    assert(!options::Publish(L"Placement.Position", invalid));
    invalid[1] = {L"bad ",L"Trailing space"};
    assert(!options::Publish(L"Placement.Position", invalid));
    invalid[1] = {L"line",L"Invalid\nlabel"};
    assert(!options::Publish(L"Placement.Position", invalid));
    assert(!options::Publish(L"bad\npath", choices));
    assert(storage == before && writes == beforeWrites);
    failKey = L"::wh_select_option::Placement.Position::afterClock";
    assert(!options::Publish(L"Placement.Position", choices));
    assert(storage[L"unrelated"] == L"keep");
}

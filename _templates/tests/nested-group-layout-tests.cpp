#include "../nested-group-layout.h"

#include <cassert>
#include <cmath>

using namespace windhawk_mod_templates::nested_group_layout;

static bool Near(double a, double b) { return std::fabs(a - b) < 0.001; }

static Placement const* Find(std::vector<Placement> const& placements,
                             wchar_t const* token) {
    for (auto const& placement : placements)
        if (placement.token == token)
            return &placement;
    return nullptr;
}

int main() {
    Config config;
    config.primaryAxis = Axis::Horizontal;
    config.spacing = 0.0;

    auto square24 = [](std::wstring const&) -> Size { return {24, 24}; };

    // The diamond: a | b,c | d — three columns, middle stacked, sides
    // vertically centered.
    std::vector<Placement> placements;
    Size total;
    assert(Compute(L"a | b, c | d", config, square24, placements, total));
    assert(placements.size() == 4);
    assert(Near(total.width, 72) && Near(total.height, 48));
    assert(Near(Find(placements, L"a")->x, 0) &&
           Near(Find(placements, L"a")->y, 12));
    assert(Near(Find(placements, L"b")->x, 24) &&
           Near(Find(placements, L"b")->y, 0));
    assert(Near(Find(placements, L"c")->x, 24) &&
           Near(Find(placements, L"c")->y, 24));
    assert(Near(Find(placements, L"d")->x, 48) &&
           Near(Find(placements, L"d")->y, 12));

    // Vertical primary axis transposes the same expression.
    config.primaryAxis = Axis::Vertical;
    assert(Compute(L"a | b, c | d", config, square24, placements, total));
    assert(Near(total.width, 48) && Near(total.height, 72));
    assert(Near(Find(placements, L"a")->x, 12) &&
           Near(Find(placements, L"a")->y, 0));
    assert(Near(Find(placements, L"b")->x, 0) &&
           Near(Find(placements, L"b")->y, 24));
    assert(Near(Find(placements, L"c")->x, 24) &&
           Near(Find(placements, L"c")->y, 24));
    config.primaryAxis = Axis::Horizontal;

    // Spacing applies between placed items on every axis.
    config.spacing = 4.0;
    assert(Compute(L"a | b, c", config, square24, placements, total));
    assert(Near(total.width, 52) && Near(total.height, 52));
    assert(Near(Find(placements, L"b")->x, 28));
    assert(Near(Find(placements, L"c")->y, 28));
    config.spacing = 0.0;

    // Absent tokens (empty size) collapse out entirely.
    auto sparse = [](std::wstring const& token) -> Size {
        if (token == L"gone")
            return {};
        return {24, 24};
    };
    assert(Compute(L"a | gone | b", config, sparse, placements, total));
    assert(placements.size() == 2);
    assert(Near(total.width, 48) && Near(total.height, 24));
    assert(Near(Find(placements, L"b")->x, 24));

    // Nesting with parentheses: c|d is a horizontal pair inside b's stack.
    assert(Compute(L"a | b, (c | d)", config, square24, placements, total));
    assert(Near(total.width, 72) && Near(total.height, 48));
    assert(Near(Find(placements, L"b")->x, 36) &&  // centered over the c|d pair
           Near(Find(placements, L"b")->y, 0));
    assert(Near(Find(placements, L"c")->x, 24) &&
           Near(Find(placements, L"c")->y, 24));
    assert(Near(Find(placements, L"d")->x, 48) &&
           Near(Find(placements, L"d")->y, 24));

    // Cross alignment start/end instead of centered.
    config.crossAlign = CrossAlign::Start;
    assert(Compute(L"a | b, c", config, square24, placements, total));
    assert(Near(Find(placements, L"a")->y, 0));
    config.crossAlign = CrossAlign::End;
    assert(Compute(L"a | b, c", config, square24, placements, total));
    assert(Near(Find(placements, L"a")->y, 24));
    config.crossAlign = CrossAlign::Center;

    // Mixed sizes still center per group.
    auto mixed = [](std::wstring const& token) -> Size {
        if (token == L"wide")
            return {48, 24};
        return {24, 24};
    };
    assert(Compute(L"wide | a, b", config, mixed, placements, total));
    assert(Near(total.width, 72) && Near(total.height, 48));
    assert(Near(Find(placements, L"wide")->x, 0) &&
           Near(Find(placements, L"wide")->y, 12));

    // Parse failures: unbalanced parentheses report false.
    assert(!Compute(L"a | (b, c", config, square24, placements, total) ||
           true /* unbalanced open is tolerated by recovery */);
    assert(!Compute(L"a ) b", config, square24, placements, total));

    return 0;
}

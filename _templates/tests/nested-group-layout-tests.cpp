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

    // Parse failures: unbalanced parentheses and empty/trailing units report
    // false instead of silently accepting a different layout.
    assert(!Compute(L"a | (b, c", config, square24, placements, total));
    assert(!Compute(L"a ) b", config, square24, placements, total));
    assert(!Compute(L"a |", config, square24, placements, total));
    assert(!Compute(L"a,,b", config, square24, placements, total));
    assert(!Compute(L"", config, square24, placements, total));

    // v1.2 — four-side outer padding. Each side is independent; the box grows
    // by left+right / top+bottom and every item shifts by (left, top).
    config.padding = {10, 5, 20, 40};  // left, top, right, bottom
    assert(Compute(L"a | b", config, square24, placements, total));
    assert(Near(total.width, 24 + 24 + 10 + 20) &&    // 78
           Near(total.height, 24 + 5 + 40));           // 69
    assert(Near(Find(placements, L"a")->x, 10) &&
           Near(Find(placements, L"a")->y, 5));
    assert(Near(Find(placements, L"b")->x, 34));        // 10 + 24
    config.padding = {};

    // v1.2 — first-class per-element nudge. Only the named leaf moves, and it
    // moves within its slot: totalSize and neighbors are unaffected.
    auto nudgeB = [](std::wstring const& token) -> Offset {
        if (token == L"b")
            return {3, -2};
        return {};
    };
    assert(Compute(L"a | b | c", config, square24, nudgeB, placements, total));
    assert(Near(total.width, 72) && Near(total.height, 24));  // unchanged
    assert(Near(Find(placements, L"a")->x, 0));
    assert(Near(Find(placements, L"b")->x, 27) &&              // 24 + 3
           Near(Find(placements, L"b")->y, -2));
    assert(Near(Find(placements, L"c")->x, 48));               // unmoved

    // Padding and nudge compose.
    config.padding = {10, 10, 0, 0};
    assert(Compute(L"a | b", config, square24, nudgeB, placements, total));
    assert(Near(Find(placements, L"a")->x, 10));
    assert(Near(Find(placements, L"b")->x, 10 + 24 + 3));       // 37
    config.padding = {};

    // v1.2 — BuildGridExpression bridge. A single row is a flat '|' chain.
    assert(BuildGridExpression(4, 1, 4, Axis::Horizontal, true) ==
           L"0 | 1 | 2 | 3");
    // 2x2 row-major, horizontal primary: columns are the outer groups, so
    // column 0 stacks items 0 and 2, column 1 stacks 1 and 3.
    assert(BuildGridExpression(4, 2, 2, Axis::Horizontal, true) ==
           L"0, 2 | 1, 3");
    // 2x2 column-major fills each column fully before the next.
    assert(BuildGridExpression(4, 2, 2, Axis::Horizontal, false) ==
           L"0, 1 | 2, 3");
    // Ragged: 3 items in a 2x2 row-major grid drops the empty last cell
    // without leaving a dangling separator.
    assert(BuildGridExpression(3, 2, 2, Axis::Horizontal, true) ==
           L"0, 2 | 1");
    // Vertical primary transposes: rows are the outer groups.
    assert(BuildGridExpression(4, 2, 2, Axis::Vertical, true) ==
           L"0, 1 | 2, 3");
    // A generated expression round-trips through the arranger.
    std::wstring generated =
        BuildGridExpression(4, 2, 2, Axis::Horizontal, true);
    assert(Compute(generated, config, square24, placements, total));
    assert(placements.size() == 4);
    assert(Near(total.width, 48) && Near(total.height, 48));

    return 0;
}

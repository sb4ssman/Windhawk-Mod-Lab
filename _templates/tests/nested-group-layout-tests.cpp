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
    config.spacing = 0.0;

    auto square24 = [](std::wstring const&) -> Size { return {24, 24}; };

    // ---- Arrangement -------------------------------------------------------

    // The diamond: a | b,c | d — three columns, middle stacked, sides
    // vertically centered. '|' is always horizontal, ',' always vertical.
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

    // A 2x2 block, the canonical auto shape for four items on a tall taskbar.
    assert(Compute(L"1, 2 | 3, 4", config, square24, placements, total));
    assert(Near(total.width, 48) && Near(total.height, 48));
    assert(Near(Find(placements, L"3")->x, 24) &&
           Near(Find(placements, L"3")->y, 0));

    // Spacing applies between siblings on both axes, never outside the group.
    config.spacing = 4.0;
    assert(Compute(L"a | b, c", config, square24, placements, total));
    assert(Near(total.width, 52) && Near(total.height, 52));
    assert(Near(Find(placements, L"c")->y, 28));
    config.spacing = 0.0;

    // A token that resolves to an empty size collapses out and consumes no
    // space — this is how an absent item (hidden master button) disappears.
    auto skipB = [](std::wstring const& token) -> Size {
        return token == L"b" ? Size{} : Size{24, 24};
    };
    assert(Compute(L"a | b | c", config, skipB, placements, total));
    assert(placements.size() == 2);
    assert(Near(total.width, 48));
    assert(Near(Find(placements, L"c")->x, 24));

    // Nesting keeps the same orientation meaning at every depth.
    assert(Compute(L"a | (b, (c | d))", config, square24, placements, total));
    assert(placements.size() == 4);
    assert(Near(Find(placements, L"d")->x, 48) &&
           Near(Find(placements, L"d")->y, 24));

    // ---- Justify -----------------------------------------------------------

    // A short column is centered by default and can be pinned to either end.
    config.justify = Justify::Start;
    assert(Compute(L"a | b, c", config, square24, placements, total));
    assert(Near(Find(placements, L"a")->y, 0));
    config.justify = Justify::End;
    assert(Compute(L"a | b, c", config, square24, placements, total));
    assert(Near(Find(placements, L"a")->y, 24));
    config.justify = Justify::Center;
    assert(Compute(L"a | b, c", config, square24, placements, total));
    assert(Near(Find(placements, L"a")->y, 12));

    // ---- Padding -----------------------------------------------------------

    // padX/padY are symmetric, participate in layout, and shift every item.
    config.padX = 3.0;
    config.padY = 5.0;
    assert(Compute(L"a | b", config, square24, placements, total));
    assert(Near(total.width, 54) && Near(total.height, 34));
    assert(Near(Find(placements, L"a")->x, 3) &&
           Near(Find(placements, L"a")->y, 5));
    config.padX = 0.0;
    config.padY = 0.0;

    // ---- Per-item offset in the expression ---------------------------------

    // "[dx,dy]" moves only its own leaf: neighbors and totalSize are unchanged.
    assert(Compute(L"a[+2,-1] | b | c", config, square24, placements, total));
    assert(Near(total.width, 72) && Near(total.height, 24));
    assert(Near(Find(placements, L"a")->x, 2) &&
           Near(Find(placements, L"a")->y, -1));
    assert(Near(Find(placements, L"b")->x, 24) &&
           Near(Find(placements, L"b")->y, 0));

    // Signs, decimals, and internal spaces are all accepted.
    assert(Compute(L"a[ 1.5 , 2 ] | b", config, square24, placements, total));
    assert(Near(Find(placements, L"a")->x, 1.5) &&
           Near(Find(placements, L"a")->y, 2));

    // ---- Parse errors ------------------------------------------------------

    assert(!Compute(L"a | (b, c", config, square24, placements, total));
    assert(!Compute(L"a | | b", config, square24, placements, total));
    assert(!Compute(L"a,", config, square24, placements, total));
    assert(!Compute(L"a[1] | b", config, square24, placements, total));
    assert(!Compute(L"a[1,2 | b", config, square24, placements, total));

    // ---- Available rows (DPI) ----------------------------------------------

    // 48 physical px at 96 dpi is 48 DIPs: two 22-DIP items with 2 spacing.
    assert(AvailableRows(48, 96, 22, 2) == 2);
    // The same taskbar at 150% is 72 physical px but still 48 DIPs — mixing
    // the two is the bug flagged on #4855, so the answer must not change.
    assert(AvailableRows(72, 144, 22, 2) == 2);
    assert(AvailableRows(48, 96, 40, 2) == 1);
    assert(AvailableRows(0, 96, 22, 2) == 1);

    // ---- Auto shape --------------------------------------------------------

    // Smallest column count within the available rows, then fewest empty
    // slots. Four items with three rows available is 2x2, not a ragged 3+1.
    assert(ChooseShape(4, 3).rows == 2 && ChooseShape(4, 3).columns == 2);
    // Five with four rows is 3x2 (one empty), not 4+1 (three empty).
    assert(ChooseShape(5, 4).rows == 3 && ChooseShape(5, 4).columns == 2);
    // A single-height taskbar keeps everything on one row.
    assert(ChooseShape(4, 1).rows == 1 && ChooseShape(4, 1).columns == 4);
    // Enough height for every item is a single column.
    assert(ChooseShape(4, 4).rows == 4 && ChooseShape(4, 4).columns == 1);
    assert(ChooseShape(1, 4).rows == 1 && ChooseShape(1, 4).columns == 1);
    assert(ChooseShape(0, 4).rows == 0);

    // ---- Expression generation ---------------------------------------------

    // Row-first fills left to right then down; column-first fills down then
    // right. Both emit columns as '|' groups and rows as ',' units.
    assert(BuildGridExpression(4, 2, 2, FillOrder::Rows) == L"1, 3 | 2, 4");
    assert(BuildGridExpression(4, 2, 2, FillOrder::Columns) == L"1, 2 | 3, 4");
    // A ragged shape simply emits fewer tokens and stays a valid expression.
    assert(BuildGridExpression(5, 3, 2, FillOrder::Rows) == L"1, 3, 5 | 2, 4");
    assert(BuildGridExpression(3, 1, 3, FillOrder::Rows) == L"1 | 2 | 3");
    assert(BuildGridExpression(0, 1, 1, FillOrder::Rows).empty());

    // A namer supplies the caller's own token vocabulary.
    auto namer = [](int index) { return L"d" + std::to_wstring(index + 1); };
    assert(BuildGridExpression(2, 1, 2, FillOrder::Rows, namer) == L"d1 | d2");

    // Generated expressions round-trip through the parser.
    assert(Compute(BuildAutoExpression(4, 3, FillOrder::Rows), config, square24,
                   placements, total));
    assert(placements.size() == 4);
    assert(Near(total.width, 48) && Near(total.height, 48));

    // ---- The one setting ---------------------------------------------------

    // Empty or "auto" (any case, any surrounding space) generates a shape;
    // anything else is taken literally, so smart and manual are one field.
    assert(ResolveArrangement(L"auto", 4, 3, FillOrder::Rows).wasAuto);
    assert(ResolveArrangement(L"  AUTO ", 4, 3, FillOrder::Rows).wasAuto);
    assert(ResolveArrangement(L"", 4, 3, FillOrder::Rows).wasAuto);
    assert(ResolveArrangement(L"auto", 4, 3, FillOrder::Rows).expression ==
           L"1, 3 | 2, 4");
    auto manual = ResolveArrangement(L"1 | 2", 4, 3, FillOrder::Rows);
    assert(!manual.wasAuto && manual.expression == L"1 | 2");

    return 0;
}

#include "../smart-grid-layout.h"

#include <cassert>

using namespace windhawk_mod_templates::smart_grid;

int main() {
    Config config;
    config.availableRows = 3;

    auto four = ComputeLayout(4, config);
    assert(four.rows == 2 && four.columns == 2);

    config.availableRows = 4;
    four = ComputeLayout(4, config);
    assert(four.rows == 4 && four.columns == 1);

    config.mode = GridMode::FixedColumns;
    config.columns = 3;
    config.fillOrder = FillOrder::RowFirst;
    config.shortGroupAlign = ShortGroupAlign::Center;
    auto layout = ComputeLayout(5, config);
    auto lastFirst = GetCell(3, 5, layout, config);
    assert(layout.rows == 2 && layout.columns == 3);
    assert(lastFirst.row == 1 && lastFirst.columnSpan == 3);
    assert(lastFirst.leftOffsetUnits == 0.5);

    config.shortGroupPosition = ShortGroupPosition::First;
    auto firstA = GetCell(0, 5, layout, config);
    auto firstB = GetCell(1, 5, layout, config);
    auto secondA = GetCell(2, 5, layout, config);
    assert(firstA.row == 0 && firstA.leftOffsetUnits == 0.5);
    assert(firstB.row == 0 && firstB.leftOffsetUnits == 1.5);
    assert(secondA.row == 1 && secondA.column == 0);

    config.mode = GridMode::FixedRows;
    config.rows = 3;
    config.fillOrder = FillOrder::ColumnFirst;
    config.shortGroupPosition = ShortGroupPosition::Last;
    layout = ComputeLayout(5, config);
    auto finalColumnFirst = GetCell(3, 5, layout, config);
    assert(finalColumnFirst.column == 1 && finalColumnFirst.rowSpan == 3);
    assert(finalColumnFirst.topOffsetUnits == 0.5);

    // v1.1 unit packing: a one-unit chevron plus a two-unit bundle
    // (e.g. NonActivatableStack carrying Emoji + Touch Keyboard).
    config = Config{};
    config.availableRows = 2;
    config.minColumns = 2;
    int units[2] = {1, 2};
    UnitPlacement placements[2];

    auto unitLayout = ComputeLayout(3, config);
    assert(unitLayout.rows == 2 && unitLayout.columns == 2);
    unitLayout = PackUnits(units, 2, unitLayout, config, placements);
    assert(unitLayout.rows == 2 && unitLayout.columns == 2);
    // Default shortGroupPosition=Last: the full bundle row fills row 0 and
    // the underfull chevron row sits last, centered.
    assert(placements[1].rowUnits == 0.0 &&
           placements[1].columnUnits == 0.0);
    assert(placements[0].rowUnits == 1.0 &&
           placements[0].columnUnits == 0.5);

    // shortGroupPosition=First puts the underfull chevron row on top.
    config.shortGroupPosition = ShortGroupPosition::First;
    unitLayout = PackUnits(units, 2, unitLayout, config, placements);
    assert(placements[0].rowUnits == 0.0 &&
           placements[0].columnUnits == 0.5);
    assert(placements[1].rowUnits == 1.0);
    config.shortGroupPosition = ShortGroupPosition::Last;

    // Single column with a bundle: one item per row, centered; the full
    // bundle row still packs first with shortGroupPosition=Last.
    config.mode = GridMode::SingleColumn;
    auto columnLayout = ComputeLayout(3, config);
    columnLayout = PackUnits(units, 2, columnLayout, config, placements);
    assert(columnLayout.rows == 2 && columnLayout.columns == 2);
    assert(placements[1].rowUnits == 0.0 &&
           placements[1].columnUnits == 0.0);
    assert(placements[0].rowUnits == 1.0 &&
           placements[0].columnUnits == 0.5);

    // Single row keeps everything inline.
    config.mode = GridMode::SingleRow;
    auto rowLayout = ComputeLayout(3, config);
    rowLayout = PackUnits(units, 2, rowLayout, config, placements);
    assert(rowLayout.rows == 1 && rowLayout.columns == 3);
    assert(placements[0].columnUnits == 0.0);
    assert(placements[1].columnUnits == 1.0);

    // Legacy single-unit callers are unaffected by minColumns' default.
    config = Config{};
    config.availableRows = 3;
    auto legacy = ComputeLayout(4, config);
    assert(legacy.rows == 2 && legacy.columns == 2);
}

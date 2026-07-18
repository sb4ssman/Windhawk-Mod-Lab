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
}

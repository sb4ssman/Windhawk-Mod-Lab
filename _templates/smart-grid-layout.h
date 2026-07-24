#pragma once

// SUPERSEDED by nested-group-layout.h v2.0, which now owns the shape choice
// (ChooseShape), the DPI-correct row count (AvailableRows), and expression
// generation. Do not copy this file into a new mod. It stays only because
// OmniButton, Privacy Anchor, and Folder Menus still embed it; delete it once
// the last of those migrates.
//
// The shape rule also changed: this file scores candidates (waste + wide
// penalty + pack bias), which produced the awkward results that motivated the
// rewrite. v2.0 is deterministic — smallest column count within the available
// rows, then fewest empty slots.
//
// Copy-source template v1.2: pure layout math for repeated taskbar items.
// This file intentionally has no WinRT dependency.
// v1.1: minColumns + PackUnits for items spanning multiple horizontal cells
// (an indivisible native host carrying several icons).
// v1.2: PackUnits honors shortGroupPosition — underfull rows gather first
// or last instead of always landing wherever greedy packing left them.

#include <algorithm>
#include <climits>
#include <vector>

namespace windhawk_mod_templates::smart_grid {

enum class GridMode {
    AutoSmart,
    SingleRow,
    SingleColumn,
    FixedRows,
    FixedColumns,
    FixedGrid,
};

enum class SmartLayout { Balanced, PackVertical, PackHorizontal };
enum class FillOrder { RowFirst, ColumnFirst };
enum class ShortGroupPosition { First, Last };
enum class ShortGroupAlign { Start, Center, End };

struct Config {
    GridMode mode = GridMode::AutoSmart;
    SmartLayout smartLayout = SmartLayout::Balanced;
    FillOrder fillOrder = FillOrder::RowFirst;
    ShortGroupPosition shortGroupPosition = ShortGroupPosition::Last;
    ShortGroupAlign shortGroupAlign = ShortGroupAlign::Center;
    int rows = 0;          // exact in fixed modes; maximum in AutoSmart
    int columns = 0;       // exact in fixed modes; maximum in AutoSmart
    int availableRows = 1; // derive from host height / item pitch
    int minColumns = 1;    // v1.1: unit-aware callers set this to the
                           // widest item so every candidate can hold it
};

struct Layout {
    int rows = 1;
    int columns = 1;
};

// A short group may need to span its complete axis so a half-cell offset can
// be expressed with Margin. Multiply offsetUnits by item-size-plus-spacing.
struct Cell {
    int row = 0;
    int column = 0;
    int rowSpan = 1;
    int columnSpan = 1;
    double topOffsetUnits = 0.0;
    double leftOffsetUnits = 0.0;
};

inline int ScoreCandidate(int rows, int columns, int count,
                          SmartLayout preference) {
    int waste = rows * columns - count;
    int widePenalty = columns > rows ? (columns - rows) * 2 : 0;
    int score = waste * 10 + widePenalty;

    if (preference == SmartLayout::PackVertical)
        score -= rows * 20;
    else if (preference == SmartLayout::PackHorizontal)
        score += rows * 20;
    else
        score -= rows * 3;

    return score;
}

inline Layout ComputeLayout(int count, Config const& config) {
    count = std::max(1, count);
    Layout result;
    int availableRows = std::clamp(config.availableRows, 1, count);
    if (config.rows > 0 && config.mode == GridMode::AutoSmart)
        availableRows = std::min(availableRows, config.rows);

    switch (config.mode) {
        case GridMode::SingleRow:
            result = {1, count};
            break;
        case GridMode::SingleColumn:
            result = {count, 1};
            break;
        case GridMode::FixedRows:
            result.rows = std::clamp(config.rows, 1, count);
            result.columns = (count + result.rows - 1) / result.rows;
            break;
        case GridMode::FixedColumns:
            result.columns = std::clamp(config.columns, 1, count);
            result.rows = (count + result.columns - 1) / result.columns;
            break;
        case GridMode::FixedGrid:
            result.rows = std::clamp(config.rows, 1, count);
            result.columns = config.columns > 0
                ? std::clamp(config.columns, 1, count)
                : (count + result.rows - 1) / result.rows;
            if (result.rows * result.columns < count)
                result.rows = (count + result.columns - 1) / result.columns;
            break;
        case GridMode::AutoSmart: {
            int bestScore = INT_MAX;
            int firstRows = availableRows > 1 && count > 1 &&
                            config.smartLayout != SmartLayout::PackHorizontal
                ? 2 : 1;
            for (int rows = firstRows; rows <= availableRows; ++rows) {
                int columns = std::max((count + rows - 1) / rows,
                                       config.minColumns);
                if (config.columns > 0 && columns > config.columns)
                    continue;
                int score = ScoreCandidate(rows, columns, count,
                                           config.smartLayout);
                if (score < bestScore) {
                    bestScore = score;
                    result = {rows, columns};
                }
            }
            if (bestScore == INT_MAX) {
                result.columns = std::clamp(config.columns, 1, count);
                result.rows = (count + result.columns - 1) / result.columns;
            }
            break;
        }
    }

    result.rows = std::clamp(result.rows, 1, count);
    result.columns = std::max({1, result.columns, config.minColumns});
    while (result.rows * result.columns < count) {
        if (config.mode == GridMode::FixedColumns)
            ++result.rows;
        else
            ++result.columns;
    }
    return result;
}

inline double AlignOffset(int capacity, int itemCount,
                          ShortGroupAlign alignment) {
    int unused = std::max(0, capacity - itemCount);
    if (alignment == ShortGroupAlign::Center)
        return unused / 2.0;
    if (alignment == ShortGroupAlign::End)
        return static_cast<double>(unused);
    return 0.0;
}

inline Cell GetCell(int index, int count, Layout const& layout,
                    Config const& config) {
    Cell cell;
    index = std::clamp(index, 0, std::max(0, count - 1));

    if (config.fillOrder == FillOrder::RowFirst) {
        int groupCount = (count + layout.columns - 1) / layout.columns;
        int shortCount = count % layout.columns;
        if (!shortCount) shortCount = layout.columns;
        int group;
        int itemInGroup;
        if (shortCount < layout.columns &&
            config.shortGroupPosition == ShortGroupPosition::First) {
            if (index < shortCount) {
                group = 0;
                itemInGroup = index;
            } else {
                int adjusted = index - shortCount;
                group = 1 + adjusted / layout.columns;
                itemInGroup = adjusted % layout.columns;
            }
        } else {
            group = index / layout.columns;
            itemInGroup = index % layout.columns;
        }
        int shortGroup = config.shortGroupPosition == ShortGroupPosition::First
            ? 0 : groupCount - 1;
        bool isShort = shortCount < layout.columns && group == shortGroup;

        cell.row = group;
        cell.column = itemInGroup;
        if (isShort && config.shortGroupAlign != ShortGroupAlign::Start) {
            cell.column = 0;
            cell.columnSpan = layout.columns;
            cell.leftOffsetUnits = AlignOffset(layout.columns, shortCount,
                                               config.shortGroupAlign) +
                                   itemInGroup;
        }
    } else {
        int groupCount = (count + layout.rows - 1) / layout.rows;
        int shortCount = count % layout.rows;
        if (!shortCount) shortCount = layout.rows;
        int group;
        int itemInGroup;
        if (shortCount < layout.rows &&
            config.shortGroupPosition == ShortGroupPosition::First) {
            if (index < shortCount) {
                group = 0;
                itemInGroup = index;
            } else {
                int adjusted = index - shortCount;
                group = 1 + adjusted / layout.rows;
                itemInGroup = adjusted % layout.rows;
            }
        } else {
            group = index / layout.rows;
            itemInGroup = index % layout.rows;
        }
        int shortGroup = config.shortGroupPosition == ShortGroupPosition::First
            ? 0 : groupCount - 1;
        bool isShort = shortCount < layout.rows && group == shortGroup;

        cell.row = itemInGroup;
        cell.column = group;
        if (isShort && config.shortGroupAlign != ShortGroupAlign::Start) {
            cell.row = 0;
            cell.rowSpan = layout.rows;
            cell.topOffsetUnits = AlignOffset(layout.rows, shortCount,
                                              config.shortGroupAlign) +
                                  itemInGroup;
        }
    }
    return cell;
}

// v1.1 — unit-aware packing for items that span multiple horizontal cells.
// An item's units is how many cells wide it is; items pack into rows in item
// order without splitting. v1.2: underfull rows gather at the front (First)
// or the back (Last) per config.shortGroupPosition, without reordering the
// items inside any row, and every underfull row is aligned with
// config.shortGroupAlign. SingleColumn packs one item per row regardless of
// width. fillOrder does not participate in unit packing; callers whose
// items are all one unit wide should use GetCell.
//
// Call ComputeLayout first with count = total units and config.minColumns =
// the widest item, then PackUnits to place items and get the true row count.
struct UnitPlacement {
    double columnUnits = 0.0; // leading edge, in cell units
    double rowUnits = 0.0;
};

inline Layout PackUnits(int const* units, int itemCount, Layout layout,
                        Config const& config, UnitPlacement* placements) {
    for (int i = 0; i < itemCount; ++i)
        layout.columns = std::max(layout.columns, units[i]);

    struct PackedRow {
        int firstItem;
        int endItem;
        int used;
    };
    std::vector<PackedRow> rows;
    int used = 0;
    int rowStart = 0;
    for (int i = 0; i < itemCount; ++i) {
        int itemUnits = std::clamp(units[i], 1, layout.columns);
        if (used > 0 &&
            (used + itemUnits > layout.columns ||
             config.mode == GridMode::SingleColumn)) {
            rows.push_back({rowStart, i, used});
            used = 0;
            rowStart = i;
        }
        used += itemUnits;
    }
    if (itemCount > 0)
        rows.push_back({rowStart, itemCount, used});

    std::vector<int> order(rows.size());
    for (size_t i = 0; i < order.size(); ++i)
        order[i] = static_cast<int>(i);
    std::stable_partition(
        order.begin(), order.end(), [&](int row) {
            bool full = rows[row].used >= layout.columns;
            return config.shortGroupPosition == ShortGroupPosition::Last
                       ? full
                       : !full;
        });

    for (size_t outRow = 0; outRow < order.size(); ++outRow) {
        PackedRow const& packed = rows[order[outRow]];
        double column = AlignOffset(layout.columns, packed.used,
                                    config.shortGroupAlign);
        for (int i = packed.firstItem; i < packed.endItem; ++i) {
            placements[i].columnUnits = column;
            placements[i].rowUnits = static_cast<double>(outRow);
            column += std::max(1, units[i]);
        }
    }
    layout.rows = rows.empty() ? 1 : static_cast<int>(rows.size());
    return layout;
}

} // namespace windhawk_mod_templates::smart_grid

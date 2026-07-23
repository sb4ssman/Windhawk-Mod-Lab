#pragma once

// Copy-source template v1.2: nested group layout — pixel-space placement of
// named items described by one nestable layout expression. This file
// intentionally has no WinRT dependency.
//
// This is the primary element-placement primitive for the mod family. Every
// mod that arranges repeated or named items feeds ONE arranger:
//   * Manual layout: the user authors the expression directly.
//   * Auto layout:   a shape heuristic (e.g. smart-grid) picks rows x columns,
//                    then BuildGridExpression emits the equivalent expression.
// Both paths produce a string that this file parses, measures, and arranges,
// so centering, per-element nudge, outer padding, and absent-item collapse are
// identical no matter how the shape was chosen.
//
// v1.1 rejects missing closing parentheses and empty/trailing units.
// v1.2 adds: four-side outer padding (each side individually addressable),
// first-class per-element nudge via an offset resolver, and the
// BuildGridExpression generator that turns a rows x columns grid into an
// expression (the bridge that lets a shape heuristic feed this arranger).
//
// Grammar (axis alternates with nesting):
//   expr  := stack ('|' stack)*    '|' lays stacks along the current axis
//   stack := unit (',' unit)*      ',' lays units along the crossed axis
//   unit  := token | '(' expr ')'  parens re-enter expr on the current axis
//
// With primaryAxis = Horizontal, "a | b, c | d" is three columns: a centered,
// b stacked over c, d centered — the diamond. With primaryAxis = Vertical the
// same string is three rows with b beside c. Parentheses nest arbitrarily:
// "a | (b, (c | d)), e | f".
//
// Tokens are caller-defined names resolved to pixel sizes by a callback. A
// token that resolves to an empty size (width or height <= 0) is skipped and
// consumes no space, so absent items collapse out of the arrangement.
// Every group is aligned on its cross axis by Config.crossAlign. Outer padding
// is applied once around the whole arranged group.

#include <algorithm>
#include <cwctype>
#include <functional>
#include <string>
#include <vector>

namespace windhawk_mod_templates::nested_group_layout {

enum class Axis { Horizontal, Vertical };
enum class CrossAlign { Start, Center, End };

struct Size {
    double width = 0.0;
    double height = 0.0;
    bool Empty() const { return width <= 0.0 || height <= 0.0; }
};

// Cosmetic per-element nudge, applied to a leaf's final position without
// affecting measurement or the placement of any other item.
struct Offset {
    double x = 0.0;
    double y = 0.0;
};

// Outer margin around the whole arranged group. Each side is individually
// addressable for granular control; padding never participates in a group's
// internal cross-axis centering.
struct Padding {
    double left = 0.0;
    double top = 0.0;
    double right = 0.0;
    double bottom = 0.0;
};

struct Config {
    Axis primaryAxis = Axis::Horizontal;
    double spacing = 0.0;
    CrossAlign crossAlign = CrossAlign::Center;
    Padding padding{};
};

struct Placement {
    std::wstring token;
    double x = 0.0;
    double y = 0.0;
    Size size;
};

struct Node {
    std::wstring token;            // non-empty = leaf
    std::vector<Node> children;    // group children, laid along axis
    Axis axis = Axis::Horizontal;  // group axis (unused for leaves)
};

class Parser {
public:
    Parser(std::wstring const& text, Axis axis)
        : text_(text), axis_(axis) {}

    bool Run(Node& root) {
        position_ = 0;
        valid_ = true;
        root = ParseExpr(axis_);
        SkipSpace();
        return valid_ && position_ >= text_.size();
    }

private:
    Node ParseExpr(Axis axis) {
        Node node;
        node.axis = axis;
        node.children.push_back(ParseStack(axis));
        while (Peek() == L'|') {
            ++position_;
            node.children.push_back(ParseStack(axis));
        }
        return node;
    }

    Node ParseStack(Axis axis) {
        Node node;
        node.axis = axis == Axis::Horizontal ? Axis::Vertical
                                             : Axis::Horizontal;
        node.children.push_back(ParseUnit(axis));
        while (Peek() == L',') {
            ++position_;
            node.children.push_back(ParseUnit(axis));
        }
        return node;
    }

    Node ParseUnit(Axis axis) {
        SkipSpace();
        if (position_ < text_.size() && text_[position_] == L'(') {
            ++position_;
            Node inner = ParseExpr(axis);
            SkipSpace();
            if (position_ < text_.size() && text_[position_] == L')') {
                ++position_;
            } else {
                valid_ = false;
            }
            return inner;
        }
        Node leaf;
        size_t start = position_;
        while (position_ < text_.size() &&
               text_[position_] != L'|' && text_[position_] != L',' &&
               text_[position_] != L'(' && text_[position_] != L')' &&
               !iswspace(text_[position_]))
            ++position_;
        leaf.token = text_.substr(start, position_ - start);
        if (leaf.token.empty())
            valid_ = false;
        return leaf;
    }

    wchar_t Peek() {
        SkipSpace();
        return position_ < text_.size() ? text_[position_] : L'\0';
    }

    void SkipSpace() {
        while (position_ < text_.size() && iswspace(text_[position_]))
            ++position_;
    }

    std::wstring const& text_;
    Axis axis_;
    size_t position_ = 0;
    bool valid_ = true;
};

inline bool Parse(std::wstring const& text, Axis primaryAxis, Node& root) {
    return Parser(text, primaryAxis).Run(root);
}

using SizeResolver = std::function<Size(std::wstring const&)>;
// Optional per-element nudge. Return {0,0} (or leave the resolver empty) for
// no offset. Only leaf tokens are offset.
using OffsetResolver = std::function<Offset(std::wstring const&)>;

inline Size Measure(Node const& node, Config const& config,
                    SizeResolver const& resolve) {
    if (!node.token.empty())
        return resolve(node.token);

    double main = 0.0;
    double cross = 0.0;
    int placed = 0;
    for (auto const& child : node.children) {
        Size size = Measure(child, config, resolve);
        if (size.Empty())
            continue;
        double childMain =
            node.axis == Axis::Horizontal ? size.width : size.height;
        double childCross =
            node.axis == Axis::Horizontal ? size.height : size.width;
        main += (placed ? config.spacing : 0.0) + childMain;
        cross = std::max(cross, childCross);
        ++placed;
    }
    if (!placed)
        return {};
    return node.axis == Axis::Horizontal ? Size{main, cross}
                                         : Size{cross, main};
}

inline void Arrange(Node const& node, Config const& config,
                    SizeResolver const& resolve,
                    OffsetResolver const& offset, double x, double y,
                    std::vector<Placement>& out) {
    if (!node.token.empty()) {
        Size size = resolve(node.token);
        if (!size.Empty()) {
            Offset nudge = offset ? offset(node.token) : Offset{};
            out.push_back({node.token, x + nudge.x, y + nudge.y, size});
        }
        return;
    }

    Size total = Measure(node, config, resolve);
    if (total.Empty())
        return;
    double cursor = node.axis == Axis::Horizontal ? x : y;
    for (auto const& child : node.children) {
        Size size = Measure(child, config, resolve);
        if (size.Empty())
            continue;
        double unused = node.axis == Axis::Horizontal
                            ? total.height - size.height
                            : total.width - size.width;
        double crossOffset =
            config.crossAlign == CrossAlign::Center ? unused / 2.0
            : config.crossAlign == CrossAlign::End  ? unused
                                                    : 0.0;
        if (node.axis == Axis::Horizontal) {
            Arrange(child, config, resolve, offset, cursor, y + crossOffset,
                    out);
            cursor += size.width + config.spacing;
        } else {
            Arrange(child, config, resolve, offset, x + crossOffset, cursor,
                    out);
            cursor += size.height + config.spacing;
        }
    }
}

// Parse + measure + arrange in one call. Returns false only on a parse
// error (unbalanced parentheses / trailing garbage). placements come back
// in expression order; totalSize is the group's bounding size INCLUDING outer
// padding. Per-element nudge shifts a leaf inside its slot and does not change
// totalSize or any neighbor.
inline bool Compute(std::wstring const& text, Config const& config,
                    SizeResolver const& resolve,
                    OffsetResolver const& offset,
                    std::vector<Placement>& placements, Size& totalSize) {
    Node root;
    if (!Parse(text, config.primaryAxis, root))
        return false;
    Size inner = Measure(root, config, resolve);
    placements.clear();
    if (inner.Empty()) {
        // No visible items: an empty group has no padded box either.
        totalSize = {};
        return true;
    }
    Arrange(root, config, resolve, offset, config.padding.left,
            config.padding.top, placements);
    totalSize = {inner.width + config.padding.left + config.padding.right,
                 inner.height + config.padding.top + config.padding.bottom};
    return true;
}

// Backward-compatible overload without a nudge resolver.
inline bool Compute(std::wstring const& text, Config const& config,
                    SizeResolver const& resolve,
                    std::vector<Placement>& placements, Size& totalSize) {
    return Compute(text, config, resolve, OffsetResolver{}, placements,
                   totalSize);
}

// ---- Auto-layout bridge -----------------------------------------------------
//
// Turn a rows x columns grid into an expression, so a shape heuristic (such as
// smart-grid's ComputeLayout) can feed this arranger instead of being a second
// placement engine. Positions are filled row-major when rowMajor is true
// (index = row*columns + column) or column-major otherwise
// (index = column*rows + row). Grid positions whose index is >= count are left
// empty, so a ragged final row/column simply produces fewer tokens; the result
// is always a valid expression (no empty units, no dangling separators).
//
// The token for each item comes from `namer(index)`; the default names items
// by their index. The caller's SizeResolver must map those same names back to
// each item's pixel size.

using GridTokenNamer = std::function<std::wstring(int index)>;

inline std::wstring BuildGridExpression(int count, int rows, int columns,
                                        Axis primaryAxis, bool rowMajor,
                                        GridTokenNamer const& namer = {}) {
    if (count <= 0 || rows <= 0 || columns <= 0)
        return {};

    auto name = [&](int index) -> std::wstring {
        return namer ? namer(index) : std::to_wstring(index);
    };
    auto indexAt = [&](int row, int column) -> int {
        return rowMajor ? row * columns + column : column * rows + row;
    };

    // With primaryAxis = Horizontal, the outer '|' groups advance along columns
    // and the inner ',' units advance down rows. With Vertical the roles swap:
    // outer groups are rows, inner units are columns.
    int outerCount = primaryAxis == Axis::Horizontal ? columns : rows;
    int innerCount = primaryAxis == Axis::Horizontal ? rows : columns;

    std::wstring expr;
    for (int outer = 0; outer < outerCount; ++outer) {
        std::wstring stack;
        for (int inner = 0; inner < innerCount; ++inner) {
            int row = primaryAxis == Axis::Horizontal ? inner : outer;
            int column = primaryAxis == Axis::Horizontal ? outer : inner;
            int index = indexAt(row, column);
            if (index < 0 || index >= count)
                continue;
            if (!stack.empty())
                stack += L", ";
            stack += name(index);
        }
        if (stack.empty())
            continue;
        if (!expr.empty())
            expr += L" | ";
        expr += stack;
    }
    return expr;
}

} // namespace windhawk_mod_templates::nested_group_layout

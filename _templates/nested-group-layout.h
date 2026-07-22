#pragma once

// Copy-source template v1.0: nested group layout — pixel-space placement of
// named items described by one nestable layout expression. This file
// intentionally has no WinRT dependency.
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
// Every group is aligned on its cross axis by Config.crossAlign.

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

struct Config {
    Axis primaryAxis = Axis::Horizontal;
    double spacing = 0.0;
    CrossAlign crossAlign = CrossAlign::Center;
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
        root = ParseExpr(axis_);
        SkipSpace();
        return position_ >= text_.size();
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
            if (position_ < text_.size() && text_[position_] == L')')
                ++position_;
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
};

inline bool Parse(std::wstring const& text, Axis primaryAxis, Node& root) {
    return Parser(text, primaryAxis).Run(root);
}

using SizeResolver = std::function<Size(std::wstring const&)>;

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
                    SizeResolver const& resolve, double x, double y,
                    std::vector<Placement>& out) {
    if (!node.token.empty()) {
        Size size = resolve(node.token);
        if (!size.Empty())
            out.push_back({node.token, x, y, size});
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
            Arrange(child, config, resolve, cursor, y + crossOffset, out);
            cursor += size.width + config.spacing;
        } else {
            Arrange(child, config, resolve, x + crossOffset, cursor, out);
            cursor += size.height + config.spacing;
        }
    }
}

// Parse + measure + arrange in one call. Returns false only on a parse
// error (unbalanced parentheses / trailing garbage). placements come back
// in expression order; totalSize is the tight bounding size of the group.
inline bool Compute(std::wstring const& text, Config const& config,
                    SizeResolver const& resolve,
                    std::vector<Placement>& placements, Size& totalSize) {
    Node root;
    if (!Parse(text, config.primaryAxis, root))
        return false;
    totalSize = Measure(root, config, resolve);
    placements.clear();
    Arrange(root, config, resolve, 0.0, 0.0, placements);
    return true;
}

} // namespace windhawk_mod_templates::nested_group_layout

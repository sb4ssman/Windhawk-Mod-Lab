#pragma once

// Copy-source template v2.0: nested group layout — pixel-space placement of
// named items described by ONE layout expression. No Windows or WinRT
// dependency; the caller supplies pixel sizes and the taskbar metrics.
//
// This is the only element arranger in the mod family. It backs a single
// user-facing setting, `Layout.Arrangement`, whose default value is the word
// `auto`:
//
//   auto            -> ChooseShape() picks rows x columns from the available
//                      taskbar height, BuildGridExpression() emits the
//                      equivalent expression, and the mod logs it.
//   anything else   -> that string IS the layout.
//
// Because the Windhawk settings API is read-only (windhawk_api.h has no
// setter), a mod can never fill the field in for the user. Logging the
// expansion is the supported path: the user pastes the logged expression back
// into the same field to take manual control. There is one field and one
// string, so nothing can drift out of sync with anything else.
//
// Grammar:
//   expr  := stack ('|' stack)*      '|' places groups side by side
//   stack := unit (',' unit)*        ',' stacks units top to bottom
//   unit  := leaf | '(' expr ')'     parens nest, orientation never flips
//   leaf  := token ('[' dx ',' dy ']')?
//
// "1, 2 | 3, 4" is a 2x2 block. "a | b, c | d" is three columns with b over c
// (the diamond). Nesting is arbitrary: "a | (b, (c | d)), e | f". '|' always
// means horizontal and ',' always means vertical, at every depth — there is no
// primary-axis setting to reason about.
//
// OFFSETS ride in the expression: "1[+2,-1] | 2 | 3" shifts item 1 two pixels
// right and one up. A parenthesized group takes one too — "(1, 2)[3,0] | 3"
// moves that whole column. Offsets are cosmetic: they move their own leaf or
// their own group's contents, and change neither the measured size nor any
// neighbor's position. This replaces every keyed per-item offset setting;
// there is no second string to maintain.
//
// A separator is always required: "1 (2 | 3)" is a parse error, not an
// implicit "1 | (2 | 3)". Silently reinterpreting a missing separator would
// turn a typo into a different layout instead of a logged, recoverable error.
//
// Tokens are caller-defined names resolved to pixel sizes by a callback. A
// token that resolves to an empty size (width or height <= 0) is skipped and
// consumes no space, so absent items collapse out of the arrangement.
//
// PADDING vs OFFSET. Config.padX / padY are symmetric outer padding: they
// participate in layout and are included in the returned totalSize. Group
// offset (Adjust.OffsetX / OffsetY) is a visual translation that must NOT
// reserve space, so it is deliberately not handled here — the mod applies it
// to the container it places, after this arranger has sized the group.
//
// Every group is centered on its cross axis by Config.justify.

#include <algorithm>
#include <cwctype>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

namespace windhawk_mod_templates::nested_group_layout {

enum class Axis { Horizontal, Vertical };  // node orientation, not a setting
enum class Justify { Start, Center, End };
enum class FillOrder { Rows, Columns };

// An item is sized either absolutely (width x height) or RELATIVE TO THE AXIS
// its group happens to lay out along. Axis-relative sizing exists because an
// item like a Task View button should be "as wide as it needs and as tall as
// the buttons beside it" when it is a column, and the mirror image when it is
// a row — and in a hand-written arrangement the mod cannot know which it will
// be. The parent group knows its own axis, so it resolves this at measure and
// arrange time:
//
//   thickness — extent ALONG the group's axis (its width as a column, its
//               height as a row)
//   cross     — extent ACROSS the group's axis; 0 means fill, i.e. match
//               whatever the rest of the group measures
struct Size {
    double width = 0.0;
    double height = 0.0;
    bool axisRelative = false;
    double thickness = 0.0;
    double cross = 0.0;

    bool Empty() const {
        return axisRelative ? thickness <= 0.0
                            : (width <= 0.0 || height <= 0.0);
    }
};

// Size an item against its group's axis. cross = 0 fills the group.
inline Size AlongAxis(double thickness, double cross = 0.0) {
    Size size;
    size.axisRelative = true;
    size.thickness = thickness;
    size.cross = cross;
    return size;
}

// CONTENT-SIZED ITEMS. A settings-driven item size describes a GLYPH: a box of
// a chosen width that a character is centered in. It does not describe TEXT.
// "9%", "80%", and "100%" are three different widths, a font or locale change
// moves them again, and a battery percentage grows while you watch it. Handing
// such an item the same fixed width as its neighbours reserves too little
// space, and the overflow is discovered at paint time — as a clipped edge.
//
// The SizeResolver is a callback precisely so a mod can answer with something
// it measured. Measure the live element (native_glyph_surface::MeasureNatural)
// and pass the result through here: the arrangement then RESERVES the real
// width, the group's total grows to match, and nothing clips.
//
// `minimum` keeps a short value from collapsing below the item size the user
// chose, so "9%" still lines up with the glyphs above it. Round `measured` up
// and add a pixel or two of slack, or the item will re-measure every time its
// text ticks over.
inline Size ContentAlong(double measured, double minimum, double cross) {
    return {std::max(measured, minimum), cross};
}

// Cosmetic per-leaf nudge parsed from the expression's "[dx,dy]" suffix.
struct Offset {
    double x = 0.0;
    double y = 0.0;
};

struct Config {
    double spacing = 0.0;
    Justify justify = Justify::Center;
    double padX = 0.0;  // reserved on BOTH left and right
    double padY = 0.0;  // reserved on BOTH top and bottom
};

struct Placement {
    std::wstring token;
    double x = 0.0;
    double y = 0.0;
    Size size;
};

struct Node {
    std::wstring token;            // non-empty = leaf
    Offset offset;                 // from the "[dx,dy]" suffix; leaf or group
    std::vector<Node> children;    // group children, laid along axis
    Axis axis = Axis::Horizontal;  // group axis (unused for leaves)
};

// Where an arrangement stopped making sense, and what was expected there.
// Report both: a hand-edited expression is much easier to fix with a column
// number than with "did not parse".
struct ParseError {
    size_t position = 0;
    std::wstring expected;
};

class Parser {
public:
    explicit Parser(std::wstring const& text) : text_(text) {}

    bool Run(Node& root) {
        position_ = 0;
        valid_ = true;
        root = ParseExpr();
        SkipSpace();
        if (valid_ && position_ < text_.size())
            Fail(position_, L"a separator ('|' or ',') or end of arrangement");
        return valid_;
    }

    ParseError const& Error() const { return error_; }

private:
    void Fail(size_t position, wchar_t const* expected) {
        if (valid_) {  // keep the first failure; later ones are fallout
            valid_ = false;
            error_ = {position, expected};
        }
    }

    Node ParseExpr() {
        Node node;
        node.axis = Axis::Horizontal;
        node.children.push_back(ParseStack());
        while (Peek() == L'|') {
            ++position_;
            node.children.push_back(ParseStack());
        }
        return node;
    }

    Node ParseStack() {
        Node node;
        node.axis = Axis::Vertical;
        node.children.push_back(ParseUnit());
        while (Peek() == L',') {
            ++position_;
            node.children.push_back(ParseUnit());
        }
        return node;
    }

    Node ParseUnit() {
        SkipSpace();
        if (position_ < text_.size() && text_[position_] == L'(') {
            ++position_;
            Node inner = ParseExpr();
            SkipSpace();
            if (position_ < text_.size() && text_[position_] == L')')
                ++position_;
            else
                Fail(position_, L"a closing ')'");
            // A group takes an offset too, moving everything inside it.
            if (position_ < text_.size() && text_[position_] == L'[')
                inner.offset = ParseOffset();
            return inner;
        }

        Node leaf;
        size_t start = position_;
        while (position_ < text_.size() && !IsDelimiter(text_[position_]))
            ++position_;
        leaf.token = text_.substr(start, position_ - start);
        if (leaf.token.empty()) {
            Fail(position_, L"a name");
            return leaf;
        }
        if (position_ < text_.size() && text_[position_] == L'[')
            leaf.offset = ParseOffset();
        return leaf;
    }

    // "[dx,dy]" — signs optional, spaces allowed, both components required.
    Offset ParseOffset() {
        ++position_;  // consume '['
        Offset offset;
        offset.x = ParseNumber();
        SkipSpace();
        if (position_ < text_.size() && text_[position_] == L',')
            ++position_;
        else
            Fail(position_, L"a ',' between the x and y offsets");
        offset.y = ParseNumber();
        SkipSpace();
        if (position_ < text_.size() && text_[position_] == L']')
            ++position_;
        else
            Fail(position_, L"a closing ']'");
        return offset;
    }

    double ParseNumber() {
        SkipSpace();
        wchar_t* end = nullptr;
        double value = std::wcstod(text_.c_str() + position_, &end);
        size_t consumed = end ? (size_t)(end - (text_.c_str() + position_)) : 0;
        if (!consumed) {
            Fail(position_, L"a number");
            return 0.0;
        }
        position_ += consumed;
        return value;
    }

    static bool IsDelimiter(wchar_t c) {
        return c == L'|' || c == L',' || c == L'(' || c == L')' ||
               c == L'[' || c == L']' || iswspace(c);
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
    size_t position_ = 0;
    bool valid_ = true;
    ParseError error_;
};

inline bool Parse(std::wstring const& text, Node& root,
                  ParseError* error = nullptr) {
    Parser parser(text);
    bool ok = parser.Run(root);
    if (!ok && error)
        *error = parser.Error();
    return ok;
}

// ---- Token vocabulary -------------------------------------------------------
//
// A token is an item's stable IDENTITY, never its displayed label. Labels are
// not unique, can contain the expression's own delimiters, can be empty or an
// emoji, and renaming one would silently break an arrangement the user wrote.
// Each mod declares its vocabulary and documents it:
//
//   fixed set     -> semantic names: wifi, volume, battery, percent, clock
//   dynamic set   -> 1, 2, 3, ... because the set changes at runtime
//   either        -> an extra named item such as "master"
//
// A dynamic mod may accept a readable alias for a number (desktop2 == 2). Log
// the token-to-label map next to the arrangement so a user can tell which
// number is which item without the arrangement depending on the labels.
//
// Matching is case-insensitive: someone typing "Wifi" means wifi.

inline bool TokenIs(std::wstring const& token, wchar_t const* name) {
    size_t i = 0;
    for (; i < token.size() && name[i]; ++i)
        if (towlower(token[i]) != towlower(name[i]))
            return false;
    return i == token.size() && !name[i];
}

// "desktop2" -> 2 with prefix L"desktop"; 0 when the token does not match.
inline int TokenIndexWithPrefix(std::wstring const& token,
                                wchar_t const* prefix) {
    size_t i = 0;
    for (; prefix[i]; ++i)
        if (i >= token.size() || towlower(token[i]) != towlower(prefix[i]))
            return 0;
    if (i >= token.size())
        return 0;
    int value = 0;
    for (; i < token.size(); ++i) {
        if (token[i] < L'0' || token[i] > L'9')
            return 0;
        value = value * 10 + (token[i] - L'0');
    }
    return value;
}

using SizeResolver = std::function<Size(std::wstring const&)>;

inline Size Measure(Node const& node, Config const& config,
                    SizeResolver const& resolve) {
    if (!node.token.empty())
        return resolve(node.token);

    // The grammar wraps every unit in a group, so most groups have a single
    // child. Such a group IS its child — pass the size through verbatim, or an
    // axis-relative child would be flattened into a concrete size by its own
    // wrapper before the real parent ever sees it.
    {
        Node const* only = nullptr;
        int visible = 0;
        for (auto const& child : node.children) {
            if (Measure(child, config, resolve).Empty())
                continue;
            only = &child;
            if (++visible > 1)
                break;
        }
        if (visible == 1)
            return Measure(*only, config, resolve);
    }

    double main = 0.0;
    double cross = 0.0;
    double fillFallback = 0.0;
    int placed = 0;
    for (auto const& child : node.children) {
        Size size = Measure(child, config, resolve);
        if (size.Empty())
            continue;
        double childMain, childCross;
        if (size.axisRelative) {
            childMain = size.thickness;
            // A filling item takes its cross extent FROM the group, so it must
            // not drive the group's cross size — otherwise it would size itself.
            childCross = size.cross;
            fillFallback = std::max(fillFallback, size.thickness);
        } else {
            childMain =
                node.axis == Axis::Horizontal ? size.width : size.height;
            childCross =
                node.axis == Axis::Horizontal ? size.height : size.width;
        }
        main += (placed ? config.spacing : 0.0) + childMain;
        cross = std::max(cross, childCross);
        ++placed;
    }
    if (!placed)
        return {};
    // Degenerate case: every child fills, so nothing established a cross size.
    // Fall back to the largest thickness rather than collapsing the group.
    if (cross <= 0.0)
        cross = fillFallback;
    return node.axis == Axis::Horizontal ? Size{main, cross}
                                         : Size{cross, main};
}

// Resolve a child's size against its parent group's axis, so an axis-relative
// item becomes concrete width x height.
inline Size ConcreteSize(Size const& size, Axis axis, Size const& groupTotal) {
    if (!size.axisRelative)
        return size;
    double groupCross =
        axis == Axis::Horizontal ? groupTotal.height : groupTotal.width;
    double cross = size.cross > 0.0 ? size.cross : groupCross;
    return axis == Axis::Horizontal ? Size{size.thickness, cross}
                                    : Size{cross, size.thickness};
}

inline void Arrange(Node const& node, Config const& config,
                    SizeResolver const& resolve, double x, double y,
                    std::vector<Placement>& out,
                    Size const* resolvedSize = nullptr) {
    if (!node.token.empty()) {
        Size size = resolvedSize ? *resolvedSize : resolve(node.token);
        if (!size.Empty())
            out.push_back(
                {node.token, x + node.offset.x, y + node.offset.y, size});
        return;
    }

    Size total = Measure(node, config, resolve);
    if (total.Empty())
        return;
    // A group's own offset moves everything inside it and nothing outside.
    x += node.offset.x;
    y += node.offset.y;

    // Single-child group: forward the size the real parent already resolved,
    // so axis-relative sizing survives the grammar's per-unit wrapper.
    {
        Node const* only = nullptr;
        int visible = 0;
        for (auto const& child : node.children) {
            if (Measure(child, config, resolve).Empty())
                continue;
            only = &child;
            if (++visible > 1)
                break;
        }
        if (visible == 1) {
            Arrange(*only, config, resolve, x, y, out, resolvedSize);
            return;
        }
    }

    double cursor = node.axis == Axis::Horizontal ? x : y;
    for (auto const& child : node.children) {
        Size measured = Measure(child, config, resolve);
        if (measured.Empty())
            continue;
        Size size = ConcreteSize(measured, node.axis, total);
        double unused = node.axis == Axis::Horizontal
                            ? total.height - size.height
                            : total.width - size.width;
        double crossOffset = config.justify == Justify::Center ? unused / 2.0
                             : config.justify == Justify::End  ? unused
                                                               : 0.0;
        if (node.axis == Axis::Horizontal) {
            Arrange(child, config, resolve, cursor, y + crossOffset, out,
                    &size);
            cursor += size.width + config.spacing;
        } else {
            Arrange(child, config, resolve, x + crossOffset, cursor, out,
                    &size);
            cursor += size.height + config.spacing;
        }
    }
}

// Parse + measure + arrange in one call. Returns false only on a parse error
// (unbalanced parentheses, malformed offset, trailing garbage) — the caller
// should then fall back to the auto expression and log that it did.
// placements come back in expression order; totalSize is the group's bounding
// box INCLUDING outer padding. A per-item offset shifts its leaf without
// changing totalSize or any neighbor.
inline bool Compute(std::wstring const& text, Config const& config,
                    SizeResolver const& resolve,
                    std::vector<Placement>& placements, Size& totalSize,
                    ParseError* error = nullptr) {
    Node root;
    if (!Parse(text, root, error))
        return false;
    Size inner = Measure(root, config, resolve);
    placements.clear();
    if (inner.Empty()) {
        // No visible items: an empty group has no padded box either.
        totalSize = {};
        return true;
    }
    if (inner.axisRelative) {
        // The whole arrangement is one axis-relative item, so there is no group
        // for it to fill against; square it off on its own thickness.
        double cross = inner.cross > 0.0 ? inner.cross : inner.thickness;
        inner = Size{inner.thickness, cross};
    }
    Arrange(root, config, resolve, config.padX, config.padY, placements,
            &inner);
    totalSize = {inner.width + config.padX * 2.0,
                 inner.height + config.padY * 2.0};
    return true;
}

// ---- Taskbar metrics --------------------------------------------------------
//
// The taskbar rect comes from GetWindowRect in PHYSICAL pixels while every XAML
// size is a DIP. Dividing one by the other is the DPI bug flagged on PR #4855
// (blocking) and #4843. The mod supplies the raw numbers:
//
//   RECT r{}; GetWindowRect(hTaskbarWnd, &r);
//   int rows = AvailableRows(r.bottom - r.top, GetDpiForWindow(hTaskbarWnd),
//                            itemHeight, spacing);

inline double PixelsToDip(double physicalPixels, unsigned dpi) {
    return dpi ? physicalPixels * 96.0 / (double)dpi : physicalPixels;
}

// How many item rows fit in a height already expressed in DIPs. Pitch is one
// item plus one gap; the trailing gap of the last row is not required, hence
// the + spacing.
//
// RESERVE FIRST. This is the height available to the ITEM GRID, not the whole
// taskbar. Anything else that occupies vertical space — outer padY, an extra
// item shaped as a row (a sliver above or below) — must be subtracted before
// calling, or the grid claims height that is already spoken for and the
// assembled group overflows its host.
inline int RowsInHeight(double heightDip, double itemHeight, double spacing) {
    double pitch = itemHeight + std::max(0.0, spacing);
    if (pitch <= 0.0 || heightDip <= 0.0)
        return 1;
    return std::max(1, (int)((heightDip + std::max(0.0, spacing)) / pitch));
}

// Convenience for the common case with nothing else reserved.
inline int AvailableRows(double taskbarHeightPx, unsigned dpi,
                         double itemHeight, double spacing) {
    return RowsInHeight(PixelsToDip(taskbarHeightPx, dpi), itemHeight, spacing);
}

// ---- The auto shape ---------------------------------------------------------
//
// Deterministic, not scored. Take the smallest column count reachable within
// the available rows — that is what "use the taskbar's height" means — and
// among the row counts that produce it, the one with the fewest empty slots.
// So 4 items with 3 rows available gives 2x2 rather than a ragged 3+1, and 5
// items with 4 rows available gives 3x2 rather than 4+1.

struct Shape {
    int rows = 1;
    int columns = 1;
};

inline Shape ChooseShape(int count, int maxRows) {
    if (count <= 0)
        return {0, 0};
    int limit = std::max(1, std::min(maxRows, count));
    Shape best{1, count};
    int bestWaste = 0;
    bool first = true;
    for (int rows = 1; rows <= limit; ++rows) {
        int columns = (count + rows - 1) / rows;
        int waste = rows * columns - count;
        if (first || columns < best.columns ||
            (columns == best.columns && waste < bestWaste)) {
            first = false;
            best = {rows, columns};
            bestWaste = waste;
        }
    }
    return best;
}

// ---- Expression generation --------------------------------------------------
//
// Turn a rows x columns shape into an expression so the auto path and the
// manual path are the same code below this point. Positions fill row-major
// (left to right, then down) for FillOrder::Rows or column-major (top to
// bottom, then right) for FillOrder::Columns. Grid positions past `count` are
// simply absent, so a ragged final row or column yields fewer tokens and the
// result is always a valid expression. Justify aligns that ragged group.
//
// Tokens come from namer(index); the default names items by 1-based number,
// matching what a user reads on screen. The caller's SizeResolver must map
// those same names back to pixel sizes.

using TokenNamer = std::function<std::wstring(int index)>;

inline std::wstring BuildGridExpression(int count, int rows, int columns,
                                        FillOrder fill,
                                        TokenNamer const& namer = {}) {
    if (count <= 0 || rows <= 0 || columns <= 0)
        return {};

    auto name = [&](int index) -> std::wstring {
        return namer ? namer(index) : std::to_wstring(index + 1);
    };

    // '|' groups are columns, ',' units are rows, always.
    std::wstring expr;
    for (int column = 0; column < columns; ++column) {
        std::wstring stack;
        for (int row = 0; row < rows; ++row) {
            int index = fill == FillOrder::Rows ? row * columns + column
                                                : column * rows + row;
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

inline std::wstring BuildAutoExpression(int count, int maxRows, FillOrder fill,
                                        TokenNamer const& namer = {}) {
    Shape shape = ChooseShape(count, maxRows);
    return BuildGridExpression(count, shape.rows, shape.columns, fill, namer);
}

// ---- Items the arrangement forgot -------------------------------------------
//
// A hand-written arrangement names the items that existed when it was written.
// When the set is dynamic — a desktop is added, a folder appears — the new item
// is in no group, resolves to nothing, and silently vanishes from the taskbar.
// That is a trap, so a mod with a dynamic set offers a policy:
//
//   Append (default) — arrange the unlisted items automatically and put that
//                      block after everything the user wrote, so a new item is
//                      always reachable and the written block stays intact.
//   Ignore           — the arrangement is the whole truth; unlisted items stay
//                      off the taskbar until the user adds them.
//
// A mod that appends should log that it did, so the user knows to fold the new
// item into their arrangement when they next edit it.

// Whether a token the user wrote refers to the same item as one the mod
// expects. Defaults to a case-insensitive name match, which is WRONG for any
// mod that accepts aliases: "desktop1" and "1" are the same button, and
// comparing them as strings makes every aliased item look missing and get
// appended a second time. A mod with a vocabulary must supply this.
using TokenMatcher =
    std::function<bool(std::wstring const& placed, std::wstring const& expected)>;

inline std::vector<std::wstring> MissingTokens(
    std::vector<std::wstring> const& expected,
    std::vector<Placement> const& placements,
    TokenMatcher const& same = {}) {
    std::vector<std::wstring> missing;
    for (auto const& token : expected) {
        bool found = false;
        for (auto const& placement : placements) {
            bool match = same ? same(placement.token, token)
                              : TokenIs(placement.token, token.c_str());
            if (match) {
                found = true;
                break;
            }
        }
        if (!found)
            missing.push_back(token);
    }
    return missing;
}

inline std::wstring AppendMissing(std::wstring const& expression,
                                  std::vector<std::wstring> const& missing,
                                  int maxRows, FillOrder fill) {
    if (missing.empty())
        return expression;
    auto namer = [&missing](int index) { return missing[index]; };
    std::wstring block = BuildAutoExpression((int)missing.size(), maxRows, fill,
                                             namer);
    if (block.empty())
        return expression;
    if (expression.empty())
        return block;
    return L"(" + expression + L") | (" + block + L")";
}

// ---- The one setting --------------------------------------------------------
//
// Resolve `Layout.Arrangement` to the expression to arrange. Empty or the word
// "auto" (any case, surrounding space ignored) means generate one. The caller
// logs the result when wasAuto is true so the user can paste it back into the
// same field and edit it.

struct Arrangement {
    std::wstring expression;
    bool wasAuto = false;
};

inline bool IsAutoSetting(std::wstring const& setting) {
    size_t first = setting.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos)
        return true;
    size_t last = setting.find_last_not_of(L" \t\r\n");
    std::wstring trimmed = setting.substr(first, last - first + 1);
    if (trimmed.size() != 4)
        return false;
    for (size_t i = 0; i < 4; ++i)
        if (towlower(trimmed[i]) != L"auto"[i])
            return false;
    return true;
}

inline Arrangement ResolveArrangement(std::wstring const& setting, int count,
                                      int maxRows, FillOrder fill,
                                      TokenNamer const& namer = {}) {
    if (IsAutoSetting(setting))
        return {BuildAutoExpression(count, maxRows, fill, namer), true};
    return {setting, false};
}

}  // namespace windhawk_mod_templates::nested_group_layout

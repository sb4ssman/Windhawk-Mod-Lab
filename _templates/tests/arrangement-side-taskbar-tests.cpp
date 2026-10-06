// Side-taskbar behaviour of both arrangement components. A written
// arrangement is laid out exactly as written on every taskbar edge; only the
// generated pieces - "auto" and the block appended for items an arrangement
// does not name - learn which way the taskbar runs (`across`). Includes the
// component bodies directly, so it tests exactly what assemble.py ships.
// Build with Windhawk's clang, -static:
//   clang++ -std=c++20 -O1 -static arrangement-side-taskbar-tests.cpp -o t.exe

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cwctype>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace plain {
#include "../components/arrangement-expression/body.h"
}
namespace axis {
#include "../components/arrangement-expression-axis/body.h"
}

// Namespaces cannot be template arguments; these adapters expose each
// component's types and entry points to the shared suite.
#define ADAPTER(Name, ns)                                                     \
    struct Name {                                                             \
        using Size = ns::Size;                                                \
        using Config = ns::Config;                                            \
        using Placement = ns::Placement;                                      \
        using FillOrder = ns::FillOrder;                                      \
        static bool Compute(std::wstring const& text, Config const& config,   \
                            ns::SizeResolver const& resolve,                  \
                            std::vector<Placement>& placements, Size& total) { \
            return ns::Compute(text, config, resolve, placements, total);     \
        }                                                                     \
        static std::wstring Auto(int count, int maxLines, FillOrder fill,     \
                                 bool across) {                               \
            return ns::BuildAutoExpression(count, maxLines, fill, {}, across); \
        }                                                                     \
        static std::wstring Append(std::wstring const& expression,           \
                                   std::vector<std::wstring> const& missing,  \
                                   int maxLines, bool across) {               \
            return ns::AppendMissing(expression, missing, maxLines,           \
                                     FillOrder::Rows, across);                \
        }                                                                     \
        static std::wstring Resolve(std::wstring const& setting, int count,   \
                                    int maxLines, bool across) {              \
            return ns::ResolveArrangement(setting, count, maxLines,           \
                                          FillOrder::Rows, {}, across)        \
                .expression;                                                  \
        }                                                                     \
    };
ADAPTER(Plain, plain)
ADAPTER(Axis, axis)

static bool Near(double a, double b) { return std::fabs(a - b) < 0.001; }

template <typename Placement>
static Placement const* Find(std::vector<Placement> const& placements,
                             wchar_t const* token) {
    for (auto const& placement : placements)
        if (placement.token == token) return &placement;
    return nullptr;
}

template <typename NS>
static void RunSuite() {
    using Size = typename NS::Size;
    using Placement = typename NS::Placement;
    using FillOrder = typename NS::FillOrder;
    auto square24 = [](std::wstring const&) -> Size { return {24, 24}; };
    typename NS::Config config;
    std::vector<Placement> placements;
    Size total;

    // ---- auto down a side taskbar fills ACROSS its width -------------------
    // Four items, six fit across: one row of four, not a column.
    assert(NS::Auto(4, 6, FillOrder::Rows, true) == L"1 | 2 | 3 | 4");
    // ...and on a bottom taskbar with two rows: the familiar 2x2.
    assert(NS::Auto(4, 2, FillOrder::Rows, false) == L"1, 3 | 2, 4");

    // Five items, three across: fewest rows (2), rows first ON SCREEN -
    // 1 2 3 on top, 4 5 below. (The short third column holds only 3, which
    // Justify centers vertically - that is the ragged-group setting, so its
    // y is not asserted.)
    std::wstring side = NS::Auto(5, 3, FillOrder::Rows, true);
    assert(NS::Compute(side, config, square24, placements, total));
    auto at = [&](wchar_t const* token) { return Find(placements, token); };
    assert(Near(at(L"1")->y, at(L"2")->y));
    assert(at(L"2")->x > at(L"1")->x && at(L"3")->x > at(L"2")->x);
    assert(at(L"4")->y > at(L"1")->y && Near(at(L"4")->x, at(L"1")->x));
    assert(Near(total.width, 72) && Near(total.height, 48));

    // Columns first on screen: 1 2 down the first column.
    side = NS::Auto(5, 3, FillOrder::Columns, true);
    assert(NS::Compute(side, config, square24, placements, total));
    assert(Near(at(L"2")->x, at(L"1")->x) && at(L"2")->y > at(L"1")->y);

    // ---- items the arrangement forgot: below on a side taskbar -------------
    assert(NS::Append(L"a | b", {L"c"}, 6, true) == L"(a | b), (c)");
    assert(NS::Append(L"a | b", {L"c"}, 2, false) == L"(a | b) | (c)");
    assert(NS::Append(L"a", {}, 2, true) == L"a");

    // ---- a written arrangement is literal on every edge ----------------------
    // ResolveArrangement never rewrites what the user wrote.
    assert(NS::Resolve(L"a | b[3,-2]", 4, 6, true) == L"a | b[3,-2]");
    assert(NS::Resolve(L"auto", 2, 6, true) == L"1 | 2");
    // And a nudge means what it says: 3 right, 2 up.
    assert(NS::Compute(L"a | b[3,-2]", config, square24, placements, total));
    assert(Near(at(L"b")->x, 27) && Near(at(L"b")->y, -2));
}

int main() {
    RunSuite<Plain>();
    RunSuite<Axis>();
    std::puts("ARRANGEMENT_SIDE_TASKBAR_TESTS_OK");
    return 0;
}

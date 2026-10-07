"""Test the actual shipped margin compensation for both native stacking axes."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'tray-utility-customizer/tray-utility-customizer.wh.cpp').read_text(encoding='utf-8')
start = source.index('static Thickness CompensatedIconMargin(')
end = source.index('\n}\n', start) + 3
helper = source[start:end]
size_start = source.index('static ngl::Size NativeItemSize(')
size_end = source.index('\n}\n', size_start) + 3
size_helper = source[size_start:size_end]
program = r'''
#include <algorithm>
#include <cassert>
#include <cmath>
struct Thickness { double Left, Top, Right, Bottom; };
struct IconTarget { double x, y, width, height; };
namespace ngl { struct Size { double width, height; }; }
''' + size_helper + helper + r'''
void Check(bool vertical, IconTarget const* targets, int count) {
    double flow = 0;
    for (int i = 0; i < count; ++i) {
        double nativeOrigin = flow;
        auto margin = CompensatedIconMargin(targets[i], vertical, flow);
        assert(std::abs(margin.Left + (vertical ? 0 : nativeOrigin) - targets[i].x) < 0.001);
        assert(std::abs(margin.Top + (vertical ? nativeOrigin : 0) - targets[i].y) < 0.001);
        assert(flow >= nativeOrigin);
    }
}
int main() {
    auto top = NativeItemSize(24, 160, false);
    auto left = NativeItemSize(160, 24, true);
    auto right = NativeItemSize(140, 24, true);
    assert(top.width == 24 && top.height == 24);
    assert(left.width == 24 && left.height == 24);
    assert(right.width == 24 && right.height == 24);
    // Three native side controls occupy 72 DIP, not 3 * the full tray width.
    assert(left.width * 3 == 72 && right.width * 3 == 72);
    auto label = NativeItemSize(64, 40, false);
    assert(label.width == 64 && label.height == 40);
    auto pending = NativeItemSize(0, 0, true);
    assert(pending.width == 24 && pending.height == 24);
    IconTarget row[] = {{0,0,24,24}, {24,0,24,24}, {48,0,24,24}};
    IconTarget reverse[] = {{48,0,24,24}, {24,0,24,24}, {0,0,24,24}};
    IconTarget column[] = {{0,0,24,24}, {0,24,24,24}, {0,48,24,24}};
    IconTarget nudges[] = {{-2,3,24,24}, {30,-1,20,22}, {1,35,18,24}};
    for (bool vertical : {false, true}) {
        Check(vertical, row, 3); Check(vertical, reverse, 3);
        Check(vertical, column, 3); Check(vertical, nudges, 3);
    }
}
'''
with tempfile.TemporaryDirectory(prefix='tray-flow-') as temp:
    cpp = Path(temp) / 'test.cpp'
    exe = Path(temp) / 'test.exe'
    cpp.write_text(program, encoding='utf-8')
    subprocess.run(['C:/Program Files/Windhawk/Compiler/bin/clang++.exe',
                    '-std=c++20', '-static', str(cpp), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print('TRAY_NATIVE_FLOW_REGRESSION_OK')

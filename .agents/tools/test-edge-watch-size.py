"""Exercise the shipped SizeChanged decision without a live XAML taskbar."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / '_templates/components/taskbar-metrics/body.h').read_text()
begin = source.index('            auto size = args.NewSize();')
end = source.index('\n        });', begin)
handler = source[begin:end]
program = r'''
#include <cmath>
#include <cassert>
struct Size { double Width, Height; };
struct Args { Size value; Size NewSize() const { return value; } };
int callbacks = 0;
void Changed() { ++callbacks; }
struct Watch { bool side; double thickness; void (*onChange)() = Changed; };
void Notify(Watch* target, Args args) {
''' + handler + r'''
}
int main() {
    Watch horizontal{false, 48};
    Notify(&horizontal, {{700, 48}}); // content-sized frame grows
    Notify(&horizontal, {{100, 48}}); // and shrinks
    assert(callbacks == 0);
    Notify(&horizontal, {{100, 48.4}});
    assert(callbacks == 0);
    Notify(&horizontal, {{100, 48.5}}); // threshold includes exactly 0.5
    assert(callbacks == 1);
    Notify(&horizontal, {{60, 900}}); // native move to a side
    assert(callbacks == 2 && horizontal.side && horizontal.thickness == 60);
    Notify(&horizontal, {{60, 700}}); // side length only
    assert(callbacks == 2);
    Notify(&horizontal, {{61, 700}}); // side thickness
    assert(callbacks == 3);
    Notify(&horizontal, {{900, 48}}); // return to horizontal
    assert(callbacks == 4 && !horizontal.side);
    horizontal.onChange = nullptr;
    Notify(&horizontal, {{900, 50}});
    assert(callbacks == 4 && horizontal.thickness == 50);
}
'''
with tempfile.TemporaryDirectory(prefix='edge-watch-') as temporary:
    cpp = Path(temporary) / 'test.cpp'
    exe = Path(temporary) / 'test.exe'
    cpp.write_text(program)
    subprocess.run(['C:/Program Files/Windhawk/Compiler/bin/clang++.exe',
                    '-std=c++23', '-static', str(cpp), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print('EDGE_WATCH_SIZE_REGRESSION_OK')

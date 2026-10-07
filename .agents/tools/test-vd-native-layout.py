from pathlib import Path
root=Path(__file__).resolve().parents[2]
source=(root/'taskbar-vd-switcher/taskbar-vd-switcher.wh.cpp').read_text(encoding='utf-8')
def function(signature):
    start=source.index(signature)
    opening=source.index('{',start)
    depth=1;end=opening+1
    while depth:
        depth += (source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]+'\n'
body=(root/'_templates/components/arrangement-expression-axis/body.h').as_posix()
start=source.index('enum class VdPosition {')
end=source.index('// Settings were reorganised',start)
definitions=source[start:end]
functions=['static bool TaskViewInGrid(', 'static bool TaskViewIsVerticalPlacement(',
'static bool TaskViewTakesALine(', 'static int AvailableRows(',
'static ngl::Config MakeLayoutConfig(', 'static ngl::FillOrder LayoutFillOrder(',
'static int DesktopIndexFromToken(', 'static bool IsMasterToken(',
'static ngl::Size ResolveLayoutToken(', 'static std::wstring MasterToken(',
'static std::wstring AddTaskViewButton(', 'static std::vector<std::wstring> ExpectedTokens(',
'static bool ComputeButtonPlacements(', 'static double CompactExtent(',
'static ngl::Size AutoCell(']
harness='''#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cwctype>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#define CHECK(test) do { if (!(test)) {std::fprintf(stderr,"FAIL line %d: %s\\n",__LINE__,#test);std::exit(1);} } while(0)
namespace ngl {
'''+ '#include "'+body+'"\n}\n'+definitions+'''
static bool g_side=false;
static HWND g_taskbarWnd=nullptr;
static void Wh_Log(wchar_t const*,...) {}
namespace taskbar_window { static HWND ResolveTaskbarWnd(HWND w) {return w;} }
namespace taskbar_metrics {
enum class Orientation {Horizontal,Vertical};
struct Metrics { bool valid=true;double constrainedDip=40;UINT dpi=96;Orientation orientation=Orientation::Horizontal; };
static Metrics fixture;
static Metrics GetMetrics(HWND) {return fixture;}
static wchar_t const* OrientationName(Orientation) {return L"fixture";}
}
struct Bar { HWND window=nullptr,taskbar=nullptr;double scale=1;int count=2,current=0;bool side=false; };
static std::wstring GetButtonLabel(int i,int) {return std::to_wstring(i+1);}
'''+ ''.join(function(signature) for signature in functions)+'''
static void Run(int count,bool master,double scale=1) {
    g_settings=ModSettings{};
    g_settings.arrangement=L"auto";
    g_settings.fillOrder=ngl::FillOrder::Columns;
    g_settings.taskViewButton=master;
    g_settings.taskViewPlacement=VdTaskViewPlacement::InGrid;
    taskbar_metrics::fixture={true,40,UINT(96*scale),taskbar_metrics::Orientation::Horizontal};
    Bar bar;bar.count=count;bar.scale=scale;
    auto cell=AutoCell(bar);
    std::vector<ngl::Placement> placements;ngl::Size total;
    CHECK(ComputeButtonPlacements(count,placements,total,false,&cell));
    CHECK(placements.size()==size_t(count+(master?1:0)));
    CHECK(total.height<=40+0.0001);
    CHECK(cell.height<=19.0001); // REAL GDI font; old +4 margin failed this
    CHECK(placements[0].x==placements[1].x && placements[0].y<placements[1].y);
    if (master) {
        CHECK(placements.size()==4);
        auto const& taskView=placements.back();
        CHECK(IsMasterToken(taskView.token));
        CHECK(taskView.x>placements[0].x && taskView.y>placements[0].y);
        CHECK(taskView.size.width==placements[0].size.width);
        CHECK(taskView.size.height==placements[0].size.height);
    }
    std::printf("REAL_FONT_LAYOUT_OK desktops=%d taskview=%d scale=%.2f cell=%.2fx%.2f group=%.2fx%.2f\\n",
        count,master,scale,cell.width,cell.height,total.width,total.height);
}
int main() {
    Run(2,false);Run(3,false);Run(3,true);
    // Icon-font metrics must participate without changing the grid-cell size.
    g_settings.taskViewFontFamily=L"Segoe MDL2 Assets";
    Bar iconBar;iconBar.count=3;
    auto cell=AutoCell(iconBar);
    CHECK(cell.height<=19.0001);
    for (double scale:{1.25,1.5,2.0}) {Run(2,false,scale);Run(3,true,scale);}
    // Manual sizes stay literal; row-first retains the configured dimensions.
    g_settings=ModSettings{};g_settings.arrangement=L"1, 2";
    std::vector<ngl::Placement> placements;ngl::Size total;
    CHECK(ComputeButtonPlacements(2,placements,total));CHECK(total.height==46);
    g_settings.arrangement=L"auto";
    CHECK(ComputeButtonPlacements(2,placements,total));
    CHECK(total.height==22 && total.width==42);
    std::puts("VD_NATIVE_LAYOUT_PIPELINE_OK");
}
'''
import subprocess
import tempfile
with tempfile.TemporaryDirectory(prefix='vd-native-layout-') as folder:
    temporary=Path(folder)
    cpp=temporary/'test.cpp'
    exe=temporary/'test.exe'
    cpp.write_text(harness,encoding='utf-8')
    subprocess.run([r'C:\Program Files\Windhawk\Compiler\bin\clang++.exe',
        '-std=c++23','-O1','-static',str(cpp),'-lgdi32','-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)


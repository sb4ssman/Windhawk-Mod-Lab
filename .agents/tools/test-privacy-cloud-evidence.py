"""Compile the candidate's actual detector with deterministic Windows evidence."""
from pathlib import Path
import subprocess
import tempfile

source = Path('privacy-indicator-anchor/privacy-indicator-anchor.wh.cpp').read_text(encoding='utf-8')
detector = source[source.index('// Read-only evidence.'):source.index('// Refresh only the domains')]
stub = r'''
#include <windows.h>
#include <tlhelp32.h>
#include <atomic>
#include <cwchar>
#include <initializer_list>
#include <cassert>
enum class PrivacyBlockReason { None, PolicyDisabled, EvidenceUnavailable };
static std::atomic<bool> g_recallItemEnabled{true}, g_onedriveItemEnabled{true};
static std::atomic<bool> g_recallRunning{false}, g_onedriveRunning{false};
static std::atomic<PrivacyBlockReason> g_recallReason{PrivacyBlockReason::EvidenceUnavailable}, g_onedriveReason{PrivacyBlockReason::EvidenceUnavailable};
static bool available=true, process=false, sameSession=true;
static DWORD policy=0, allow=1;
static LONG FakeReg(HKEY,PCWSTR,PCWSTR name,DWORD,LPDWORD,PVOID out,LPDWORD) {
    *static_cast<DWORD*>(out)=wcscmp(name,L"AllowRecallEnablement")==0 ? allow : policy;
    return ERROR_SUCCESS;
}
static HANDLE FakeSnapshot(DWORD,DWORD) { return available ? reinterpret_cast<HANDLE>(1) : INVALID_HANDLE_VALUE; }
static BOOL FakeSession(DWORD pid,DWORD* session) { *session=(pid==2 && !sameSession)?2:1; return TRUE; }
static DWORD FakePid() { return 1; }
static BOOL FakeFirst(HANDLE,PROCESSENTRY32W* pe) { pe->th32ProcessID=2; wcscpy(pe->szExeFile,process?L"OneDrive.exe":L"Other.exe"); return TRUE; }
static BOOL FakeNext(HANDLE,PROCESSENTRY32W*) { SetLastError(ERROR_NO_MORE_FILES); return FALSE; }
static BOOL FakeClose(HANDLE) { return TRUE; }
#define RegGetValueW FakeReg
#define CreateToolhelp32Snapshot FakeSnapshot
#define ProcessIdToSessionId FakeSession
#define GetCurrentProcessId FakePid
#define Process32FirstW FakeFirst
#define Process32NextW FakeNext
#define CloseHandle FakeClose
'''
tests = r'''
int main() {
    bool changed=false;
    RefreshCloudEvidence(changed);
    assert(!g_recallRunning && !g_onedriveRunning);
    assert(g_recallReason==PrivacyBlockReason::EvidenceUnavailable);
    assert(g_onedriveReason==PrivacyBlockReason::EvidenceUnavailable);
    process=true; changed=false; RefreshCloudEvidence(changed);
    assert(changed && g_onedriveRunning && g_onedriveReason==PrivacyBlockReason::None);
    sameSession=false; RefreshCloudEvidence(changed);
    assert(!g_onedriveRunning && g_onedriveReason==PrivacyBlockReason::EvidenceUnavailable);
    sameSession=true; available=false; RefreshCloudEvidence(changed);
    assert(!g_onedriveRunning && g_onedriveReason==PrivacyBlockReason::EvidenceUnavailable);
    available=true; policy=1; RefreshCloudEvidence(changed);
    assert(g_recallReason==PrivacyBlockReason::PolicyDisabled && g_onedriveReason==PrivacyBlockReason::PolicyDisabled);
    policy=0; allow=0; RefreshCloudEvidence(changed);
    assert(g_recallReason==PrivacyBlockReason::PolicyDisabled && g_onedriveReason==PrivacyBlockReason::None);
    g_recallItemEnabled=false; g_onedriveItemEnabled=false; changed=false;
    RefreshCloudEvidence(changed); assert(!changed);
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp = Path(directory)/'test.cpp'; exe=Path(directory)/'test.exe'
    cpp.write_text(stub+detector+tests,encoding='utf-8')
    subprocess.run(['C:/Program Files/Windhawk/Compiler/bin/clang++.exe', '-std=c++23', str(cpp), '-o', str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
print('PRIVACY_CLOUD_EVIDENCE_OK')

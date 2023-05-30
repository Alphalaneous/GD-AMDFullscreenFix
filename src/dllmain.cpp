#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(push, 0)
#include <cocos2d.h>
#pragma warning(pop)
#include <MinHook.h>
#include <gd.h>
#include <chrono>
#include <thread>
#include <random>
#include <CCGL.h>

LONG_PTR oWindowProc;
bool newWindowProcSet = false;

int width;
int height;

bool isFullscreen(HWND windowHandle)
{
    MONITORINFO monitorInfo = { 0 };
    monitorInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);

    RECT windowRect;
    GetWindowRect(windowHandle, &windowRect);

    return windowRect.left == monitorInfo.rcMonitor.left
        && windowRect.right == monitorInfo.rcMonitor.right
        && windowRect.top == monitorInfo.rcMonitor.top
        && windowRect.bottom == monitorInfo.rcMonitor.bottom;
}

LRESULT CALLBACK nWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

    cocos2d::CCEGLView* ccEGLView = cocos2d::CCEGLView::sharedOpenGLView();

    gd::GameManager* manager = gd::GameManager::sharedState();



    switch (msg) {

    case WM_SETFOCUS: {

        if (isFullscreen(hwnd)) {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_DRAWFRAME | SWP_FRAMECHANGED);
        }

        break;
    }
    case WM_KILLFOCUS:
        
        RECT rect;
        if (GetWindowRect(hwnd, &rect))
        {
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
        }

        if (isFullscreen(hwnd)) {
            SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 1280, 720, SWP_NOACTIVATE);
        }

        break;
    }

    return CallWindowProc((WNDPROC)oWindowProc, hwnd, msg, wparam, lparam);
}

bool(__thiscall* MenuLayer_init)(gd::MenuLayer* self);

bool __fastcall MenuLayer_init_H(gd::MenuLayer* self, void*) {

    if (!MenuLayer_init(self)) return false;



    if (!newWindowProcSet) oWindowProc = SetWindowLongPtrA(GetForegroundWindow(), GWL_WNDPROC, (LONG_PTR)nWindowProc);
    newWindowProcSet = true;

    return true;
}


DWORD WINAPI thread_func(void* hModule) {
    MH_Initialize();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(100, 2000);

    int random = distr(gen);

    std::this_thread::sleep_for(std::chrono::milliseconds(random));

    /*AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);*/

    auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
   
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1907B0),
        MenuLayer_init_H,
        reinterpret_cast<void**>(&MenuLayer_init)
    );


    MH_EnableHook(MH_ALL_HOOKS);
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {

    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x100, thread_func, handle, 0, 0);
    }
    return TRUE;
}
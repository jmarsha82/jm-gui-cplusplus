#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>

#include <algorithm>
#include <cwctype>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")

namespace
{
constexpr int InputId = 1001;
constexpr int AddButtonId = 1002;
constexpr int TaskListId = 1003;

HWND g_input = nullptr;
HWND g_addButton = nullptr;
HWND g_taskList = nullptr;
WNDPROC g_originalInputProc = nullptr;

std::vector<std::wstring> g_tasks;
bool g_sortAscending = true;

std::wstring trim(const std::wstring& text)
{
    auto first = std::find_if_not(text.begin(), text.end(), [](wchar_t ch) {
        return std::iswspace(ch) != 0;
    });

    auto last = std::find_if_not(text.rbegin(), text.rend(), [](wchar_t ch) {
        return std::iswspace(ch) != 0;
    }).base();

    if (first >= last)
    {
        return L"";
    }

    return std::wstring(first, last);
}

std::wstring getWindowText(HWND window)
{
    const int length = GetWindowTextLengthW(window);
    std::wstring text(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(window, text.data(), length + 1);
    text.resize(static_cast<size_t>(length));
    return text;
}

void refreshTaskList()
{
    ListView_DeleteAllItems(g_taskList);

    for (size_t index = 0; index < g_tasks.size(); ++index)
    {
        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(index);
        item.iSubItem = 0;
        item.pszText = const_cast<LPWSTR>(g_tasks[index].c_str());
        ListView_InsertItem(g_taskList, &item);
    }
}

void addTaskFromInput()
{
    const std::wstring task = trim(getWindowText(g_input));
    if (task.empty())
    {
        SetFocus(g_input);
        return;
    }

    g_tasks.push_back(task);
    refreshTaskList();

    SetWindowTextW(g_input, L"");
    SetFocus(g_input);
}

void sortTasksByName()
{
    std::sort(g_tasks.begin(), g_tasks.end(), [](const std::wstring& left, const std::wstring& right) {
        const int comparison = CompareStringOrdinal(
            left.c_str(),
            -1,
            right.c_str(),
            -1,
            TRUE);

        return comparison == CSTR_LESS_THAN;
    });

    if (!g_sortAscending)
    {
        std::reverse(g_tasks.begin(), g_tasks.end());
    }

    g_sortAscending = !g_sortAscending;
    refreshTaskList();
}

void layoutControls(HWND window)
{
    RECT client{};
    GetClientRect(window, &client);

    constexpr int margin = 16;
    constexpr int buttonWidth = 96;
    constexpr int controlHeight = 32;
    constexpr int gap = 8;

    const int width = client.right - client.left;
    const int height = client.bottom - client.top;
    const int contentWidth = std::max(120, width - (margin * 2));
    const int contentHeight = std::max(80, height - margin);
    const int inputWidth = std::max(80, contentWidth - buttonWidth - gap);
    const int listTop = margin + controlHeight + 14;

    MoveWindow(g_input, margin, margin, inputWidth, controlHeight, TRUE);
    MoveWindow(g_addButton, margin + inputWidth + gap, margin, buttonWidth, controlHeight, TRUE);
    MoveWindow(g_taskList, margin, listTop, contentWidth, std::max(40, contentHeight - listTop), TRUE);

    ListView_SetColumnWidth(g_taskList, 0, contentWidth - 4);
}

LRESULT CALLBACK inputProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_KEYDOWN && wParam == VK_RETURN)
    {
        addTaskFromInput();
        return 0;
    }

    return CallWindowProcW(g_originalInputProc, window, message, wParam, lParam);
}

void createControls(HWND window)
{
    g_input = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0,
        0,
        0,
        0,
        window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(InputId)),
        GetModuleHandleW(nullptr),
        nullptr);

    g_addButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"Add",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0,
        0,
        0,
        0,
        window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(AddButtonId)),
        GetModuleHandleW(nullptr),
        nullptr);

    g_taskList = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0,
        0,
        0,
        0,
        window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(TaskListId)),
        GetModuleHandleW(nullptr),
        nullptr);

    ListView_SetExtendedListViewStyle(g_taskList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMNW column{};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<LPWSTR>(L"Name");
    column.cx = 400;
    column.iSubItem = 0;
    ListView_InsertColumn(g_taskList, 0, &column);

    g_originalInputProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(g_input, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(inputProc)));

    HFONT guiFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    SendMessageW(g_input, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
    SendMessageW(g_addButton, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
    SendMessageW(g_taskList, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        createControls(window);
        layoutControls(window);
        return 0;

    case WM_SIZE:
        layoutControls(window);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == AddButtonId && HIWORD(wParam) == BN_CLICKED)
        {
            addTaskFromInput();
            return 0;
        }
        break;

    case WM_NOTIFY:
    {
        const auto* notification = reinterpret_cast<LPNMHDR>(lParam);
        if (notification->idFrom == TaskListId && notification->code == LVN_COLUMNCLICK)
        {
            sortTasksByName();
            return 0;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int commandShow)
{
    INITCOMMONCONTROLSEX commonControls{};
    commonControls.dwSize = sizeof(commonControls);
    commonControls.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&commonControls);

    const wchar_t className[] = L"TaskListAppWindow";

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&windowClass))
    {
        MessageBoxW(nullptr, L"Could not register the application window.", L"Task List App", MB_ICONERROR);
        return 1;
    }

    HWND window = CreateWindowExW(
        0,
        className,
        L"Task List App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        640,
        480,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!window)
    {
        MessageBoxW(nullptr, L"Could not create the application window.", L"Task List App", MB_ICONERROR);
        return 1;
    }

    ShowWindow(window, commandShow);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}

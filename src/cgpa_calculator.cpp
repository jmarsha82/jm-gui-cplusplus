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

#include "cgpa/cgpa_core.hpp"

#include <algorithm>
#include <cwctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")

namespace
{
constexpr int StudentInputId = 2001;
constexpr int CourseInputId = 2002;
constexpr int CreditsInputId = 2003;
constexpr int GradeComboId = 2004;
constexpr int AddCourseButtonId = 2005;
constexpr int ClearButtonId = 2006;
constexpr int CourseListId = 2007;
constexpr int SummaryTextId = 2008;

HWND g_studentInput = nullptr;
HWND g_courseInput = nullptr;
HWND g_creditsInput = nullptr;
HWND g_gradeCombo = nullptr;
HWND g_addButton = nullptr;
HWND g_clearButton = nullptr;
HWND g_courseList = nullptr;
HWND g_summaryText = nullptr;
WNDPROC g_originalCourseInputProc = nullptr;

std::vector<cgpa::CourseResult> g_courses;

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

std::wstring formatDouble(double value)
{
    std::wostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}

void setFonts()
{
    HFONT guiFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    HWND controls[] = {
        g_studentInput,
        g_courseInput,
        g_creditsInput,
        g_gradeCombo,
        g_addButton,
        g_clearButton,
        g_courseList,
        g_summaryText,
    };

    for (HWND control : controls)
    {
        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(guiFont), TRUE);
    }
}

void addStaticLabel(HWND window, const wchar_t* text, int x, int y, int width, int height)
{
    HWND label = CreateWindowExW(
        0,
        L"STATIC",
        text,
        WS_CHILD | WS_VISIBLE,
        x,
        y,
        width,
        height,
        window,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr);

    SendMessageW(label, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
}

void updateSummary()
{
    const cgpa::Summary summary = cgpa::summarize(g_courses);
    const std::wstring student = trim(getWindowText(g_studentInput));
    const std::wstring displayName = student.empty() ? L"Student" : student;

    std::wostringstream text;
    text << displayName << L"\r\n"
         << L"Courses: " << summary.courseCount << L"\r\n"
         << L"Total credits: " << summary.totalCredits << L"\r\n"
         << L"Total grade points: " << formatDouble(summary.totalGradePoints) << L"\r\n"
         << L"Semester GPA: " << formatDouble(summary.gpa) << L"\r\n"
         << L"CGPA: " << formatDouble(summary.gpa) << L"\r\n"
         << L"Overall course grade: " << summary.overallGrade;

    SetWindowTextW(g_summaryText, text.str().c_str());
}

void refreshCourseList()
{
    ListView_DeleteAllItems(g_courseList);

    for (size_t index = 0; index < g_courses.size(); ++index)
    {
        const cgpa::CourseResult& course = g_courses[index];
        const std::wstring credits = std::to_wstring(course.credits);
        const std::wstring gradePoints = formatDouble(course.gradePoint * static_cast<double>(course.credits));

        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(index);
        item.iSubItem = 0;
        item.pszText = const_cast<LPWSTR>(course.name.c_str());
        ListView_InsertItem(g_courseList, &item);

        ListView_SetItemText(g_courseList, static_cast<int>(index), 1, const_cast<LPWSTR>(credits.c_str()));
        ListView_SetItemText(g_courseList, static_cast<int>(index), 2, const_cast<LPWSTR>(course.grade.c_str()));
        ListView_SetItemText(g_courseList, static_cast<int>(index), 3, const_cast<LPWSTR>(gradePoints.c_str()));
    }

    updateSummary();
}

bool tryReadCredits(int& credits)
{
    const std::wstring text = trim(getWindowText(g_creditsInput));
    if (text.empty())
    {
        return false;
    }

    wchar_t* end = nullptr;
    const long value = wcstol(text.c_str(), &end, 10);
    if (*end != L'\0' || value <= 0 || value > 30)
    {
        return false;
    }

    credits = static_cast<int>(value);
    return true;
}

std::wstring selectedGrade()
{
    wchar_t grade[8]{};
    GetWindowTextW(g_gradeCombo, grade, static_cast<int>(std::size(grade)));
    return grade;
}

void addCourseFromInput()
{
    const std::wstring courseName = trim(getWindowText(g_courseInput));
    int credits = 0;

    if (courseName.empty())
    {
        MessageBoxW(nullptr, L"Enter a course name.", L"CGPA Calculator", MB_ICONINFORMATION);
        SetFocus(g_courseInput);
        return;
    }

    if (!tryReadCredits(credits))
    {
        MessageBoxW(nullptr, L"Enter credits as a whole number from 1 to 30.", L"CGPA Calculator", MB_ICONINFORMATION);
        SetFocus(g_creditsInput);
        return;
    }

    const std::wstring grade = selectedGrade();
    g_courses.push_back({courseName, credits, grade, cgpa::gradePointFor(grade)});

    SetWindowTextW(g_courseInput, L"");
    SetWindowTextW(g_creditsInput, L"");
    SetFocus(g_courseInput);
    refreshCourseList();
}

void clearCourses()
{
    g_courses.clear();
    SetWindowTextW(g_courseInput, L"");
    SetWindowTextW(g_creditsInput, L"");
    refreshCourseList();
    SetFocus(g_courseInput);
}

void layoutControls(HWND window)
{
    RECT client{};
    GetClientRect(window, &client);

    constexpr int margin = 16;
    constexpr int labelHeight = 20;
    constexpr int controlHeight = 28;
    constexpr int gap = 8;
    constexpr int buttonWidth = 96;
    constexpr int summaryWidth = 230;

    const int width = client.right - client.left;
    const int height = client.bottom - client.top;
    const int contentWidth = std::max(520, width - (margin * 2));
    const int listWidth = std::max(280, contentWidth - summaryWidth - gap);
    const int top = margin + labelHeight;
    const int rowTwoTop = top + controlHeight + 32;
    const int listTop = rowTwoTop + controlHeight + 16;
    const int listHeight = std::max(120, height - listTop - margin);

    MoveWindow(g_studentInput, margin, top, listWidth, controlHeight, TRUE);

    const int gradeWidth = 90;
    const int creditsWidth = 90;
    const int actionsWidth = (buttonWidth * 2) + gap;
    const int courseWidth = std::max(140, listWidth - creditsWidth - gradeWidth - actionsWidth - (gap * 4));

    MoveWindow(g_courseInput, margin, rowTwoTop, courseWidth, controlHeight, TRUE);
    MoveWindow(g_creditsInput, margin + courseWidth + gap, rowTwoTop, creditsWidth, controlHeight, TRUE);
    MoveWindow(g_gradeCombo, margin + courseWidth + creditsWidth + (gap * 2), rowTwoTop, gradeWidth, 160, TRUE);
    MoveWindow(g_addButton, margin + courseWidth + creditsWidth + gradeWidth + (gap * 3), rowTwoTop, buttonWidth, controlHeight, TRUE);
    MoveWindow(g_clearButton, margin + courseWidth + creditsWidth + gradeWidth + buttonWidth + (gap * 4), rowTwoTop, buttonWidth, controlHeight, TRUE);
    MoveWindow(g_courseList, margin, listTop, listWidth, listHeight, TRUE);
    MoveWindow(g_summaryText, margin + listWidth + gap, top, summaryWidth, height - top - margin, TRUE);

    ListView_SetColumnWidth(g_courseList, 0, std::max(140, listWidth - 260));
    ListView_SetColumnWidth(g_courseList, 1, 70);
    ListView_SetColumnWidth(g_courseList, 2, 70);
    ListView_SetColumnWidth(g_courseList, 3, 110);
}

LRESULT CALLBACK courseInputProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_KEYDOWN && wParam == VK_RETURN)
    {
        addCourseFromInput();
        return 0;
    }

    return CallWindowProcW(g_originalCourseInputProc, window, message, wParam, lParam);
}

void addListColumn(int index, const wchar_t* title, int width)
{
    LVCOLUMNW column{};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    column.pszText = const_cast<LPWSTR>(title);
    column.cx = width;
    column.iSubItem = index;
    ListView_InsertColumn(g_courseList, index, &column);
}

void createControls(HWND window)
{
    addStaticLabel(window, L"Student name", 16, 12, 180, 20);
    addStaticLabel(window, L"Course", 16, 72, 180, 20);

    g_studentInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(StudentInputId)), GetModuleHandleW(nullptr), nullptr);
    g_courseInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(CourseInputId)), GetModuleHandleW(nullptr), nullptr);
    g_creditsInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(CreditsInputId)), GetModuleHandleW(nullptr), nullptr);
    g_gradeCombo = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(GradeComboId)), GetModuleHandleW(nullptr), nullptr);
    g_addButton = CreateWindowExW(0, L"BUTTON", L"Add Course", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(AddCourseButtonId)), GetModuleHandleW(nullptr), nullptr);
    g_clearButton = CreateWindowExW(0, L"BUTTON", L"Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ClearButtonId)), GetModuleHandleW(nullptr), nullptr);
    g_courseList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(CourseListId)), GetModuleHandleW(nullptr), nullptr);
    g_summaryText = CreateWindowExW(WS_EX_CLIENTEDGE, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(SummaryTextId)), GetModuleHandleW(nullptr), nullptr);

    const wchar_t* grades[] = {L"A", L"A-", L"B+", L"B", L"B-", L"C+", L"C", L"C-", L"D+", L"D", L"F"};
    for (const wchar_t* grade : grades)
    {
        SendMessageW(g_gradeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(grade));
    }
    SendMessageW(g_gradeCombo, CB_SETCURSEL, 0, 0);

    ListView_SetExtendedListViewStyle(g_courseList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    addListColumn(0, L"Course", 240);
    addListColumn(1, L"Credits", 70);
    addListColumn(2, L"Grade", 70);
    addListColumn(3, L"Grade Points", 110);

    g_originalCourseInputProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(g_courseInput, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(courseInputProc)));

    setFonts();
    updateSummary();
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
        if (LOWORD(wParam) == AddCourseButtonId && HIWORD(wParam) == BN_CLICKED)
        {
            addCourseFromInput();
            return 0;
        }
        if (LOWORD(wParam) == ClearButtonId && HIWORD(wParam) == BN_CLICKED)
        {
            clearCourses();
            return 0;
        }
        if (LOWORD(wParam) == StudentInputId && HIWORD(wParam) == EN_CHANGE)
        {
            updateSummary();
            return 0;
        }
        break;

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
    commonControls.dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&commonControls);

    const wchar_t className[] = L"CGPACalculatorWindow";

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&windowClass))
    {
        MessageBoxW(nullptr, L"Could not register the application window.", L"CGPA Calculator", MB_ICONERROR);
        return 1;
    }

    HWND window = CreateWindowExW(
        0,
        className,
        L"CGPA Calculator",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        840,
        560,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!window)
    {
        MessageBoxW(nullptr, L"Could not create the application window.", L"CGPA Calculator", MB_ICONERROR);
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

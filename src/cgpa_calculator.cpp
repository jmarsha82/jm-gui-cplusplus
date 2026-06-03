#include <raylib.h>

#include "cgpa/cgpa_core.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace
{
constexpr int WindowWidth = 1120;
constexpr int WindowHeight = 720;

constexpr Color Ink{15, 21, 34, 255};
constexpr Color Panel{24, 32, 48, 255};
constexpr Color PanelSoft{31, 41, 59, 255};
constexpr Color Mint{77, 221, 166, 255};
constexpr Color Citrus{255, 203, 92, 255};
constexpr Color Coral{255, 111, 105, 255};
constexpr Color Text{236, 244, 241, 255};
constexpr Color Muted{157, 176, 185, 255};
constexpr Color Stroke{255, 255, 255, 34};
constexpr Color Field{11, 16, 27, 255};

const std::vector<std::string> Grades{"A", "A-", "B+", "B", "B-", "C+", "C", "C-", "D+", "D", "F"};

struct TextBox
{
    Rectangle bounds{};
    std::string text;
    bool focused = false;
    bool numeric = false;
};

std::string trim(const std::string& value)
{
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();

    if (first >= last)
    {
        return {};
    }

    return std::string(first, last);
}

std::wstring widen(const std::string& value)
{
    return std::wstring(value.begin(), value.end());
}

std::string narrow(const std::wstring& value)
{
    std::string converted;
    converted.reserve(value.size());
    for (wchar_t ch : value)
    {
        converted.push_back(static_cast<char>(ch));
    }
    return converted;
}

std::string formatDouble(double value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}

bool button(Rectangle bounds, const char* label, Color fill, Color hoverFill)
{
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    const bool pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    DrawRectangleRounded(bounds, 0.18f, 12, hovered ? hoverFill : fill);
    DrawRectangleRoundedLinesEx(bounds, 0.18f, 12, 1.0f, Stroke);

    const int fontSize = 18;
    DrawText(
        label,
        static_cast<int>(bounds.x + ((bounds.width - MeasureText(label, fontSize)) / 2.0f)),
        static_cast<int>(bounds.y + ((bounds.height - fontSize) / 2.0f)),
        fontSize,
        Text);

    return pressed;
}

void drawTextBox(TextBox& box, const char* label, const char* placeholder)
{
    DrawText(label, static_cast<int>(box.bounds.x), static_cast<int>(box.bounds.y - 24), 16, Muted);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        box.focused = CheckCollisionPointRec(GetMousePosition(), box.bounds);
    }

    DrawRectangleRounded(box.bounds, 0.14f, 12, Field);
    DrawRectangleRoundedLinesEx(box.bounds, 0.14f, 12, box.focused ? 2.0f : 1.0f, box.focused ? Mint : Stroke);

    const char* shown = box.text.empty() ? placeholder : box.text.c_str();
    DrawText(shown, static_cast<int>(box.bounds.x + 14), static_cast<int>(box.bounds.y + 14), 19, box.text.empty() ? Muted : Text);

    if (box.focused && (GetTime() - static_cast<int>(GetTime())) < 0.55)
    {
        const int caretX = static_cast<int>(box.bounds.x + 15 + MeasureText(box.text.c_str(), 19));
        DrawLine(caretX, static_cast<int>(box.bounds.y + 13), caretX, static_cast<int>(box.bounds.y + 38), Mint);
    }
}

void updateTextBox(TextBox& box, int maxLength)
{
    if (!box.focused)
    {
        return;
    }

    for (int key = GetCharPressed(); key > 0; key = GetCharPressed())
    {
        const bool allowed = box.numeric ? (key >= '0' && key <= '9') : (key >= 32 && key <= 126);
        if (allowed && static_cast<int>(box.text.size()) < maxLength)
        {
            box.text.push_back(static_cast<char>(key));
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !box.text.empty())
    {
        box.text.pop_back();
    }
}

int parseCredits(const std::string& value)
{
    const std::string cleaned = trim(value);
    if (cleaned.empty())
    {
        return 0;
    }

    const int credits = std::stoi(cleaned);
    return credits > 0 && credits <= 30 ? credits : 0;
}

void drawGradeSelector(int& selectedGrade)
{
    DrawText("Grade", 520, 148, 16, Muted);
    for (int index = 0; index < static_cast<int>(Grades.size()); ++index)
    {
        const Rectangle chip{520.0f + (index % 6) * 55.0f, 174.0f + (index / 6) * 42.0f, 46.0f, 32.0f};
        const bool active = index == selectedGrade;
        const bool hovered = CheckCollisionPointRec(GetMousePosition(), chip);
        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            selectedGrade = index;
        }

        DrawRectangleRounded(chip, 0.24f, 10, active ? Mint : hovered ? PanelSoft : Field);
        DrawRectangleRoundedLinesEx(chip, 0.24f, 10, 1.0f, active ? Mint : Stroke);
        DrawText(Grades[index].c_str(), static_cast<int>(chip.x + 11), static_cast<int>(chip.y + 8), 16, active ? Ink : Text);
    }
}

void drawSummary(const cgpa::Summary& summary, const std::string& student)
{
    const Rectangle card{820, 118, 260, 516};
    DrawRectangleRounded(card, 0.06f, 12, Panel);
    DrawRectangleRoundedLinesEx(card, 0.06f, 12, 1.0f, Stroke);

    DrawText(student.empty() ? "Student" : student.c_str(), 844, 146, 24, Text);
    DrawText("Academic snapshot", 846, 178, 16, Muted);

    const char* labels[] = {"Courses", "Credits", "Grade points", "Semester GPA", "CGPA", "Overall grade"};
    const std::string values[] = {
        std::to_string(summary.courseCount),
        std::to_string(summary.totalCredits),
        formatDouble(summary.totalGradePoints),
        formatDouble(summary.gpa),
        formatDouble(summary.gpa),
        narrow(summary.overallGrade),
    };

    for (int index = 0; index < 6; ++index)
    {
        const int y = 226 + (index * 58);
        DrawText(labels[index], 846, y, 15, Muted);
        DrawText(values[index].c_str(), 846, y + 20, index == 5 ? 30 : 24, index == 5 ? Citrus : Text);
    }
}

void drawCourseTable(const std::vector<cgpa::CourseResult>& courses)
{
    const Rectangle table{40, 320, 760, 314};
    DrawRectangleRounded(table, 0.04f, 10, Panel);
    DrawRectangleRoundedLinesEx(table, 0.04f, 10, 1.0f, Stroke);
    DrawRectangleRounded({56, 338, 728, 42}, 0.08f, 8, PanelSoft);

    DrawText("Course", 76, 351, 16, Mint);
    DrawText("Credits", 426, 351, 16, Mint);
    DrawText("Grade", 526, 351, 16, Mint);
    DrawText("Points", 626, 351, 16, Mint);

    if (courses.empty())
    {
        DrawText("No courses yet", 76, 450, 30, Text);
        DrawText("Add a course above to start calculating.", 78, 490, 18, Muted);
        return;
    }

    for (int index = 0; index < static_cast<int>(courses.size()) && index < 6; ++index)
    {
        const auto& course = courses[index];
        const int y = 398 + (index * 39);
        const Color row = index % 2 == 0 ? Color{255, 255, 255, 8} : Color{255, 255, 255, 14};
        DrawRectangleRounded({56, static_cast<float>(y), 728, 32}, 0.08f, 8, row);
        DrawText(narrow(course.name).c_str(), 76, y + 8, 16, Text);
        DrawText(std::to_string(course.credits).c_str(), 438, y + 8, 16, Text);
        DrawText(narrow(course.grade).c_str(), 536, y + 8, 16, Citrus);
        DrawText(formatDouble(course.gradePoint * course.credits).c_str(), 626, y + 8, 16, Text);
    }

    if (courses.size() > 6)
    {
        DrawText(TextFormat("+%d more courses", static_cast<int>(courses.size() - 6)), 76, 604, 16, Muted);
    }
}
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WindowWidth, WindowHeight, "CGPA Calculator");
    SetTargetFPS(60);

    TextBox student{{40, 118, 450, 52}, {}, true, false};
    TextBox course{{40, 202, 330, 52}, {}, false, false};
    TextBox credits{{390, 202, 90, 52}, {}, false, true};
    std::vector<cgpa::CourseResult> courses;
    int selectedGrade = 0;
    std::string message;

    while (!WindowShouldClose())
    {
        updateTextBox(student, 40);
        updateTextBox(course, 42);
        updateTextBox(credits, 2);

        BeginDrawing();
        ClearBackground(Ink);
        DrawRectangleGradientV(0, 0, WindowWidth, WindowHeight, Ink, {24, 42, 47, 255});

        DrawText("CGPA Calculator", 40, 34, 42, Text);
        DrawText("Track courses, credits, and grades in a calmer workspace.", 42, 86, 18, Muted);

        drawTextBox(student, "Student name", "Student");
        drawTextBox(course, "Course", "Course name");
        drawTextBox(credits, "Credits", "3");
        drawGradeSelector(selectedGrade);

        const bool addPressed = button({40, 274, 132, 46}, "Add Course", Mint, {93, 238, 184, 255});
        const bool clearPressed = button({188, 274, 96, 46}, "Clear", PanelSoft, {42, 55, 79, 255});

        if (addPressed || (course.focused && IsKeyPressed(KEY_ENTER)) || (credits.focused && IsKeyPressed(KEY_ENTER)))
        {
            const std::string courseName = trim(course.text);
            const int courseCredits = parseCredits(credits.text);
            if (courseName.empty())
            {
                message = "Enter a course name.";
            }
            else if (courseCredits == 0)
            {
                message = "Credits must be a whole number from 1 to 30.";
            }
            else
            {
                const std::wstring grade = widen(Grades[selectedGrade]);
                courses.push_back({widen(courseName), courseCredits, grade, cgpa::gradePointFor(grade)});
                course.text.clear();
                credits.text.clear();
                course.focused = true;
                credits.focused = false;
                message.clear();
            }
        }

        if (clearPressed)
        {
            courses.clear();
            course.text.clear();
            credits.text.clear();
            message.clear();
        }

        const cgpa::Summary summary = cgpa::summarize(courses);

        if (!message.empty())
        {
            DrawText(message.c_str(), 316, 288, 16, Coral);
        }

        drawCourseTable(courses);
        drawSummary(summary, trim(student.text));
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

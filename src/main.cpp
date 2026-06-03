#include <raylib.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{
constexpr int WindowWidth = 860;
constexpr int WindowHeight = 620;
constexpr int MaxTaskLength = 72;

constexpr Color Ink{16, 22, 35, 255};
constexpr Color Panel{24, 32, 48, 255};
constexpr Color PanelSoft{31, 41, 59, 255};
constexpr Color Mint{77, 221, 166, 255};
constexpr Color Citrus{255, 203, 92, 255};
constexpr Color Coral{255, 111, 105, 255};
constexpr Color Text{236, 244, 241, 255};
constexpr Color Muted{157, 176, 185, 255};
constexpr Color Stroke{255, 255, 255, 34};
constexpr Color Field{11, 16, 27, 255};

struct TextBox
{
    Rectangle bounds{};
    std::string text;
    bool focused = false;
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

bool button(Rectangle bounds, const char* label, Color fill, Color hoverFill)
{
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    const bool pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    DrawRectangleRounded(bounds, 0.22f, 12, hovered ? hoverFill : fill);
    DrawRectangleRoundedLinesEx(bounds, 0.22f, 12, 1.0f, Stroke);

    const int fontSize = 18;
    const int labelWidth = MeasureText(label, fontSize);
    DrawText(
        label,
        static_cast<int>(bounds.x + ((bounds.width - labelWidth) / 2.0f)),
        static_cast<int>(bounds.y + ((bounds.height - fontSize) / 2.0f)),
        fontSize,
        Text);

    return pressed;
}

void drawTextBox(TextBox& box, const char* placeholder)
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        box.focused = CheckCollisionPointRec(GetMousePosition(), box.bounds);
    }

    DrawRectangleRounded(box.bounds, 0.16f, 12, Field);
    DrawRectangleRoundedLinesEx(box.bounds, 0.16f, 12, box.focused ? 2.0f : 1.0f, box.focused ? Mint : Stroke);

    const char* shown = box.text.empty() ? placeholder : box.text.c_str();
    DrawText(shown, static_cast<int>(box.bounds.x + 16), static_cast<int>(box.bounds.y + 15), 20, box.text.empty() ? Muted : Text);

    if (box.focused && (GetTime() - static_cast<int>(GetTime())) < 0.55)
    {
        const int caretX = static_cast<int>(box.bounds.x + 17 + MeasureText(box.text.c_str(), 20));
        DrawLine(caretX, static_cast<int>(box.bounds.y + 14), caretX, static_cast<int>(box.bounds.y + 39), Mint);
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
        if (key >= 32 && key <= 126 && static_cast<int>(box.text.size()) < maxLength)
        {
            box.text.push_back(static_cast<char>(key));
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !box.text.empty())
    {
        box.text.pop_back();
    }
}

void drawHeader()
{
    DrawText("Task List", 40, 34, 42, Text);
    DrawText("Capture the work, sort it cleanly, keep the day moving.", 42, 86, 18, Muted);
}

void drawTaskRows(const std::vector<std::string>& tasks)
{
    const Rectangle list{40, 186, 780, 372};
    DrawRectangleRounded(list, 0.04f, 10, Panel);
    DrawRectangleRoundedLinesEx(list, 0.04f, 10, 1.0f, Stroke);

    DrawRectangleRounded({56, 204, 748, 44}, 0.08f, 8, PanelSoft);
    DrawText("Name", 76, 217, 18, Mint);
    DrawText("Click Sort to toggle alphabetical order", 495, 217, 16, Muted);

    if (tasks.empty())
    {
        DrawText("No tasks yet", 76, 312, 30, Text);
        DrawText("Type a task above and press Enter or Add.", 78, 352, 18, Muted);
        return;
    }

    for (int index = 0; index < static_cast<int>(tasks.size()) && index < 8; ++index)
    {
        const int y = 266 + (index * 38);
        const Color row = index % 2 == 0 ? Color{255, 255, 255, 8} : Color{255, 255, 255, 14};
        DrawRectangleRounded({56, static_cast<float>(y), 748, 31}, 0.08f, 8, row);
        DrawText(TextFormat("%02d", index + 1), 74, y + 7, 16, Citrus);
        DrawText(tasks[index].c_str(), 116, y + 6, 18, Text);
    }

    if (tasks.size() > 8)
    {
        DrawText(TextFormat("+%d more", static_cast<int>(tasks.size() - 8)), 76, 532, 16, Muted);
    }
}
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WindowWidth, WindowHeight, "Task List App");
    SetTargetFPS(60);

    TextBox input{{40, 120, 560, 56}, {}, true};
    std::vector<std::string> tasks;
    bool sortAscending = true;

    while (!WindowShouldClose())
    {
        updateTextBox(input, MaxTaskLength);

        BeginDrawing();
        ClearBackground(Ink);
        DrawRectangleGradientV(0, 0, WindowWidth, WindowHeight, Ink, {18, 47, 48, 255});
        drawHeader();
        drawTextBox(input, "Add a task...");

        const bool addPressed = button({616, 120, 92, 56}, "Add", Mint, {93, 238, 184, 255});
        const bool sortPressed = button({720, 120, 100, 56}, sortAscending ? "Sort A-Z" : "Sort Z-A", PanelSoft, {42, 55, 79, 255});

        if ((addPressed || (input.focused && IsKeyPressed(KEY_ENTER))) && !trim(input.text).empty())
        {
            tasks.push_back(trim(input.text));
            input.text.clear();
            input.focused = true;
        }

        if (sortPressed)
        {
            std::sort(tasks.begin(), tasks.end(), [](const std::string& left, const std::string& right) {
                return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end(), [](char a, char b) {
                    return std::tolower(static_cast<unsigned char>(a)) < std::tolower(static_cast<unsigned char>(b));
                });
            });

            if (!sortAscending)
            {
                std::reverse(tasks.begin(), tasks.end());
            }
            sortAscending = !sortAscending;
        }

        drawTaskRows(tasks);
        DrawText(TextFormat("%d tasks", static_cast<int>(tasks.size())), 42, 576, 18, Muted);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

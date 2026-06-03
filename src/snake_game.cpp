#include <raylib.h>

#include <algorithm>
#include <string>
#include <vector>

namespace
{
constexpr int CellSize = 24;
constexpr int BoardColumns = 24;
constexpr int BoardRows = 18;
constexpr int BoardWidth = BoardColumns * CellSize;
constexpr int BoardHeight = BoardRows * CellSize;
constexpr int PanelWidth = 230;
constexpr int WindowWidth = BoardWidth + PanelWidth;
constexpr float TickSeconds = 0.11f;

constexpr Color Ink{18, 24, 38, 255};
constexpr Color DeepInk{9, 14, 24, 255};
constexpr Color FieldInk{20, 44, 42, 255};
constexpr Color Mint{80, 230, 166, 255};
constexpr Color Citrus{255, 205, 79, 255};
constexpr Color Coral{255, 104, 96, 255};
constexpr Color TextColor{235, 246, 241, 255};
constexpr Color MutedText{154, 181, 172, 255};
constexpr Color GridLine{255, 255, 255, 22};
constexpr Color PanelFill{255, 255, 255, 14};

enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

struct Cell
{
    int x = 0;
    int y = 0;
};

Cell deltaFor(Direction direction)
{
    switch (direction)
    {
    case Direction::Up:
        return {0, -1};
    case Direction::Down:
        return {0, 1};
    case Direction::Left:
        return {-1, 0};
    case Direction::Right:
        return {1, 0};
    }

    return {1, 0};
}

bool operator==(const Cell& first, const Cell& second)
{
    return first.x == second.x && first.y == second.y;
}

bool areOpposite(Direction first, Direction second)
{
    return (first == Direction::Up && second == Direction::Down) ||
        (first == Direction::Down && second == Direction::Up) ||
        (first == Direction::Left && second == Direction::Right) ||
        (first == Direction::Right && second == Direction::Left);
}

Rectangle cellRect(const Cell& cell)
{
    return {
        static_cast<float>(cell.x * CellSize),
        static_cast<float>(cell.y * CellSize),
        static_cast<float>(CellSize),
        static_cast<float>(CellSize),
    };
}

Color fadedMint(int index)
{
    return {
        static_cast<unsigned char>(std::max(38, Mint.r - (index * 5))),
        static_cast<unsigned char>(std::max(130, Mint.g - (index * 4))),
        static_cast<unsigned char>(std::max(112, Mint.b - (index * 2))),
        255,
    };
}

class SnakeGame
{
public:
    SnakeGame()
    {
        restart();
    }

    void update()
    {
        handleInput();

        if (m_paused || m_gameOver)
        {
            return;
        }

        m_elapsed += GetFrameTime();
        while (m_elapsed >= TickSeconds)
        {
            m_elapsed -= TickSeconds;
            step();
        }
    }

    void draw() const
    {
        BeginDrawing();
        ClearBackground(DeepInk);

        drawBackground();
        drawBoard();
        drawFood();
        drawSnake();
        drawPanel();
        drawOverlay();

        EndDrawing();
    }

private:
    void restart()
    {
        m_snake = {
            {BoardColumns / 2, BoardRows / 2},
            {(BoardColumns / 2) - 1, BoardRows / 2},
            {(BoardColumns / 2) - 2, BoardRows / 2},
        };
        m_food = {};
        m_direction = Direction::Right;
        m_nextDirection = Direction::Right;
        m_score = 0;
        m_elapsed = 0.0f;
        m_gameOver = false;
        m_paused = false;
        placeFood();
    }

    void handleInput()
    {
        Direction requested = m_nextDirection;

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            requested = Direction::Up;
        }
        else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            requested = Direction::Down;
        }
        else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        {
            requested = Direction::Left;
        }
        else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        {
            requested = Direction::Right;
        }

        if (!areOpposite(m_direction, requested))
        {
            m_nextDirection = requested;
        }

        if (IsKeyPressed(KEY_SPACE) && !m_gameOver)
        {
            m_paused = !m_paused;
        }

        if (IsKeyPressed(KEY_R))
        {
            restart();
        }
    }

    void step()
    {
        m_direction = m_nextDirection;

        const Cell delta = deltaFor(m_direction);
        const Cell nextHead{m_snake.front().x + delta.x, m_snake.front().y + delta.y};
        const bool eatsFood = nextHead == m_food;

        if (nextHead.x < 0 || nextHead.x >= BoardColumns ||
            nextHead.y < 0 || nextHead.y >= BoardRows ||
            hitsSelf(nextHead, eatsFood))
        {
            m_gameOver = true;
            return;
        }

        m_snake.insert(m_snake.begin(), nextHead);
        if (eatsFood)
        {
            m_score += 10;
            placeFood();
        }
        else
        {
            m_snake.pop_back();
        }
    }

    bool hitsSelf(const Cell& cell, bool keepsTail) const
    {
        const std::size_t checkedLength = keepsTail ? m_snake.size() : m_snake.size() - 1;
        return std::find(m_snake.begin(), m_snake.begin() + checkedLength, cell) != m_snake.begin() + checkedLength;
    }

    void placeFood()
    {
        do
        {
            m_food = {GetRandomValue(0, BoardColumns - 1), GetRandomValue(0, BoardRows - 1)};
        } while (std::find(m_snake.begin(), m_snake.end(), m_food) != m_snake.end());
    }

    void drawBackground() const
    {
        DrawRectangleGradientV(0, 0, WindowWidth, BoardHeight, DeepInk, FieldInk);
        DrawRectangleGradientH(0, 0, WindowWidth, BoardHeight, Fade(Ink, 0.92f), Fade(FieldInk, 0.76f));
    }

    void drawBoard() const
    {
        for (int column = 0; column <= BoardColumns; ++column)
        {
            DrawLine(column * CellSize, 0, column * CellSize, BoardHeight, GridLine);
        }

        for (int row = 0; row <= BoardRows; ++row)
        {
            DrawLine(0, row * CellSize, BoardWidth, row * CellSize, GridLine);
        }

        DrawRectangleRounded({12, 12, BoardWidth - 24.0f, BoardHeight - 24.0f}, 0.08f, 12, {255, 255, 255, 9});
    }

    void drawFood() const
    {
        Rectangle food = cellRect(m_food);
        food.x += 5;
        food.y += 5;
        food.width -= 10;
        food.height -= 10;

        DrawEllipseLines(
            static_cast<int>(food.x + (food.width / 2)),
            static_cast<int>(food.y + (food.height / 2)),
            food.width / 2,
            food.height / 2,
            {255, 255, 255, 180});
        DrawEllipse(
            static_cast<int>(food.x + (food.width / 2)),
            static_cast<int>(food.y + (food.height / 2)),
            food.width / 2,
            food.height / 2,
            Citrus);
        DrawEllipse(
            static_cast<int>(food.x + 11),
            static_cast<int>(food.y + 9),
            4,
            3,
            {255, 255, 255, 130});
    }

    void drawSnake() const
    {
        for (int index = static_cast<int>(m_snake.size()) - 1; index >= 0; --index)
        {
            const bool isHead = index == 0;
            Rectangle segment = cellRect(m_snake[index]);
            segment.x += 3;
            segment.y += 3;
            segment.width -= 6;
            segment.height -= 6;

            DrawRectangleRounded(segment, 0.28f, 8, isHead ? Mint : fadedMint(index));

            if (isHead)
            {
                DrawCircle(
                    static_cast<int>(segment.x + (segment.width / 2)),
                    static_cast<int>(segment.y + (segment.height / 2)),
                    4,
                    DeepInk);
            }
        }
    }

    void drawPanel() const
    {
        DrawRectangle(BoardWidth, 0, PanelWidth, BoardHeight, PanelFill);
        DrawLine(BoardWidth, 0, BoardWidth, BoardHeight, {255, 255, 255, 28});

        DrawText("Snake", BoardWidth + 26, 34, 32, TextColor);
        DrawText("Neon orchard chase", BoardWidth + 26, 86, 16, MutedText);

        drawStat("Score", std::to_string(m_score), 150, Mint);
        drawStat("Length", std::to_string(m_snake.size()), 235, Citrus);

        DrawText("Arrow keys or WASD", BoardWidth + 26, 326, 16, MutedText);
        DrawText("Space to pause", BoardWidth + 26, 350, 16, MutedText);
        DrawText("R to restart", BoardWidth + 26, 374, 16, MutedText);
    }

    void drawStat(const char* label, const std::string& value, int top, Color accent) const
    {
        const Rectangle card{BoardWidth + 26.0f, static_cast<float>(top), PanelWidth - 52.0f, 62.0f};
        DrawRectangleRounded(card, 0.12f, 8, {255, 255, 255, 14});
        DrawRectangleRoundedLinesEx(card, 0.12f, 8, 1.0f, {255, 255, 255, 28});

        DrawText(label, BoardWidth + 42, top + 10, 14, accent);
        DrawText(value.c_str(), BoardWidth + 42, top + 30, 24, TextColor);
    }

    void drawOverlay() const
    {
        if (!m_paused && !m_gameOver)
        {
            return;
        }

        const Rectangle overlay{32, 136, BoardWidth - 64.0f, 160.0f};
        DrawRectangleRounded(overlay, 0.10f, 10, {9, 14, 24, 188});

        const char* title = m_gameOver ? "Game Over" : "Paused";
        const int titleSize = 38;
        const int titleWidth = MeasureText(title, titleSize);
        DrawText(title, (BoardWidth - titleWidth) / 2, 166, titleSize, m_gameOver ? Coral : Citrus);

        const char* prompt = "Press R to restart";
        const int promptWidth = MeasureText(prompt, 18);
        DrawText(prompt, (BoardWidth - promptWidth) / 2, 232, 18, TextColor);
    }

    std::vector<Cell> m_snake;
    Cell m_food;
    Direction m_direction = Direction::Right;
    Direction m_nextDirection = Direction::Right;
    int m_score = 0;
    float m_elapsed = 0.0f;
    bool m_gameOver = false;
    bool m_paused = false;
};
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WindowWidth, BoardHeight, "Snake Raylib Game");
    SetTargetFPS(60);

    SnakeGame game;

    while (!WindowShouldClose())
    {
        game.update();
        game.draw();
    }

    CloseWindow();
    return 0;
}

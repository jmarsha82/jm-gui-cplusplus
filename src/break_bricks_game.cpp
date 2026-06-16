#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{
constexpr int WindowWidth = 1120;
constexpr int WindowHeight = 760;
constexpr int PlayLeft = 56;
constexpr int PlayTop = 96;
constexpr int PlayWidth = 780;
constexpr int PlayHeight = 604;
constexpr float PaddleBaseWidth = 132.0f;
constexpr float PaddleHeight = 18.0f;
constexpr float PaddleY = PlayTop + PlayHeight - 48.0f;
constexpr float BallRadius = 8.0f;
constexpr float BaseBallSpeed = 335.0f;
constexpr int BrickColumns = 10;
constexpr int BrickRows = 6;
constexpr float BrickGap = 8.0f;
constexpr float BrickHeight = 30.0f;

constexpr Color Ink{13, 18, 31, 255};
constexpr Color Panel{24, 31, 49, 255};
constexpr Color PanelSoft{34, 44, 68, 255};
constexpr Color Text{239, 246, 245, 255};
constexpr Color Muted{151, 171, 184, 255};
constexpr Color Stroke{255, 255, 255, 34};
constexpr Color Mint{86, 234, 177, 255};
constexpr Color Cyan{80, 206, 255, 255};
constexpr Color Violet{171, 130, 255, 255};
constexpr Color Citrus{255, 203, 86, 255};
constexpr Color Coral{255, 112, 102, 255};
constexpr Color Rose{255, 104, 166, 255};

enum class BrickKind
{
    Normal,
    SpeedUp,
    SlowDown,
    PaddleLarge,
    PaddleSmall,
    DoublePoints,
    ExtraBall
};

struct Brick
{
    Rectangle bounds{};
    BrickKind kind = BrickKind::Normal;
    bool alive = true;
};

struct Ball
{
    Vector2 position{};
    Vector2 velocity{};
    Color color = Text;
};

Color kindColor(BrickKind kind)
{
    switch (kind)
    {
    case BrickKind::SpeedUp:
        return Coral;
    case BrickKind::SlowDown:
        return Cyan;
    case BrickKind::PaddleLarge:
        return Mint;
    case BrickKind::PaddleSmall:
        return Rose;
    case BrickKind::DoublePoints:
        return Citrus;
    case BrickKind::ExtraBall:
        return Violet;
    case BrickKind::Normal:
        return {94, 112, 150, 255};
    }

    return Text;
}

const char* kindSymbol(BrickKind kind)
{
    switch (kind)
    {
    case BrickKind::SpeedUp:
        return ">>";
    case BrickKind::SlowDown:
        return "<<";
    case BrickKind::PaddleLarge:
        return "W+";
    case BrickKind::PaddleSmall:
        return "W-";
    case BrickKind::DoublePoints:
        return "2X";
    case BrickKind::ExtraBall:
        return "+B";
    case BrickKind::Normal:
        return "";
    }

    return "";
}

const char* kindName(BrickKind kind)
{
    switch (kind)
    {
    case BrickKind::SpeedUp:
        return "speed up ball";
    case BrickKind::SlowDown:
        return "slow down ball";
    case BrickKind::PaddleLarge:
        return "larger paddle";
    case BrickKind::PaddleSmall:
        return "smaller paddle";
    case BrickKind::DoublePoints:
        return "double points";
    case BrickKind::ExtraBall:
        return "extra ball";
    case BrickKind::Normal:
        return "points";
    }

    return "";
}

Vector2 normalized(Vector2 value)
{
    const float length = Vector2Length(value);
    if (length <= 0.01f)
    {
        return {0.0f, -1.0f};
    }
    return Vector2Scale(value, 1.0f / length);
}

void setBallSpeed(Ball& ball, float speed)
{
    ball.velocity = Vector2Scale(normalized(ball.velocity), speed);
}

bool button(Rectangle bounds, const char* label, Color fill, Color hover)
{
    const bool hot = CheckCollisionPointRec(GetMousePosition(), bounds);
    const bool clicked = hot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    DrawRectangleRounded(bounds, 0.18f, 12, hot ? hover : fill);
    DrawRectangleRoundedLinesEx(bounds, 0.18f, 12, 1.0f, Stroke);

    const int fontSize = 18;
    DrawText(label,
        static_cast<int>(bounds.x + ((bounds.width - MeasureText(label, fontSize)) / 2.0f)),
        static_cast<int>(bounds.y + ((bounds.height - fontSize) / 2.0f)),
        fontSize,
        Text);

    return clicked;
}

class BreakBricksGame
{
public:
    BreakBricksGame()
    {
        reset();
    }

    void update()
    {
        if (m_waiting || m_gameOver || m_won)
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                reset();
            }
            return;
        }

        const float dt = GetFrameTime();
        updateTimers(dt);
        updatePaddle(dt);
        updateBalls(dt);
    }

    void draw()
    {
        BeginDrawing();
        ClearBackground(Ink);
        DrawRectangleGradientV(0, 0, WindowWidth, WindowHeight, {12, 17, 30, 255}, {17, 48, 58, 255});

        drawHeader();
        drawArena();
        drawBricks();
        drawPaddle();
        drawBalls();
        drawSidePanel();
        drawOverlay();

        EndDrawing();
    }

private:
    void reset()
    {
        m_score = 0;
        m_lives = 3;
        m_ballSpeed = BaseBallSpeed;
        m_paddleWidth = PaddleBaseWidth;
        m_multiplierTimer = 0.0f;
        m_lastEffect = "Break colorful bricks";
        m_waiting = false;
        m_gameOver = false;
        m_won = false;
        m_paddle = {PlayLeft + (PlayWidth - m_paddleWidth) / 2.0f, PaddleY, m_paddleWidth, PaddleHeight};
        m_lastMouseX = static_cast<float>(GetMouseX());

        buildBricks();
        m_balls.clear();
        spawnBall({m_paddle.x + (m_paddle.width / 2.0f), m_paddle.y - 24.0f}, {-0.35f, -1.0f}, Mint);
    }

    void buildBricks()
    {
        m_bricks.clear();
        const float brickWidth = (PlayWidth - (BrickGap * (BrickColumns - 1))) / BrickColumns;
        const float startY = PlayTop + 56.0f;

        for (int row = 0; row < BrickRows; ++row)
        {
            for (int column = 0; column < BrickColumns; ++column)
            {
                BrickKind kind = BrickKind::Normal;
                const int pattern = (row * 7 + column * 3) % 17;
                if (pattern == 0)
                {
                    kind = BrickKind::ExtraBall;
                }
                else if (pattern == 2 || pattern == 10)
                {
                    kind = BrickKind::DoublePoints;
                }
                else if (pattern == 4 || pattern == 12)
                {
                    kind = BrickKind::PaddleLarge;
                }
                else if (pattern == 5)
                {
                    kind = BrickKind::PaddleSmall;
                }
                else if (pattern == 7 || pattern == 14)
                {
                    kind = BrickKind::SpeedUp;
                }
                else if (pattern == 9)
                {
                    kind = BrickKind::SlowDown;
                }

                m_bricks.push_back({
                    {
                        PlayLeft + column * (brickWidth + BrickGap),
                        startY + row * (BrickHeight + BrickGap),
                        brickWidth,
                        BrickHeight,
                    },
                    kind,
                    true,
                });
            }
        }
    }

    void spawnBall(Vector2 position, Vector2 direction, Color color)
    {
        m_balls.push_back({position, Vector2Scale(normalized(direction), m_ballSpeed), color});
    }

    void updateTimers(float dt)
    {
        if (m_multiplierTimer > 0.0f)
        {
            m_multiplierTimer = std::max(0.0f, m_multiplierTimer - dt);
        }
    }

    void updatePaddle(float dt)
    {
        m_paddle.width = m_paddleWidth;

        const float keyboardMove = (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) ? 1.0f : 0.0f) -
            (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) ? 1.0f : 0.0f);
        const float mouseX = static_cast<float>(GetMouseX());
        const bool mouseMoved = std::fabs(mouseX - m_lastMouseX) > 0.5f;

        if (std::fabs(keyboardMove) > 0.0f)
        {
            m_paddle.x += keyboardMove * 600.0f * dt;
        }
        else if (mouseMoved)
        {
            m_paddle.x = mouseX - (m_paddle.width / 2.0f);
        }

        m_paddle.x = std::clamp(m_paddle.x, static_cast<float>(PlayLeft), static_cast<float>(PlayLeft + PlayWidth) - m_paddle.width);
        m_lastMouseX = mouseX;
    }

    void updateBalls(float dt)
    {
        for (Ball& ball : m_balls)
        {
            ball.position = Vector2Add(ball.position, Vector2Scale(ball.velocity, dt));
            handleWallCollision(ball);
            handlePaddleCollision(ball);
            handleBrickCollision(ball);
        }

        m_balls.erase(std::remove_if(m_balls.begin(), m_balls.end(), [](const Ball& ball) {
            return ball.position.y - BallRadius > PlayTop + PlayHeight + 30;
        }), m_balls.end());

        if (m_balls.empty())
        {
            --m_lives;
            if (m_lives <= 0)
            {
                m_gameOver = true;
                return;
            }

            m_paddleWidth = PaddleBaseWidth;
            m_ballSpeed = BaseBallSpeed;
            m_paddle.width = m_paddleWidth;
            spawnBall({m_paddle.x + (m_paddle.width / 2.0f), m_paddle.y - 24.0f}, {0.35f, -1.0f}, Mint);
        }

        m_won = std::none_of(m_bricks.begin(), m_bricks.end(), [](const Brick& brick) {
            return brick.alive;
        });
    }

    void handleWallCollision(Ball& ball)
    {
        if (ball.position.x - BallRadius <= PlayLeft)
        {
            ball.position.x = PlayLeft + BallRadius;
            ball.velocity.x = std::fabs(ball.velocity.x);
        }
        if (ball.position.x + BallRadius >= PlayLeft + PlayWidth)
        {
            ball.position.x = PlayLeft + PlayWidth - BallRadius;
            ball.velocity.x = -std::fabs(ball.velocity.x);
        }
        if (ball.position.y - BallRadius <= PlayTop)
        {
            ball.position.y = PlayTop + BallRadius;
            ball.velocity.y = std::fabs(ball.velocity.y);
        }
    }

    void handlePaddleCollision(Ball& ball)
    {
        if (ball.velocity.y <= 0.0f || !CheckCollisionCircleRec(ball.position, BallRadius, m_paddle))
        {
            return;
        }

        const float hit = ((ball.position.x - m_paddle.x) / m_paddle.width) - 0.5f;
        ball.position.y = m_paddle.y - BallRadius - 0.5f;
        ball.velocity = Vector2Scale(normalized({hit * 1.6f, -1.0f}), m_ballSpeed);
    }

    void handleBrickCollision(Ball& ball)
    {
        for (Brick& brick : m_bricks)
        {
            if (!brick.alive || !CheckCollisionCircleRec(ball.position, BallRadius, brick.bounds))
            {
                continue;
            }

            brick.alive = false;
            ball.velocity.y *= -1.0f;
            applyBrickEffect(brick.kind, ball);
            return;
        }
    }

    void applyBrickEffect(BrickKind kind, const Ball& sourceBall)
    {
        const int points = m_multiplierTimer > 0.0f ? 20 : 10;
        m_score += points;
        m_lastEffect = kindName(kind);

        switch (kind)
        {
        case BrickKind::SpeedUp:
            m_ballSpeed = std::min(560.0f, m_ballSpeed + 45.0f);
            for (Ball& ball : m_balls)
            {
                setBallSpeed(ball, m_ballSpeed);
            }
            break;
        case BrickKind::SlowDown:
            m_ballSpeed = std::max(230.0f, m_ballSpeed - 45.0f);
            for (Ball& ball : m_balls)
            {
                setBallSpeed(ball, m_ballSpeed);
            }
            break;
        case BrickKind::PaddleLarge:
            m_paddleWidth = std::min(210.0f, m_paddleWidth + 24.0f);
            break;
        case BrickKind::PaddleSmall:
            m_paddleWidth = std::max(84.0f, m_paddleWidth - 24.0f);
            break;
        case BrickKind::DoublePoints:
            m_multiplierTimer = 9.0f;
            break;
        case BrickKind::ExtraBall:
            if (m_balls.size() < 5)
            {
                spawnBall(sourceBall.position, {-sourceBall.velocity.x, -std::fabs(sourceBall.velocity.y)}, Violet);
            }
            break;
        case BrickKind::Normal:
            break;
        }
    }

    void drawHeader() const
    {
        DrawText("Break Bricks", 56, 30, 42, Text);
        DrawText("One player. Color-coded power bricks. Keep every ball alive.", 58, 74, 18, Muted);
    }

    void drawArena() const
    {
        const Rectangle arena{PlayLeft, PlayTop, PlayWidth, PlayHeight};
        DrawRectangleRounded(arena, 0.035f, 12, {9, 13, 25, 220});
        DrawRectangleRoundedLinesEx(arena, 0.035f, 12, 1.0f, Stroke);

        for (int i = 0; i < 8; ++i)
        {
            DrawCircleGradient(
                {static_cast<float>(PlayLeft + 92 + (i * 94)), static_cast<float>(PlayTop + 440 - ((i % 3) * 70))},
                58,
                Fade(kindColor(static_cast<BrickKind>((i % 6) + 1)), 0.12f),
                Fade(Ink, 0.0f));
        }
    }

    void drawBricks() const
    {
        for (const Brick& brick : m_bricks)
        {
            if (!brick.alive)
            {
                continue;
            }

            const Color color = kindColor(brick.kind);
            DrawRectangleRounded(brick.bounds, 0.22f, 10, color);
            DrawRectangleRounded({brick.bounds.x + 5, brick.bounds.y + 4, brick.bounds.width - 10, 8}, 0.4f, 6, {255, 255, 255, 72});
            DrawRectangleRoundedLinesEx(brick.bounds, 0.22f, 10, 1.0f, {255, 255, 255, 54});

            const char* symbol = kindSymbol(brick.kind);
            if (symbol[0] != '\0')
            {
                DrawText(symbol,
                    static_cast<int>(brick.bounds.x + (brick.bounds.width - MeasureText(symbol, 15)) / 2.0f),
                    static_cast<int>(brick.bounds.y + 9),
                    15,
                    Ink);
            }
        }
    }

    void drawPaddle() const
    {
        DrawRectangleRounded({m_paddle.x - 6, m_paddle.y + 6, m_paddle.width + 12, 10}, 0.8f, 12, {0, 0, 0, 58});
        DrawRectangleRounded(m_paddle, 0.6f, 14, Mint);
        DrawRectangleRounded({m_paddle.x + 10, m_paddle.y + 3, m_paddle.width - 20, 5}, 0.8f, 10, {255, 255, 255, 90});
    }

    void drawBalls() const
    {
        for (const Ball& ball : m_balls)
        {
            DrawCircleV(ball.position, BallRadius + 4.0f, Fade(ball.color, 0.18f));
            DrawCircleV(ball.position, BallRadius, ball.color);
            DrawCircleV({ball.position.x - 3.0f, ball.position.y - 3.0f}, 2.5f, {255, 255, 255, 160});
        }
    }

    void drawSidePanel() const
    {
        const Rectangle panel{872, 96, 196, 604};
        DrawRectangleRounded(panel, 0.07f, 14, Panel);
        DrawRectangleRoundedLinesEx(panel, 0.07f, 14, 1.0f, Stroke);

        DrawText("Score", 896, 128, 16, Muted);
        DrawText(std::to_string(m_score).c_str(), 896, 150, 34, Text);

        DrawText("Lives", 896, 216, 16, Muted);
        DrawText(std::to_string(m_lives).c_str(), 896, 238, 30, Citrus);

        DrawText("Balls", 982, 216, 16, Muted);
        DrawText(std::to_string(m_balls.size()).c_str(), 982, 238, 30, Violet);

        DrawText("Speed", 896, 304, 16, Muted);
        DrawText(TextFormat("%.0f", m_ballSpeed), 896, 326, 26, Cyan);

        DrawText("Last effect", 896, 388, 16, Muted);
        DrawText(m_lastEffect.c_str(), 896, 412, 17, Text);

        DrawText("Attributes", 896, 476, 16, Muted);
        drawLegendRow(896, 506, BrickKind::SpeedUp);
        drawLegendRow(896, 532, BrickKind::SlowDown);
        drawLegendRow(896, 558, BrickKind::PaddleLarge);
        drawLegendRow(896, 584, BrickKind::PaddleSmall);
        drawLegendRow(896, 610, BrickKind::DoublePoints);
        drawLegendRow(896, 636, BrickKind::ExtraBall);

        if (m_multiplierTimer > 0.0f)
        {
            DrawText(TextFormat("2X %.1fs", m_multiplierTimer), 896, 666, 18, Citrus);
        }
    }

    void drawLegendRow(int x, int y, BrickKind kind) const
    {
        DrawRectangleRounded({static_cast<float>(x), static_cast<float>(y), 26, 16}, 0.25f, 8, kindColor(kind));
        DrawText(kindSymbol(kind), x + 4, y + 2, 10, Ink);
        DrawText(kindName(kind), x + 36, y - 1, 13, Text);
    }

    void drawOverlay()
    {
        if (!m_gameOver && !m_won)
        {
            return;
        }

        DrawRectangle(PlayLeft, PlayTop, PlayWidth, PlayHeight, {8, 12, 22, 185});
        const char* title = m_won ? "Board Cleared" : "Game Over";
        const Color accent = m_won ? Mint : Coral;
        DrawText(title, PlayLeft + 232, PlayTop + 220, 44, accent);
        DrawText("Press Space or click Restart", PlayLeft + 246, PlayTop + 278, 22, Text);
        if (button({PlayLeft + 310.0f, PlayTop + 326.0f, 160.0f, 46.0f}, "Restart", PanelSoft, {45, 59, 88, 255}))
        {
            reset();
        }
    }

    std::vector<Brick> m_bricks;
    std::vector<Ball> m_balls;
    Rectangle m_paddle{};
    float m_paddleWidth = PaddleBaseWidth;
    float m_ballSpeed = BaseBallSpeed;
    float m_multiplierTimer = 0.0f;
    int m_score = 0;
    int m_lives = 3;
    float m_lastMouseX = 0.0f;
    std::string m_lastEffect;
    bool m_waiting = false;
    bool m_gameOver = false;
    bool m_won = false;
};
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(WindowWidth, WindowHeight, "Break Bricks Game");
    SetTargetFPS(60);

    BreakBricksGame game;

    while (!WindowShouldClose())
    {
        game.update();
        game.draw();
    }

    CloseWindow();
    return 0;
}

#include <deque>
#include <algorithm>
#include <raylib.h>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <queue>

enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Vector2Int
{
    int x, y;
    bool operator==(const Vector2Int &other) const
    {
        return x == other.x && y == other.y;
    }
};

struct Game
{
    int width;
    int height;
    std::deque<Vector2Int> snake;
    Vector2Int apple;
    Direction direction;
    std::queue<Direction> directionQueue;

    Game(int w, int h, Direction dir, const Vector2Int &applePos, const std::deque<Vector2Int> &initialSnake)
        : width(w), height(h), direction(dir), apple(applePos), snake(initialSnake) {}
};

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 450;
constexpr int BORDER_THICKNESS = 2;
constexpr int FPS = 60;
constexpr float MOVE_INTERVAL = 0.1f;
const Color SNAKE_HEAD_COLOR{71, 130, 255, 255};
const Color SNAKE_BODY_COLOR{26, 80, 196, 255};
const Color BORDER_COLOR{0, 0, 0, 255};
const Color BORDER_BG{160, 255, 112, 255};
const Vector2Int INITIAL_HEAD{10, 10};

int GetCellSize(int gameWidth, int gameHeight, int screenWidth, int screenHeight)
{
    float cellWidth = (float)(screenWidth - BORDER_THICKNESS * 2) / gameWidth;
    float cellHeight = (float)(screenHeight - BORDER_THICKNESS * 2) / gameHeight;
    return static_cast<int>(std::min(cellWidth, cellHeight));
}

Direction GetNewDirection(Direction current)
{
    if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && current != Direction::RIGHT)
        return Direction::LEFT;
    if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && current != Direction::LEFT)
        return Direction::RIGHT;
    if ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && current != Direction::DOWN)
        return Direction::UP;
    if ((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) && current != Direction::UP)
        return Direction::DOWN;
    return current;
}

Vector2Int OffsetFromDirection(Direction dir)
{
    switch (dir)
    {
    case Direction::UP:
        return {0, -1};
    case Direction::DOWN:
        return {0, 1};
    case Direction::LEFT:
        return {-1, 0};
    case Direction::RIGHT:
        return {1, 0};
    }
    return {0, 0};
}

bool IsGameOver(const Game &game, const Vector2Int &newHead)
{
    if (newHead.x < 0 || newHead.x >= game.width || newHead.y < 0 || newHead.y >= game.height)
        return true;
    return std::find(game.snake.begin(), game.snake.end(), newHead) != game.snake.end();
}

Vector2Int GetNewApplePosition(const Game &game)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distX(0, game.width - 1);
    std::uniform_int_distribution<> distY(0, game.height - 1);

    Vector2Int pos;
    do
    {
        pos = {distX(gen), distY(gen)};
    } while (std::find(game.snake.begin(), game.snake.end(), pos) != game.snake.end());
    return pos;
}

void ResetGame(Game &game)
{
    int centerX = game.width / 2;
    int centerY = game.height / 2;

    Vector2Int head = {centerX, centerY};
    Vector2Int second, third;

    switch (game.direction)
    {
    case Direction::UP:
        second = {centerX, centerY + 1};
        third = {centerX, centerY + 2};
        break;
    case Direction::DOWN:
        second = {centerX, centerY - 1};
        third = {centerX, centerY - 2};
        break;
    case Direction::LEFT:
        second = {centerX + 1, centerY};
        third = {centerX + 2, centerY};
        break;
    case Direction::RIGHT:
    default:
        second = {centerX - 1, centerY};
        third = {centerX - 2, centerY};
        break;
    }

    game.snake = {head, second, third};
    game.apple = GetNewApplePosition(game);

    if (game.direction != Direction::UP && game.direction != Direction::DOWN &&
        game.direction != Direction::LEFT && game.direction != Direction::RIGHT)
    {
        game.direction = Direction::RIGHT;
    }
}

void QueueDirection(Game &game, Direction newDirection)
{
    if (game.directionQueue.size() >= 3)
        return;

    if (!game.directionQueue.empty() && game.directionQueue.back() == newDirection)
        return;

    if (game.directionQueue.empty())
    {
        if ((newDirection == Direction::LEFT && game.direction != Direction::RIGHT) ||
            (newDirection == Direction::RIGHT && game.direction != Direction::LEFT) ||
            (newDirection == Direction::UP && game.direction != Direction::DOWN) ||
            (newDirection == Direction::DOWN && game.direction != Direction::UP))
        {
            game.directionQueue.push(newDirection);
        }
    }
    else
    {
        Direction lastQueued = game.directionQueue.back();
        if ((newDirection == Direction::LEFT && lastQueued != Direction::RIGHT) ||
            (newDirection == Direction::RIGHT && lastQueued != Direction::LEFT) ||
            (newDirection == Direction::UP && lastQueued != Direction::DOWN) ||
            (newDirection == Direction::DOWN && lastQueued != Direction::UP))
        {
            game.directionQueue.push(newDirection);
        }
    }
}

void HandleInput(Game &game)
{
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
        QueueDirection(game, Direction::LEFT);
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
        QueueDirection(game, Direction::RIGHT);
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
        QueueDirection(game, Direction::UP);
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
        QueueDirection(game, Direction::DOWN);
}

bool Update(Game &game)
{
    if (!game.directionQueue.empty())
    {
        game.direction = game.directionQueue.front();
        game.directionQueue.pop();
    }

    Vector2Int offset = OffsetFromDirection(game.direction);

    Vector2Int newHead{game.snake.front().x + offset.x, game.snake.front().y + offset.y};
    if (IsGameOver(game, newHead))
        return true;

    game.snake.push_front(newHead);

    if (newHead == game.apple)
    {
        game.apple = GetNewApplePosition(game);
    }
    else
    {
        game.snake.pop_back();
    }

    return false;
}

void Render(const Game &game, int screenWidth, int screenHeight)
{
    ClearBackground(RAYWHITE);

    int cellSize = GetCellSize(game.width, game.height, screenWidth, screenHeight);
    float gameWidthPx = static_cast<float>(cellSize * game.width);
    float gameHeightPx = static_cast<float>(cellSize * game.height);

    float offsetX = floorf((screenWidth - gameWidthPx) / 2.0f);
    float offsetY = floorf((screenHeight - gameHeightPx) / 2.0f);

    DrawRectangle(offsetX - BORDER_THICKNESS, offsetY - BORDER_THICKNESS,
                  gameWidthPx + BORDER_THICKNESS * 2, gameHeightPx + BORDER_THICKNESS * 2,
                  BORDER_BG);

    DrawRectangleLinesEx(
        Rectangle{offsetX - BORDER_THICKNESS, offsetY - BORDER_THICKNESS,
                  gameWidthPx + BORDER_THICKNESS * 2, gameHeightPx + BORDER_THICKNESS * 2},
        BORDER_THICKNESS, BORDER_COLOR);

    DrawRectangle(offsetX + game.apple.x * cellSize,
                  offsetY + game.apple.y * cellSize,
                  cellSize, cellSize, RED);

    int snakeLength = game.snake.size();
    for (int i = 0; i < snakeLength; ++i)
    {
        const auto &coord = game.snake[i];
        float gradientFactor = static_cast<float>(i) / snakeLength;
        Color color = {
            static_cast<unsigned char>(SNAKE_HEAD_COLOR.r * (1 - gradientFactor)),
            static_cast<unsigned char>(SNAKE_HEAD_COLOR.g * (1 - gradientFactor)),
            static_cast<unsigned char>(SNAKE_HEAD_COLOR.b * (1 - gradientFactor)),
            255};

        DrawRectangle(offsetX + coord.x * cellSize,
                      offsetY + coord.y * cellSize,
                      cellSize, cellSize, color);
    }
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Snake");
    SetTargetFPS(FPS);

    Game game(25, 25, Direction::RIGHT, {0, 0}, {});
    ResetGame(game);

    float moveTimer = 0.0f;

    auto previousTime = std::chrono::high_resolution_clock::now();

    while (!WindowShouldClose())
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - previousTime;
        previousTime = currentTime;
        float deltaTime = elapsed.count();

        moveTimer += deltaTime;

        HandleInput(game);

        if (moveTimer >= MOVE_INTERVAL)
        {
            moveTimer = 0.0f;

            if (Update(game))
                ResetGame(game);
        }

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        BeginDrawing();
        Render(game, screenWidth, screenHeight);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#include <algorithm>
#include <raylib.h>
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
    int x;
    int y;
    bool operator==(const Vector2Int &other) const = default;
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
        : width(w), height(h), snake(initialSnake), apple(applePos), direction(dir) {}
};

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 450;
constexpr int BORDER_THICKNESS = 2;
constexpr int FPS = 60;
constexpr float MOVE_INTERVAL = 0.1f;
const Color SNAKE_HEAD_COLOR{71, 130, 255, 255};
const Color BORDER_COLOR{0, 0, 0, 255};
const Color BORDER_BG{160, 255, 112, 255};

int GetCellSize(int gameWidth, int gameHeight, int screenWidth, int screenHeight)
{
    int cellWidth = (screenWidth - BORDER_THICKNESS * 2) / gameWidth;
    int cellHeight = (screenHeight - BORDER_THICKNESS * 2) / gameHeight;
    return std::min(cellWidth, cellHeight);
}

Direction GetNewDirection(Direction current)
{
    using enum Direction;
    if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && current != RIGHT)
        return LEFT;
    if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && current != LEFT)
        return RIGHT;
    if ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && current != DOWN)
        return UP;
    if ((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) && current != UP)
        return DOWN;
    return current;
}

Vector2Int OffsetFromDirection(Direction dir)
{
    using enum Direction;
    switch (dir)
    {
    case UP:
        return {0, -1};
    case DOWN:
        return {0, 1};
    case LEFT:
        return {-1, 0};
    case RIGHT:
        return {1, 0};
    }
    return {0, 0};
}

bool IsGameOver(const Game &game, const Vector2Int &newHead)
{
    if (newHead.x < 0 || newHead.x >= game.width || newHead.y < 0 || newHead.y >= game.height)
        return true;
    return std::ranges::find(game.snake, newHead) != game.snake.end();
}

Vector2Int GetNewApplePosition(const Game &game)
{
    Vector2Int pos;
    do
    {
        pos = {GetRandomValue(0, game.width - 1), GetRandomValue(0, game.height - 1)};
    } while (std::ranges::find(game.snake, pos) != game.snake.end());
    return pos;
}

void ResetGame(Game &game)
{
    int centerX = game.width / 2;
    int centerY = game.height / 2;

    Vector2Int head = {centerX, centerY};
    Vector2Int second;
    Vector2Int third;

    using enum Direction;
    switch (game.direction)
    {
    case UP:
        second = {centerX, centerY + 1};
        third = {centerX, centerY + 2};
        break;
    case DOWN:
        second = {centerX, centerY - 1};
        third = {centerX, centerY - 2};
        break;
    case LEFT:
        second = {centerX + 1, centerY};
        third = {centerX + 2, centerY};
        break;
    case RIGHT:
    default:
        second = {centerX - 1, centerY};
        third = {centerX - 2, centerY};
        break;
    }

    game.snake = {head, second, third};
    game.apple = GetNewApplePosition(game);

    if (game.direction != UP && game.direction != DOWN &&
        game.direction != LEFT && game.direction != RIGHT)
    {
        game.direction = RIGHT;
    }
}

void QueueDirection(Game &game, Direction newDirection)
{
    using enum Direction;

    if (game.directionQueue.size() >= 3)
        return;

    if (!game.directionQueue.empty() && game.directionQueue.back() == newDirection)
        return;

    if (game.directionQueue.empty())
    {
        if ((newDirection == LEFT && game.direction != RIGHT) ||
            (newDirection == RIGHT && game.direction != LEFT) ||
            (newDirection == UP && game.direction != DOWN) ||
            (newDirection == DOWN && game.direction != UP))
        {
            game.directionQueue.push(newDirection);
        }
    }
    else
    {
        Direction lastQueued = game.directionQueue.back();
        if ((newDirection == LEFT && lastQueued != RIGHT) ||
            (newDirection == RIGHT && lastQueued != LEFT) ||
            (newDirection == UP && lastQueued != DOWN) ||
            (newDirection == DOWN && lastQueued != UP))
        {
            game.directionQueue.push(newDirection);
        }
    }
}

void HandleInput(Game &game)
{
    using enum Direction;
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
        QueueDirection(game, LEFT);
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
        QueueDirection(game, RIGHT);
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
        QueueDirection(game, UP);
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
        QueueDirection(game, DOWN);
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

    int gameWidthPx = cellSize * game.width;
    int gameHeightPx = cellSize * game.height;

    int offsetX = (screenWidth - gameWidthPx) / 2;
    int offsetY = (screenHeight - gameHeightPx) / 2;

    DrawRectangle(offsetX - BORDER_THICKNESS, offsetY - BORDER_THICKNESS,
                  gameWidthPx + BORDER_THICKNESS * 2, gameHeightPx + BORDER_THICKNESS * 2,
                  BORDER_BG);

    DrawRectangleLinesEx(
        Rectangle{static_cast<float>(offsetX - BORDER_THICKNESS), static_cast<float>(offsetY - BORDER_THICKNESS),
                  static_cast<float>(gameWidthPx + BORDER_THICKNESS * 2), static_cast<float>(gameHeightPx + BORDER_THICKNESS * 2)},
        static_cast<float>(BORDER_THICKNESS), BORDER_COLOR);

    DrawRectangle(offsetX + game.apple.x * cellSize,
                  offsetY + game.apple.y * cellSize,
                  cellSize, cellSize, RED);

    auto snakeLength = static_cast<int>(game.snake.size());
    for (int i = 0; i < snakeLength; ++i)
    {
        const auto &coord = game.snake[i];
        int factor = (snakeLength - i) * 255 / snakeLength;
        Color color = {
            static_cast<unsigned char>(SNAKE_HEAD_COLOR.r * factor / 255),
            static_cast<unsigned char>(SNAKE_HEAD_COLOR.g * factor / 255),
            static_cast<unsigned char>(SNAKE_HEAD_COLOR.b * factor / 255),
            255
        };

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

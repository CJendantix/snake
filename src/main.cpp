/**
 * @file snake.cpp
 * @brief Classic Snake game implementation using Raylib.
 *
 * This file contains all game logic, rendering, and input handling
 * for a simple snake game on a grid.
 * 
 * @author CJendantix
 * @date 2025-10-22
 */

#include <ranges>
#include <raylib.h>
#include <queue>
#include <random>

/**
 * @brief Represents the four possible movement directions of the snake.
 */
enum class Direction
{
    UP,    /**< Move up */
    DOWN,  /**< Move down */
    LEFT,  /**< Move left */
    RIGHT  /**< Move right */
};

/**
 * @brief Represents a 2D integer vector for grid coordinates.
 */
struct Vector2Int
{
    int x; /**< X coordinate */
    int y; /**< Y coordinate */

    /**
     * @brief Equality operator.
     * @param other Vector to compare
     * @return True if both x and y match
     */
    bool operator==(const Vector2Int &other) const = default;
};

/**
 * @brief Represents the game state.
 */
struct Game
{
    int width;                     /**< Width of the game grid */
    int height;                    /**< Height of the game grid */
    std::deque<Vector2Int> snake;  /**< Snake body coordinates */
    Vector2Int apple;              /**< Current apple position */
    Direction direction;           /**< Current snake direction */
    std::queue<Direction> directionQueue; /**< Queue of next directions */

    /**
     * @brief Construct a new Game object.
     * @param w Width of the grid
     * @param h Height of the grid
     * @param dir Initial snake direction
     * @param applePos Initial apple position
     * @param initialSnake Initial snake body
     */
    Game(int w, int h, Direction dir, const Vector2Int &applePos, const std::deque<Vector2Int> &initialSnake)
        : width(w), height(h), snake(initialSnake), apple(applePos), direction(dir) {}
};

// Screen and game constants
constexpr int SCREEN_WIDTH = 800;        /**< Window width */
constexpr int SCREEN_HEIGHT = 450;       /**< Window height */
constexpr int BORDER_THICKNESS = 2;      /**< Border thickness around the game grid */
constexpr int FPS = 60;                  /**< Target frames per second */
constexpr float MOVE_INTERVAL = 0.1f;    /**< Time between snake moves in seconds */
const Color SNAKE_HEAD_COLOR{71, 130, 255, 255}; /**< Base snake head color */
const Color BORDER_COLOR{0, 0, 0, 255};          /**< Border color */
const Color BORDER_BG{160, 255, 112, 255};      /**< Background behind the border */

/**
 * @brief Calculates the size of a single grid cell.
 * @param gameWidth Number of cells horizontally
 * @param gameHeight Number of cells vertically
 * @param screenWidth Screen width in pixels
 * @param screenHeight Screen height in pixels
 * @return Cell size in pixels
 */
int GetCellSize(int gameWidth, int gameHeight, int screenWidth, int screenHeight)
{
    int cellWidth = (screenWidth - BORDER_THICKNESS * 2) / gameWidth;
    int cellHeight = (screenHeight - BORDER_THICKNESS * 2) / gameHeight;
    return std::min(cellWidth, cellHeight);
}

/**
 * @brief Returns the coordinate offset for a given direction.
 * @param dir Direction to convert
 * @return Vector2Int offset corresponding to the direction
 */
Vector2Int OffsetFromDirection(Direction dir)
{
    using enum Direction;
    switch (dir)
    {
    case UP: return {0, -1};
    case DOWN: return {0, 1};
    case LEFT: return {-1, 0};
    case RIGHT: return {1, 0};
    }
}

/**
 * @brief Checks whether the snake collides with walls or itself.
 * @param game Current game state
 * @param newHead Position of the new snake head
 * @return True if collision occurs
 */
bool IsGameOver(const Game &game, const Vector2Int &newHead)
{
    if (newHead.x < 0 || newHead.x >= game.width || newHead.y < 0 || newHead.y >= game.height)
        return true;
    return std::ranges::find(game.snake, newHead) != game.snake.end();
}

/**
 * @brief Generates a random position for the apple, avoiding the snake.
 * @param game Current game state
 * @return Valid apple position
 */
Vector2Int GetNewApplePosition(const Game &game)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<Vector2Int> emptyCells;
    for (int x = 0; x < game.width; ++x)
    {
        for (int y = 0; y < game.height; ++y)
        {
            Vector2Int pos{x, y};
            if (std::ranges::find(game.snake, pos) == game.snake.end())
            {
                emptyCells.push_back(pos);
            }
        }
    }
    
    if (!emptyCells.empty())
    {
        std::uniform_int_distribution dis(0, static_cast<int>(emptyCells.size()) - 1);
        return emptyCells[dis(gen)];
    }

    return Vector2Int{0, 0};
}

/**
 * @brief Resets the snake and apple to start a new game.
 * @param game Game state to reset
 */
void ResetGame(Game &game)
{
    int centerX = game.width / 2;
    int centerY = game.height / 2;

    Vector2Int head = {centerX, centerY};
    Vector2Int offset = OffsetFromDirection(game.direction);
    Vector2Int second = {head.x - offset.x, head.y - offset.y};
    Vector2Int third = {second.x - offset.x, second.y - offset.y};

    game.snake = {head, second, third};
    game.apple = GetNewApplePosition(game);
}

/**
 * @brief Adds a valid direction to the snake's movement queue.
 * @param game Game state
 * @param newDirection Direction to enqueue
 */
void QueueDirection(Game &game, Direction newDirection)
{
    using enum Direction;
    if (game.directionQueue.size() >= 3)
        return;

    Direction lastDirection = game.directionQueue.empty() ? game.direction : game.directionQueue.back();

    if ((newDirection == LEFT && lastDirection != RIGHT) ||
        (newDirection == RIGHT && lastDirection != LEFT) ||
        (newDirection == UP && lastDirection != DOWN) ||
        (newDirection == DOWN && lastDirection != UP))
    {
        game.directionQueue.push(newDirection);
    }
}

/**
 * @brief Handles keyboard input for snake movement.
 * @param game Game state
 */
void HandleInput(Game &game)
{
    using enum Direction;
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) QueueDirection(game, LEFT);
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) QueueDirection(game, RIGHT);
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) QueueDirection(game, UP);
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) QueueDirection(game, DOWN);
}

/**
 * @brief Updates the snake's position and game state.
 * @param game Game state
 * @return True if game over occurs, false otherwise
 */
bool Update(Game &game)
{
    if (!game.directionQueue.empty())
    {
        game.direction = game.directionQueue.front();
        game.directionQueue.pop();
    }

    Vector2Int offset = OffsetFromDirection(game.direction);
    Vector2Int newHead{game.snake.front().x + offset.x, game.snake.front().y + offset.y};

    if (IsGameOver(game, newHead)) return true;

    game.snake.push_front(newHead);

    if (newHead == game.apple)
        game.apple = GetNewApplePosition(game);
    else
        game.snake.pop_back();

    return false;
}

/**
 * @brief Renders the game grid, snake, and apple.
 * @param game Game state
 * @param screenWidth Width of the screen
 * @param screenHeight Height of the screen
 */
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

/**
 * @brief Entry point of the program. Initializes and runs the game loop.
 * @return Exit status
 */
int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Snake");
    SetTargetFPS(FPS);

    Game game(25, 25, Direction::RIGHT, {0, 0}, {});
    ResetGame(game);

    float moveTimer = 0.0f;

    while (!WindowShouldClose())
    {
        moveTimer += GetFrameTime();

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

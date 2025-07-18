/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "game.h"
#include "config.h"
#include "resource_manager.h"
#include "renderer.h"

SpriteRenderer *sprite_Renderer;

// Game-related State data
SpriteRenderer *Renderer;
GameObject *Player;
BallObject *Ball;

Game::Game(GLuint width, GLuint height) : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
    delete Renderer;
    delete Player;
    delete Ball;
}

void Game::Init()
{
    // Load shaders
    ResourceManager::LoadShader(RES_DIR "shaders/sprite.vs", RES_DIR "shaders/sprite.fs", nullptr, "sprite");
    // Configure shaders
    glm::mat4 projection =
        glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // Load textures
    ResourceManager::LoadTexture(RES_DIR "textures/background.jpg", GL_FALSE, "background");
    ResourceManager::LoadTexture(RES_DIR "textures/awesomeface.png", GL_TRUE, "face");
    ResourceManager::LoadTexture(RES_DIR "textures/block.png", GL_FALSE, "block");
    ResourceManager::LoadTexture(RES_DIR "textures/block_solid.png", GL_FALSE, "block_solid");
    ResourceManager::LoadTexture(RES_DIR "textures/paddle.png", true, "paddle");
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // Load levels
    GameLevel one;
    one.Load(RES_DIR "levels/one.lvl", this->Width, this->Height * 0.5);
    GameLevel two;
    two.Load(RES_DIR "levels/two.lvl", this->Width, this->Height * 0.5);
    GameLevel three;
    three.Load(RES_DIR "levels/three.lvl", this->Width, this->Height * 0.5);
    GameLevel four;
    four.Load(RES_DIR "levels/four.lvl", this->Width, this->Height * 0.5);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;
    // Configure game objects
    glm::vec2 playerPos = glm::vec2(this->Width / 2.0 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::Update(GLfloat dt)
{
    // Update objects
    Ball->Move(dt, this->Width);
    // Check for collisions
    this->DoCollisions();
    // Check loss condition
    if (Ball->Position.y >= this->Height) // Did ball reach bottom edge?
    {
        this->ResetLevel();
        this->ResetPlayer();
    }
}

void Game::ProcessInput(GLfloat dt)
{
    if (this->State == GAME_ACTIVE)
    {
        GLfloat velocity = PLAYER_VELOCITY * dt;
        // Move playerboard
        if (this->Keys[GLFW_KEY_A])
        {
            if (Player->Position.x >= 0)
            {
                Player->Position.x -= velocity;
                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Player->Position.x <= this->Width - Player->Size.x)
            {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
    }
}

void Game::Render()
{
    if (this->State == GAME_ACTIVE)
    {
        // Draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0, 0),
                             glm::vec2(this->Width, this->Height), 0.0f);
        // Draw level
        this->Levels[this->Level].Draw(*Renderer);
        // Draw player
        Player->Draw(*Renderer);
        // Draw ball
        Ball->Draw(*Renderer);
    }
}

void Game::ResetLevel()
{
    if (this->Level == 0)
        this->Levels[0].Load(RES_DIR "levels/one.lvl", this->Width, this->Height * 0.5f);
    else if (this->Level == 1)
        this->Levels[1].Load(RES_DIR "levels/two.lvl", this->Width, this->Height * 0.5f);
    else if (this->Level == 2)
        this->Levels[2].Load(RES_DIR "levels/three.lvl", this->Width, this->Height * 0.5f);
    else if (this->Level == 3)
        this->Levels[3].Load(RES_DIR "levels/four.lvl", this->Width, this->Height * 0.5f);
}

void Game::ResetPlayer()
{
    // Reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0 - PLAYER_SIZE.x / 2.0, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)),
                INITIAL_BALL_VELOCITY);
}

// Collision detection
GLboolean CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) // If collision is true
            {
                // Destroy block if not solid
                if (!box.IsSolid)
                    box.Destroyed = GL_TRUE;
                // Collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (dir == LEFT || dir == RIGHT) // Horizontal collision
                {
                    Ball->Velocity.x = -Ball->Velocity.x; // Reverse horizontal velocity
                    // Relocate
                    GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
                    if (dir == LEFT)
                        Ball->Position.x += penetration; // Move ball to right
                    else
                        Ball->Position.x -= penetration; // Move ball to left;
                }
                else // Vertical collision
                {
                    Ball->Velocity.y = -Ball->Velocity.y; // Reverse vertical velocity
                    // Relocate
                    GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
                    if (dir == UP)
                        Ball->Position.y -= penetration; // Move ball bback up
                    else
                        Ball->Position.y += penetration; // Move ball back down
                }
            }
        }
    }
    // Also check collisions for player pad (unless stuck)
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        // Check where it hit the board, and change velocity based on where it hit the board
        GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
        GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        GLfloat percentage = distance / (Player->Size.x / 2);
        // Then move accordingly
        GLfloat strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        // Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) *
                         glm::length(oldVelocity); // Keep speed consistent over both axes (multiply by length of old
                                                   // velocity, so total strength is not changed)
        // Fix sticky paddle
        Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
    }
}

GLboolean CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // Collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x && two.Position.x + two.Size.x >= one.Position.x;
    // Collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y && two.Position.y + two.Size.y >= one.Position.y;
    // Collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // Get center point circle first
    glm::vec2 center(one.Position + one.Radius);
    // Calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // Get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // Now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // Now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;

    if (glm::length(difference) <
        one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two,
                    // which they are at the end of each collision resolution stage.
        return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
    else
        return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
}

// Calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),  // up
        glm::vec2(1.0f, 0.0f),  // right
        glm::vec2(0.0f, -1.0f), // down
        glm::vec2(-1.0f, 0.0f)  // left
    };
    GLfloat max = 0.0f;
    GLuint best_match = -1;
    for (GLuint i = 0; i < 4; i++)
    {
        GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}

GameObject::GameObject()
    : Position(0, 0), Size(1, 1), Velocity(0.0f), Color(1.0f), Rotation(0.0f), Sprite(), IsSolid(false),
      Destroyed(false)
{
}

GameObject::GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color, glm::vec2 velocity)
    : Position(pos), Size(size), Velocity(velocity), Color(color), Rotation(0.0f), Sprite(sprite), IsSolid(false),
      Destroyed(false)
{
}

void GameObject::Draw(SpriteRenderer &renderer)
{
    renderer.DrawSprite(this->Sprite, this->Position, this->Size, this->Rotation, this->Color);
}

#include <fstream>
#include <sstream>

void GameLevel::Load(const GLchar *file, GLuint levelWidth, GLuint levelHeight)
{
    // Clear old data
    this->Bricks.clear();
    // Load from file
    GLuint tileCode;
    GameLevel level;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<GLuint>> tileData;
    if (fstream)
    {
        while (std::getline(fstream, line)) // Read each line from level file
        {
            std::istringstream sstream(line);
            std::vector<GLuint> row;
            while (sstream >> tileCode) // Read each word seperated by spaces
                row.push_back(tileCode);
            tileData.push_back(row);
        }
        if (tileData.size() > 0)
            this->init(tileData, levelWidth, levelHeight);
    }
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
    for (GameObject &tile : this->Bricks)
        if (!tile.Destroyed)
            tile.Draw(renderer);
}

GLboolean GameLevel::IsCompleted()
{
    for (GameObject &tile : this->Bricks)
        if (!tile.IsSolid && !tile.Destroyed)
            return GL_FALSE;
    return GL_TRUE;
}

void GameLevel::init(std::vector<std::vector<GLuint>> tileData, GLuint levelWidth, GLuint levelHeight)
{
    // Calculate dimensions
    GLuint height = tileData.size();
    GLuint width =
        tileData[0].size(); // Note we can index vector at [0] since this function is only called if height > 0
    GLfloat unit_width = levelWidth / static_cast<GLfloat>(width), unit_height = levelHeight / static_cast<GLfloat>(height);
    // Initialize level tiles based on tileData
    for (GLuint y = 0; y < height; ++y)
    {
        for (GLuint x = 0; x < width; ++x)
        {
            // Check block type from level data (2D level array)
            if (tileData[y][x] == 1) // Solid
            {
                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width, unit_height);
                GameObject obj(pos, size, ResourceManager::GetTexture("block_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
                obj.IsSolid = GL_TRUE;
                this->Bricks.push_back(obj);
            }
            else if (tileData[y][x] > 1) // Non-solid; now determine its color based on level data
            {
                glm::vec3 color = glm::vec3(1.0f); // original: white
                if (tileData[y][x] == 2)
                    color = glm::vec3(0.2f, 0.6f, 1.0f);
                else if (tileData[y][x] == 3)
                    color = glm::vec3(0.0f, 0.7f, 0.0f);
                else if (tileData[y][x] == 4)
                    color = glm::vec3(0.8f, 0.8f, 0.4f);
                else if (tileData[y][x] == 5)
                    color = glm::vec3(1.0f, 0.5f, 0.0f);

                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width, unit_height);
                this->Bricks.push_back(GameObject(pos, size, ResourceManager::GetTexture("block"), color));
            }
        }
    }
}

BallObject::BallObject() : GameObject(), Radius(12.5f), Stuck(true)
{
}

BallObject::BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity, Texture2D sprite)
    : GameObject(pos, glm::vec2(radius * 2, radius * 2), sprite, glm::vec3(1.0f), velocity), Radius(radius), Stuck(true)
{
}

glm::vec2 BallObject::Move(GLfloat dt, GLuint window_width)
{
    // If not stuck to player board
    if (!this->Stuck)
    {
        // Move the ball
        this->Position += this->Velocity * dt;
        // Then check if outside window bounds and if so, reverse velocity and restore at correct position
        if (this->Position.x <= 0.0f)
        {
            this->Velocity.x = -this->Velocity.x;
            this->Position.x = 0.0f;
        }
        else if (this->Position.x + this->Size.x >= window_width)
        {
            this->Velocity.x = -this->Velocity.x;
            this->Position.x = window_width - this->Size.x;
        }
        if (this->Position.y <= 0.0f)
        {
            this->Velocity.y = -this->Velocity.y;
            this->Position.y = 0.0f;
        }
    }
    return this->Position;
}

// Resets the ball to initial Stuck Position (if ball is outside window bounds)
void BallObject::Reset(glm::vec2 position, glm::vec2 velocity)
{
    this->Position = position;
    this->Velocity = velocity;
    this->Stuck = true;
}
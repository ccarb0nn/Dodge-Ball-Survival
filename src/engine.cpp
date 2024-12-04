#include "engine.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <thread>
#include <random>

#include "shapes/circle.h"

// Screen settings
enum state {start, selection, play, lvlUP, lost, over, win};
state screen;
// Start = rules / objective
// Selection = choose player color
// Play = start / play level
// lvlUP = inform player they beat the previous level and wait for them to continue to start the next level (Play)
// Over = Player lost all 3 lives and died before beating level 5
// Win = Player beat level 5 / beat the game

const color WHITE(1, 1, 1);
const color BLACK(0, 0, 0);
const color BLUE(0, 0, 1);
const color YELLOW(1, 1, 0);
const color RED(1, 0, 0);
const color GRAY(0.7f, 0.7f, 0.7f);
const color PURPLE(0.8f, 0, 0.8f);

//Hover fill for Player Color Selection
const color redHoverFill(0.8f, 0, 0);          // Darker Red
const color whiteHoverFill(0.8f, 0.8f, 0.8f);  // Darker White
const color blueHoverFill(0, 0, 0.8f);         // Darker Blue
const color yellowHoverFill(0.8f, 0.8f, 0);      // Darker Yellow
const color grayHoverFill(0.5f, 0.5f, 0.5f);   // Darker Gray
const color purpleHoverFill(0.5f, 0, 0.5f);    // Darker Purple

// --- GAME ---
// game start count down
float gameCountDown;
int startTime = 4;
bool selected = false;
// --- Level ---
// Level count and Level timer
float countDownStarts;
int countDownTime = 20;
int lvl = 1;
// --- Player ---
// Players life count, default color, time survived, and godMode status
int life = 3;
color playerColor = WHITE;
float timeSurvived;
bool playerGodMode = false;
bool startGame = false;
// --- Easter Egg ---
// Easter egg status
bool EE1 = false;

// Player Placeholder For Player Location Viewing
color samplePLayerColor = WHITE;

// Second way of generating my random values -> dist(rd)
std::random_device rd;
std::uniform_int_distribution<> dist(0, 10000);

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(WIDTH, HEIGHT, "engine", nullptr, nullptr); // adjusted screen size
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // Load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Bubble / Circle shader
    shapeShader = this->shaderManager->loadShader("../res/shaders/circle.vert",
                                                  "../res/shaders/circle.frag",
                                                  nullptr, "circle");
    // Player / Rectangle shader
    playerShader = this->shaderManager->loadShader("../res/shaders/shape.vert",
                                              "../res/shaders/shape.frag",
                                              nullptr, "shape");


    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));

    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);

    playerShader.use();
    playerShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {

    // Player (Square/Rect) centered in the middle
    player = make_unique<Rect>(playerShader, vec2{WIDTH/2,HEIGHT/2}, 20, playerColor);
    // --- Player color options (buttons) ---
    //White
    whitePlayer = make_unique<Rect>(playerShader, vec2{WIDTH/2,HEIGHT/2.4}, vec2{100, 80}, WHITE);
    //Red
    redPlayer = make_unique<Rect>(playerShader, vec2{WIDTH/2.4,HEIGHT/2.4}, vec2{100, 80}, RED);
    //Blue
    bluePlayer = make_unique<Rect>(playerShader, vec2{WIDTH/2 + 130,HEIGHT/2.4}, vec2{100, 80}, BLUE);
    //Yellow
    yellowPlayer = make_unique<Rect>(playerShader, vec2{WIDTH/2.4,HEIGHT/3}, vec2{100, 80}, YELLOW);
    //Gray
    grayPlayer = make_unique<Rect>(playerShader, vec2{WIDTH/2,HEIGHT/3}, vec2{100, 80}, GRAY);
    //Purple
    purplePlayer = make_unique<Rect>(playerShader, vec2{WIDTH/2 + 130,HEIGHT/3}, vec2{100, 80}, PURPLE);

    // --- Other ---
    //God Mode
    godMode = make_unique<Rect>(playerShader, vec2{WIDTH/10, HEIGHT/20.1}, vec2{100, 80}, WHITE);
    // Player Location Placeholder For Viewing
    playerLocation = make_unique<Rect>(playerShader, vec2{WIDTH/2,HEIGHT/2}, 20, samplePLayerColor);

    // Bubble Stats
    int numberOfBubbles;
    float minRadius;
    float maxRadius;
    float maxSpeed ;
    vec4 color;

    // Update Bubble stats based on level
    // (LVL 1) Small bubbles, slow speed, fewewr spawn in
    if(lvl == 1) {
        numberOfBubbles = 75;
        minRadius = 5;
        maxRadius = 15;
        maxSpeed = 35;
    }
    // (LVL 2) Bubble size increases, Speed increases, Spawn count increases
    if(lvl == 2) {
        //set new level
        numberOfBubbles = 80;
        minRadius = 5;
        maxRadius = 18;
        maxSpeed = 40;
    }
    // (LVL 3) Bubble size increases, Speed increases, Spawn count increases
    if(lvl == 3) {
        //Set new level
        numberOfBubbles = 85;
        minRadius = 5;
        maxRadius = 20;
        maxSpeed = 45;
    }
    // (LVL 4) Bubble size increases, Speed increases
    if(lvl == 4) {
        //set new level
        numberOfBubbles = 90;
        minRadius = 5;
        maxRadius = 23;
        maxSpeed = 50;
    }
    // (LVL 5) Bubble size increases, Speed increases, Spawn count increases
    if(lvl == 5) {
        //set new level
        numberOfBubbles = 95;
        minRadius = 5;
        maxRadius = 25;
        maxSpeed = 55;
    }

    //Create Bubbles
    for (int i = 0; i < numberOfBubbles; ++i) {

        float x = dist(rd) % WIDTH;
        float y = dist(rd) % HEIGHT;
        vec2 position = vec2(x, y);
        float radius = dist(rd) % int(maxRadius - minRadius) + minRadius;
        vec2 velocity(dist(rd) % int(maxSpeed), dist(rd) % int(maxSpeed));

        // Each Level Has A Unique Color Pallet
        if(lvl == 1) {
            // Shades of Green/White
            color = vec4(0.7f + (dist(rd) % 55) / 255.0f, 0.9f + (dist(rd) % 35) / 255.0f, 0.6f + (dist(rd) % 45) / 255.0f, dist(rd) % 120+135);
        }
        if(lvl == 2) {
            // Shades of Blue/Purple
            color = vec4(0.3f + (dist(rd) % 100) / 255.0f, (dist(rd) % 40) / 255.0f, 0.6f + (dist(rd) % 155) / 255.0f, dist(rd) % 120+135);
        }
        if(lvl == 3) {
            // Shades of Purple
            color = vec4(dist(rd) % 40 / 255.0f, dist(rd) % 80 / 255.0f + 0.3f, 0.8f + dist(rd) % 120 / 255.0f, dist(rd) % 120+135);
        }
        if(lvl == 4) {
            // Shades of Yellow/White
            color = vec4(1.0f, 1.0f, (dist(rd) % 256) / 255.0f, dist(rd) % 120+135);
        }
        if(lvl == 5) {
            // Shades of RED
            color = vec4(1.0f, 0.2f + dist(rd) % 128 / 255.0f, 0.2f + dist(rd) % 128 / 255.0f, dist(rd) % 120+135);
        }
        // Add the bubble to bubbles
        unique_ptr<Circle> bubble = make_unique<Circle>(shapeShader, position, radius, velocity, color);
        bubble->setVelocity(velocity);
        bubbles.push_back(std::move(bubble));
    }

    // Initialize confetti off screen
    for (int i = 0; i < 150; ++i) {
        vec4 colorConfetti = {float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), 1.0f};
        confeti.push_back(make_unique<Circle>(shapeShader, vec2(rand() % WIDTH, HEIGHT + 2 + (rand() % HEIGHT)),
                                           (rand() % 5 / 5.0) + 1, colorConfetti));
    }

}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // When player presses "C" redirect them to player selection (When at start screen)
    if (screen == start) {
        if (keys[GLFW_KEY_C]) {
            screen = selection;
        }
    }

    mouseY = HEIGHT - mouseY; // Invert y-axis of mouse position
    // --- Button / Mouse Interaction (Colored Button Options) ---
    bool whiteButtonOverlapsMouse = whitePlayer->isMouseOverlaping(vec2(mouseX, mouseY));   // WHITE
    bool redButtonOverlapsMouse = redPlayer->isMouseOverlaping(vec2(mouseX, mouseY));       // RED
    bool blueButtonOverlapsMouse = bluePlayer->isMouseOverlaping(vec2(mouseX, mouseY));     // BLUE
    bool yellowButtonOverlapsMouse = yellowPlayer->isMouseOverlaping(vec2(mouseX, mouseY)); // YELLOW
    bool grayButtonOverlapsMouse = grayPlayer->isMouseOverlaping(vec2(mouseX, mouseY));     // GRAY
    bool purpleButtonOverlapsMouse = purplePlayer->isMouseOverlaping(vec2(mouseX, mouseY)); // PURPLE
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // When player selects character, start game after they clicked the color they want to use (When at selection screen)
    if(screen == selection) {
        if (!selected) {
            // --- Button / Mouse Interaction (Mouse hovers over button / No click) ---
            if(redButtonOverlapsMouse) {               // RED
                redPlayer->setColor(redHoverFill);
                playerLocation->setColor(RED);
            }
            else {
                redPlayer->setColor(RED);
            }
            if(whiteButtonOverlapsMouse) {            // WHITE
                whitePlayer->setColor(whiteHoverFill);
                playerLocation->setColor(WHITE);
            }
            else {
                whitePlayer->setColor(WHITE);
            }
            if(blueButtonOverlapsMouse) {            // BLUE
                bluePlayer->setColor(blueHoverFill);
                playerLocation->setColor(BLUE);
            }
            else {
                bluePlayer->setColor(BLUE);
            }
            if(yellowButtonOverlapsMouse) {         // YELLOW
                yellowPlayer->setColor(yellowHoverFill);
                playerLocation->setColor(YELLOW);
            }
            else {
                yellowPlayer->setColor(YELLOW);
            }
            if(grayButtonOverlapsMouse) {           // GRAY
                grayPlayer->setColor(grayHoverFill);
                playerLocation->setColor(GRAY);
            }
            else {
                grayPlayer->setColor(GRAY);
            }
            if(purpleButtonOverlapsMouse) {        // PURPLE
                purplePlayer->setColor(purpleHoverFill);
                playerLocation->setColor(PURPLE);
            }
            else {
                purplePlayer->setColor(PURPLE);
            }
        }

        // --- Button / Mouse interaction (Mouse Click) ---
        if((redButtonOverlapsMouse && mousePressedLastFrame == GLFW_PRESS && mousePressed != GLFW_PRESS) || keys[GLFW_KEY_R]) { // RED
            player->setColor(RED);
            playerLocation->setColor(RED);
            playerColor = RED;
            selected = true;
            startGame = true;
            gameCountDown = glfwGetTime();
        }
        if((whiteButtonOverlapsMouse && mousePressedLastFrame == GLFW_PRESS && mousePressed != GLFW_PRESS) || keys[GLFW_KEY_W]) { // WHITE
            player->setColor(WHITE);
            playerLocation->setColor(WHITE);
            playerColor = WHITE;
            selected = true;
            startGame = true;
            gameCountDown = glfwGetTime();
        }
        if((blueButtonOverlapsMouse && mousePressedLastFrame == GLFW_PRESS && mousePressed != GLFW_PRESS) || keys[GLFW_KEY_B]) { // BLUE
            player->setColor(BLUE);
            playerLocation->setColor(BLUE);
            playerColor = BLUE;
            selected = true;
            startGame = true;
            gameCountDown = glfwGetTime();
        }
        if((yellowButtonOverlapsMouse && mousePressedLastFrame == GLFW_PRESS && mousePressed != GLFW_PRESS) || keys[GLFW_KEY_Y]) { // YELLOW
            player->setColor(YELLOW);
            playerLocation->setColor(YELLOW);
            playerColor = YELLOW;
            selected = true;
            startGame = true;
            gameCountDown = glfwGetTime();
        }
        if((grayButtonOverlapsMouse && mousePressedLastFrame == GLFW_PRESS && mousePressed != GLFW_PRESS) || keys[GLFW_KEY_G]) { // GRAY
            player->setColor(GRAY);
            playerLocation->setColor(GRAY);
            playerColor = GRAY;
            selected = true;
            startGame = true;
            gameCountDown = glfwGetTime();
        }
        if((purpleButtonOverlapsMouse && mousePressedLastFrame == GLFW_PRESS && mousePressed != GLFW_PRESS) || keys[GLFW_KEY_P]) { // PURPLE
            player->setColor(PURPLE);
            playerLocation->setColor(PURPLE);
            playerColor = PURPLE;
            selected = true;
            startGame = true;
            gameCountDown = glfwGetTime();
        }
        // -- EASTER EGG 1 --
        //Player color is set to rainbow
        if(keys[GLFW_KEY_E]) {
            EE1 = true;
            startGame = true;
            gameCountDown = glfwGetTime();
        }

        //Save mousePressed for next frame
        mousePressedLastFrame = mousePressed;

        //Starting the countdown timer for the level
        countDownStarts = glfwGetTime();
    }

    // Hidden God Mode button which makes player invincible to the bubbles when clicked, can be turned off when clicked again (Button is located by player life count
    bool godModeButtonOverlapsMouse = godMode->isMouseOverlaping(vec2(mouseX, mouseY)); // PURPLE
    if(screen == play) {
        //Turning God Mode on/off
        if(godModeButtonOverlapsMouse && mousePressedLastFrame == GLFW_RELEASE && mousePressed == GLFW_PRESS) {
            playerGodMode = !playerGodMode;
        }

        //Save mousePressed for next frame
        mousePressedLastFrame = mousePressed;
    }

    // User starts game with "S" (only if user has not lost the game)
    if(screen != over && screen != selection && screen != start) {
        if(keys[GLFW_KEY_S]) {
            //User starts game
            screen = play;
            //Starting the countdown timer for the level
            countDownStarts = glfwGetTime();
        }
    }

    // When player loses level and tries again check if they have 3 lives left
    if (screen == lost) {
        if(keys[GLFW_KEY_S] && life != 0) {
            //Making sure player stays at same level
            lvl--;
            //Transition to level
            screen = play;
            //Starting the countdown timer for the level
            countDownStarts = glfwGetTime();
        }
        else if(life == 0){
            // Get the losing pixel art from the scene.txt file
            readFromFile(R"(C:\Users\crcar\CLionProjects\Dodge-Ball-Survival\res\art\scene.txt)");
            // Change to Game Over Screen
            screen = over;
        }
    }

    //If user is playing the game -> allow for player movement
    if(screen == play) {
        //Player is moved by the arrow keys
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            if (player->getPosY() + player->getSize().y / 2 < HEIGHT) {
                player->moveY(1.1f);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            if (player->getPosY() - player->getSize().y / 2 > 0) {
                player->moveY(-1.1f);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            if (player->getPosX() - player->getSize().x / 2 > 0) {
                player->moveX(-1.1f);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            if (player->getPosX() + player->getSize().x / 2 < WIDTH) {
                player->moveX(1.1f);
            }
        }

        //Space bar gives player a boost
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                if (player->getPosY() + player->getSize().y / 2 < HEIGHT) {
                    player->moveY(1.3f);
                }
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                if (player->getPosY() - player->getSize().y / 2 > 0) {
                    player->moveY(-1.3f);
                }
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                if (player->getPosX() - player->getSize().x / 2 > 0) {
                    player->moveX(-1.3f);
                }
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                if (player->getPosX() + player->getSize().x / 2 < WIDTH) {
                    player->moveX(1.3f);
                }
            }
        }
    }
}


void Engine::checkBounds(unique_ptr<Circle> &bubble) const {
    vec2 position = bubble->getPos();
    vec2 velocity = bubble->getVelocity();
    float bubbleRadius = bubble->getRadius();

    position += velocity * deltaTime;

    // If any bubble hits the edges of the screen, bounce it in the other direction
    if (position.x - bubbleRadius <= 0) {
        position.x = bubbleRadius;
        velocity.x = -velocity.x;
    }
    if (position.x + bubbleRadius >= WIDTH) {
        position.x = WIDTH - bubbleRadius;
        velocity.x = -velocity.x;
    }
    if (position.y - bubbleRadius <= 0) {
        position.y = bubbleRadius;
        velocity.y = -velocity.y;
    }
    if (position.y + bubbleRadius >= HEIGHT) {
        position.y = HEIGHT - bubbleRadius;
        velocity.y = -velocity.y;
    }

    bubble->setPos(position);
    bubble->setVelocity(velocity);
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float timePassed;

    // Only update player & bubble collision when screen is set to play
    // Only start timer when screen is set to play
    // When the screen is set to play the user is playing the level/game
    if(screen == play) {

        // Countdown timer (for level)
        timePassed = glfwGetTime() - countDownStarts;
        if(timePassed >= countDownTime) {
            lvl++;
            if(lvl > 5) {
                // Get the winning pixel art from the scene2.txt file
                readFromFile(R"(C:\Users\crcar\CLionProjects\Dodge-Ball-Survival\res\art\scene2.txt)");
                screen = win;
            }
            else {
                bubbles.clear();
                screen = lvlUP;
                initShapes();
                countDownTime = 20;
                countDownStarts = glfwGetTime();
            }
        }

        // Bubble & Bubble Collision Check
        // Player & Bubble Collision Check
        for (unique_ptr<Circle> &bubble: bubbles) {
            // Prevent bubbles from moving offscreen
            checkBounds(bubble);
            // Check for collisions
            for (unique_ptr<Circle> &other: bubbles) {
                // Let the player spawn in and have a few seconds before collision check is activated
                if(timePassed >= 1.5f) {
                    // Bubble and Player collision = level lost (lose a life)
                    if(bubble->isOverlapping(*player) && timePassed != 19.5 && playerGodMode != true) {
                        if(screen == play) {
                            //subtract users life from that level
                            life--;
                            timeSurvived = timePassed;
                            screen = lost;
                        }
                    }
                }
                //Bubble collision with other bubbles
                if (bubble != other && bubble->isOverlapping(*other)) {
                    bubble->bounce(*other);
                }
            }
        }
        // --- EASTER EGG 1 ---
        // set users color to rainbow (also used to show when user is in God Mode)
        if(EE1 || playerGodMode == true) {
            color randomColor = {float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), 1.0f};
            player->setColor(randomColor);
        }
        //When god mode is turned off turn the player back to selected color
        else {
            player->setColor(playerColor);
        }
    }
    if(screen == win) {
        for (unique_ptr<Shape> &feti : confeti) {
            feti->moveY(-feti->getSize().y / 5.0);
            if (feti->getPosY() < 0) {
                feti->setPos(vec2(dist(rd) % WIDTH, HEIGHT + feti->getSize().y));
            }
        }
    }
}

void Engine::render() {
    glClearColor(BLACK.red, BLACK.green, BLACK.blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shapeShader.use();

    // Game Screen Versions
    switch(screen) {
        // Welcome Screen
        case start: {
            //Displaying the goal of the game / controls / how to start
            string title = "* DODGE BALL SURVIVAL *";
            string breaker = "************************";
            string description = "Avoid the balls & survive 20 seconds to reach the next level";
            string d2 = "Beat level 5 to win the game!";

            string d4 = "[Use Arrow Keys To Move]";
            string d5 = "[Use Space Bar As A Boost]";
            string d6 = "[You Have 3 Lives]";

            string message = "- ENTER C TO CONTINUE -";
            this->fontRenderer->renderText(breaker, WIDTH/2 - (12 * breaker.length()), HEIGHT/1.1, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(title, WIDTH/2 - (12 * title.length()), HEIGHT/1.15, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(breaker, WIDTH/2 - (12 * breaker.length()), HEIGHT/1.2, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(description, WIDTH/2 - (12 * description.length()), HEIGHT/1.3, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(d2, WIDTH/2 - (12 * d2.length()), HEIGHT/1.4, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(d4, WIDTH/2 - (12 * d4.length()), HEIGHT/1.8, projection, 1, vec3{1, 1, 0});
            this->fontRenderer->renderText(d5, WIDTH/2 - (12 * d5.length()), HEIGHT/2.15, projection, 1, vec3{1, 1, 0});
            this->fontRenderer->renderText(d6, WIDTH/2 - (12 * d6.length()), HEIGHT/2.8, projection, 1, vec3{1, 1, 0});
            this->fontRenderer->renderText(message, WIDTH/2 - (12 * message.length()), HEIGHT/5.8, projection, 1, vec3{1, 1, 1});
            break;
        }
        // User gets to pick the color of their player (click on the colored box)
        case selection: {
            string description = "PICK YOUR PLAYER COLOR TO START";
            this->fontRenderer->renderText(description, WIDTH/2 - (12 * description.length()), HEIGHT/1.5, projection, 1, vec3{1, 1, 1});

            // --- Player Color Selection Buttons ---
            playerShader.use();
            // White
            whitePlayer->setUniforms();
            whitePlayer->draw();
            // Red
            redPlayer->setUniforms();
            redPlayer->draw();
            // Blue
            bluePlayer->setUniforms();
            bluePlayer->draw();
            // Yellow
            yellowPlayer->setUniforms();
            yellowPlayer->draw();
            // Gray
            grayPlayer->setUniforms();
            grayPlayer->draw();
            // Purple
            purplePlayer->setUniforms();
            purplePlayer->draw();

            // Sample Player Model
            playerLocation->setUniforms();
            playerLocation->draw();

            // Player Color Selection Button Text
            string white = "W";
            this->fontRenderer->renderText(white, WIDTH/2 - (12 * white.length()), HEIGHT/2.45, projection, 1, vec3{0, 0, 0});

            string red = "R";
            this->fontRenderer->renderText(red, WIDTH/2.4 - (12 * red.length()), HEIGHT/2.45, projection, 1, vec3{0, 0, 0});

            string blue = "B";
            this->fontRenderer->renderText(blue, WIDTH/2 + 130 - (12 * blue.length()), HEIGHT/2.45, projection, 1, vec3{0, 0, 0});

            string yellow = "Y";
            this->fontRenderer->renderText(yellow, WIDTH/2.4 - (12 * yellow.length()), HEIGHT/3.05, projection, 1, vec3{0, 0, 0});

            string gray = "G";
            this->fontRenderer->renderText(gray, WIDTH/2 - (12 * gray.length()), HEIGHT/3.05, projection, 1, vec3{0, 0, 0});

            string purple = "P";
            this->fontRenderer->renderText(purple, WIDTH/2 + 130 - (12 * purple.length()), HEIGHT/3.05, projection, 1, vec3{0, 0, 0});

            // Game Start Countdown (after color selection give a 3second countdown before starting the game so the player can get prepared)
            float timePassed;
            if (startGame) {
                // Countdown timer
                timePassed = glfwGetTime() - gameCountDown;
                //Display the countdown timer (Top right corner of the screen)
                float timeRemaining = startTime - (glfwGetTime() - gameCountDown);

                if (timeRemaining < 0) {
                    timeRemaining = 0;
                    screen = play;
                }
                string gameStart = "GAME STARTS IN: ";
                this->fontRenderer->renderText(gameStart, WIDTH/2.2 - (12 * gameStart.length()), HEIGHT/1.8, projection, 1, vec3{1, 1, 1});
                string timer = std::to_string(static_cast<int>(timeRemaining)) + "s";
                this->fontRenderer->renderText(timer, WIDTH/1.6 - (12 * timer.length()), HEIGHT/1.8, projection, 1, vec3{1, 1, 1});
            }
            break;
        }
        // Game screen
        case play: {
            //Spawn player
            playerShader.use();
            player->setUniforms();
            player->draw();

            //spawn bubbles
            shapeShader.use();
            for (unique_ptr<Circle>& bubble : bubbles) {
                bubble->setUniforms();
                bubble->draw();
            }

            // --- EASTER EGG = Process Game Hud with Random Colors ---
            if(EE1 == true) {
                // Seed the random number generator
                srand(static_cast<unsigned int>(time(0)));
                // Get the random color
                glm::vec3 randomColor = {float(rand() % 10) / 10.0f, float(rand() % 10) / 10.0f, float(rand() % 10) / 10.0f};

                //Get the current level and display it top left corner
                string currentLevel = "LVL " + std::to_string(lvl);
                this->fontRenderer->renderText(currentLevel, WIDTH/10 - (12 * currentLevel.length()), HEIGHT/1.1, projection, 1, randomColor);

                //Display the countdown timer (Top right corner of the screen)
                float timeRemaining = countDownTime - (glfwGetTime() - countDownStarts);
                if (timeRemaining < 0) {
                    timeRemaining = 0;
                }
                string timer = std::to_string(static_cast<int>(timeRemaining)) + "s";
                this->fontRenderer->renderText(timer, WIDTH/1.06 - (12 * timer.length()), HEIGHT/1.1, projection, 1, randomColor);

                //Get the number of lives the user has left (Bottom right corner of the screen)
                string livesLeft;
                if(life > 1) {
                    livesLeft = std::to_string(life) + "LIVES";
                }
                else {
                    livesLeft = std::to_string(life) + "LIFE";
                }
                this->fontRenderer->renderText(livesLeft, WIDTH/10 - (12 * livesLeft.length()), HEIGHT/20.1, projection, 1, randomColor);
            }
            // --- Process Game Hud as Default Settings (WHITE) ---
            else {
                //Get the current level and display it top left corner
                string currentLevel = "LVL " + std::to_string(lvl);
                this->fontRenderer->renderText(currentLevel, WIDTH/10 - (12 * currentLevel.length()), HEIGHT/1.1, projection, 1, vec3{1, 1, 1});

                //Display the countdown timer (Top right corner of the screen)
                float timeRemaining = countDownTime - (glfwGetTime() - countDownStarts);
                if (timeRemaining < 0) {
                    timeRemaining = 0;
                }
                string timer = std::to_string(static_cast<int>(timeRemaining)) + "s";
                this->fontRenderer->renderText(timer, WIDTH/1.06 - (12 * timer.length()), HEIGHT/1.1, projection, 1, vec3{1, 1, 1});

                //Get the number of lives the user has left (Bottom right corner of the screen)
                string livesLeft;
                if(life > 1) {
                    livesLeft = std::to_string(life) + "LIVES";
                }
                else {
                    livesLeft = std::to_string(life) + "LIFE";
                }
                this->fontRenderer->renderText(livesLeft, WIDTH/10 - (12 * livesLeft.length()), HEIGHT/20.1, projection, 1, vec3{1, 1, 1});

                // Hidden God Mode button (user takes no damage)
                godMode->setOpacity(0);
                playerShader.use();
                godMode->setUniforms();
                godMode->draw();
            }
            break;
        }
        // Level Up Pause Screen
        case lvlUP: {
            //Display next level
            string description = "YOU SURVIVED THAT ROUND!";
            string levelReached = "NEXT LEVEL IS " + std::to_string(lvl);
            string message = "PRESS S TO PLAY";
            string message2 = "OR";
            string message3 = "PRESS ESC TO GIVE UP";
            this->fontRenderer->renderText(description, WIDTH/2 - (12 * description.length()), HEIGHT/1.25, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(levelReached, WIDTH/2 - (12 * levelReached.length()), HEIGHT/1.4, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message, WIDTH/2 - (12 * message.length()), HEIGHT/3, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message2, WIDTH/2 - (12 * message2.length()), HEIGHT/4, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message3, WIDTH/2 - (12 * message3.length()), HEIGHT/6, projection, 1, vec3{1, 1, 1});

            //Drawing Player So They Can See Where They Spawn In
            playerShader.use();
            player->setUniforms();
            player->draw();

            break;
        }
        case lost:{
            //Displaying level Over message (telling the player what level they made it to and how many lives they have left)
            string description = "YOU DIED";
            string levelReached = "YOU REACHED LEVEL " + std::to_string(lvl);
            string time = "YOU SURVIVED " + std::to_string(timeSurvived) + "s";
            string message = "YOU HAVE " + std::to_string(life) + " LIVES LEFT";
            string message3 = "PRESS S TO TRY AGAIN";
            this->fontRenderer->renderText(description, WIDTH/2 - (12 * description.length()), HEIGHT/1.25, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(levelReached, WIDTH/2 - (12 * levelReached.length()), HEIGHT/1.4, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(time, WIDTH/2 - (12 * time.length()), HEIGHT/2, projection, 1, vec3{1, 1, 0});
            this->fontRenderer->renderText(message, WIDTH/2 - (12 * message.length()), HEIGHT/3.5, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message3, WIDTH/2 - (12 * message3.length()), HEIGHT/6, projection, 1, vec3{1, 1, 1});
            break;
        }
        // Game Over Screen (Player Loses)
        case over: {
            //Displaying Game over message (telling the player what level they made it to)
            string description = "GAME OVER YOU LOST";
            string levelReached = "YOU REACHED LEVEL " + std::to_string(lvl);
            string message = "PRESS ESCAPE TO EXIT";
            this->fontRenderer->renderText(description, WIDTH/2 - (12 * description.length()), HEIGHT/1.15, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(levelReached, WIDTH/2 - (12 * levelReached.length()), HEIGHT/1.3, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText(message, WIDTH/2 - (12 * message.length()), HEIGHT/6, projection, 1, vec3{1, 1, 1});

            // Game Over Pixel Art (scene.txt)
            playerShader.use();
            for (unique_ptr<Shape> &square : squares) {
                square->setUniforms();
                square->draw();
            }

            break;
        }
        // Winning Screen
        case win: {
            for (auto &c : confeti) {
                c->setUniforms();
                c->draw();
            }
            // Seed the random number generator
            srand(static_cast<unsigned int>(time(0)));
            // Get the random color
            glm::vec3 randomColor = {float(rand() % 10) / 10.0f, float(rand() % 10) / 10.0f, float(rand() % 10) / 10.0f};
            string description = "YOU WON";
            string levelReached = "YOU BEAT LEVEL 5";
            this->fontRenderer->renderText(description, WIDTH/2 - (12 * description.length()), HEIGHT/1.25, projection, 1, randomColor);
            this->fontRenderer->renderText(levelReached, WIDTH/2 - (12 * levelReached.length()), HEIGHT/1.4, projection, 1, randomColor);

            // Pixel Art
            playerShader.use();
            for (unique_ptr<Shape> &square : squares) {
                square->setUniforms();
                square->draw();
            }
            break;
        }
    }
    glfwSwapBuffers(window);
}

// Getting the Pixel Art from the file
void Engine::readFromFile(std::string filepath) {
    ifstream ins(filepath);
    if (!ins) {
        cout << "Error opening file" << endl;
    }
    ins >> std::noskipws;
    int xCoord = 0, yCoord = HEIGHT-SIDE_LENGTH;
    char letter;
    bool draw;
    color c;
    while (ins >> letter) {
        draw = true;
        switch(letter) {
            case 'r': c = color(1, 0, 0); break;
            case 'g': c = color(0.25f, 0.25f, 0.25f); break;
            case 'b': c = color(0, 0, 1); break;
            case 'y': c = color(1, 1, 0); break;
            case 'm': c = color(1, 0, 1); break;
            case 'c': c = color(0, 1, 1); break;
            case 'w': c = color(1, 1, 1); break;
            case ' ': c = color(0, 0, 0, 0); break;
            default: // newline
                draw = false;
            xCoord = 0;
            yCoord -= SIDE_LENGTH;
        }
        if (draw) {
            squares.push_back(make_unique<Rect>(playerShader, vec2(xCoord + SIDE_LENGTH/2, yCoord + SIDE_LENGTH/2), vec2(SIDE_LENGTH, SIDE_LENGTH), c));
            xCoord += SIDE_LENGTH;
        }
    }
    ins.close();
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}
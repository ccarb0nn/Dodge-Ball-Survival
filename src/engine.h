#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <vector>
#include <memory>
#include <iostream>
#include "GLFW/glfw3.h"

#include "shader/shaderManager.h"
#include "shapes/circle.h"
#include "shapes/rect.h"
#include "shapes/shape.h"
#include "font/fontRenderer.h"

using std::vector, std::unique_ptr, std::make_unique, glm::ortho, glm::mat4, glm::vec3, glm::vec4;

/**
 * @brief The Engine class.
 * @details The Engine class is responsible for initializing the GLFW window, loading shaders, and rendering the game state.
 */
class Engine {
    private:
        /// @brief The actual GLFW window.
        GLFWwindow* window{};

        /// @brief The width and height of the window.
        const unsigned int WIDTH = 1600, HEIGHT = 1200;
        const glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);

        bool keys[1024];

        /// @brief Responsible for loading and storing all the shaders used in the project.
        /// @details Initialized in initShaders()
        unique_ptr<ShaderManager> shaderManager;

        unique_ptr<FontRenderer> fontRenderer;

        // --- Shapes ---
        // The player (square)
        unique_ptr<Shape> player;
        // Bubbles (the objects the user must avoid)
        vector<unique_ptr<Circle>> bubbles;
        const int RADIUS = 50;
        //Confetti (spawns when user wins)
        vector<unique_ptr<Shape>> confeti;
        //Pixel Art
        vector<unique_ptr<Shape>> squares;

        // --- Player Color Options ---
        //Red
        unique_ptr<Shape> redPlayer;
        //White
        unique_ptr<Shape> whitePlayer;
        //Blue
        unique_ptr<Shape> bluePlayer;
        //Yellow
        unique_ptr<Shape> yellowPlayer;
        // Gray
        unique_ptr<Shape> grayPlayer;
        // Purple
        unique_ptr<Shape> purplePlayer;

        //God Mode (used to easily complete levels and check changes made, click lives left to engage)
        unique_ptr<Shape> godMode;

        //Players Location Placeholder
        unique_ptr<Shape> playerLocation;

        // Shaders
        Shader shapeShader;
        Shader playerShader;
        Shader textShader;

        double mouseX, mouseY;
        bool mousePressedLastFrame = false;

        //Pixel art
        const int SIDE_LENGTH = 20;

        /// @note Call glCheckError() after every OpenGL call to check for errors.
        GLenum glCheckError_(const char *file, int line);
        /// @brief Macro for glCheckError_ function. Used for debugging.
        #define glCheckError() glCheckError_(__FILE__, __LINE__)

    public:
        /// @brief Constructor for the Engine class.
        /// @details Initializes window and shaders.
        Engine();

        /// @brief Destructor for the Engine class.
        ~Engine();

        /// @brief Initializes the GLFW window.
        /// @return 0 if successful, -1 otherwise.
        unsigned int initWindow(bool debug = false);

        /// @brief Loads shaders from files and stores them in the shaderManager.
        /// @details Renderers are initialized here.
        void initShaders();

        //Counts down the time left in the level
        bool countDown();

        /// @brief Initializes the shapes to be rendered.
        void initShapes();

        /// @brief Populates squares vector with input from file.
        void readFromFile(string filepath);

        /// @brief Processes input from the user.
        /// @details (e.g. keyboard input, mouse input, etc.)
        void processInput();

        /// @brief Updates the game state.
        /// @details (e.g. collision detection, delta time, etc.)
        void update();

        /// @brief Renders the game state.
        /// @details Displays/renders objects on the screen.
        void render();

        /* deltaTime variables */
        float deltaTime = 0.0f; // Time between current frame and last frame
        float lastFrame = 0.0f; // Time of last frame (used to calculate deltaTime)

        // -----------------------------------
        // Getters
        // -----------------------------------

        /// @brief Returns true if the window should close.
        /// @details (Wrapper for glfwWindowShouldClose()).
        /// @return true if the window should close
        /// @return false if the window should not close
        bool shouldClose();

        /// Projection matrix used for 2D rendering (orthographic projection).
        /// We don't have to change this matrix since the screen size never changes.
        /// OpenGL uses the projection matrix to map the 3D scene to a 2D viewport.
        /// The projection matrix transforms coordinates in the camera space into normalized device coordinates (view space to clip space).
        /// @note The projection matrix is used in the vertex shader.
        // 1st quadrant
        mat4 PROJECTION = ortho(0.0f, static_cast<float>(WIDTH), 0.0f, static_cast<float>(HEIGHT), -1.0f, 1.0f);
        // 4th quadrant
        // mat4 PROJECTION = ortho(0.0f, static_cast<float>(WIDTH), static_cast<float>(HEIGHT), 0.0f, -1.0f, 1.0f);

        /// @brief Checks for collisions between all bubbles
        void checkCollisions();

        /// @brief Prevents bubbles from going off screen
        void checkBounds(unique_ptr<Circle> &bubble) const;

};

#endif //GRAPHICS_ENGINE_H
#ifndef GRAPHICS_SHAPE_H
#define GRAPHICS_SHAPE_H

#include "glm/glm.hpp"
#include <vector>
#include "../shader/shader.h"
#include "../framework/color.h"

using std::vector, glm::vec2, glm::vec3, glm::vec4, glm::mat4, glm::translate, glm::scale;

class Shape {
    public:
        /// @brief Construct a new Shape object
        /// @param shader The shader to use for rendering
        /// @param pos The position of the shape
        /// @param size The size of the shape
        /// @param color The color of the shape
        Shape(Shader& shader, vec2 pos, vec2 size, color color);

        Shape(Shader& shader, vec2 pos, vec2 size, vec4 color);

        /// @brief Copy constructor for Shape
        Shape(Shape const& other);

        /// @brief Destroy the Shape object
        virtual ~Shape() = default;

        // --------------------------------------------------------
        // Initialization functions
        // --------------------------------------------------------

        /// @brief Initializes the VAO.
        /// @details This function is called in the derived classes' constructor.
        unsigned int initVAO();

        /// @brief Initializes the VBO.
        /// @details This function is called in the derived classes' constructor.
        void initVBO();

        /// @brief Initializes the EBO.
        /// @details This function is called in the derived classes' constructor.
        /// @details If function has no indices, pass nullptr for indices and 0 for indexCount.
        void initEBO();

        // --------------------------------------------------------
        // Getters
        // --------------------------------------------------------
        // Position/Movement Functions
        float getPosX() const;
        float getPosY() const;
        vec2 getPos() const;
        virtual float getLeft() const = 0;
        virtual float getRight() const = 0;
        virtual float getTop() const = 0;
        virtual float getBottom() const = 0;

        // Color Functions
        vec4 getColor4() const;
        vec3 getColor3() const;
        float getRed() const;
        float getGreen() const;
        float getBlue() const;
        float getOpacity() const;

        // Size Functions
        vec2 getSize() const;

        // Velocity Functions
        vec2 getVelocity() const;
        void setVelocity(vec2 velocity);

        // Change Functions (add/sub to current value)
        void changePos(vec2 deltaPos);
        void changeWidth(float deltaWidth);
        void changeHeight(float deltaHeight);

        // --------------------------------------------------------
        // Setters
        // --------------------------------------------------------

        // Position
        void setPos(vec2 pos);
        void setPosX(float x);
        void setPosY(float y);

        // Movement Setters (add/sub to current value)
        void move(vec2 offset);
        void moveX(float x);
        void moveY(float y);

        // Size
        void setSize(vec2 size);
        void setSizeX(float x);
        void setSizeY(float y);

        // Change Functions
        void update(float deltaTime);

        // Color
        virtual void setColor(color color);

        virtual void setColor(vec4 color);

        virtual void setColor(vec3 color);
        void setRed(float r);
        void setGreen(float g);
        void setBlue(float b);
        void setOpacity(float a);

        // --------------------------------------------------------
        // Drawing functions
        // --------------------------------------------------------

        /// @brief Sets the uniform variables from members, and calls the virtual draw function
        virtual void setUniforms() const;

        /// @brief Pure virtual function to draw the shape.
        virtual void draw() const = 0;

        // --------------------------------------------------------
        // Collision functions
        // --------------------------------------------------------
        //Shape overlapping
        virtual bool isOverlapping(Shape const& other) const = 0;
        //Mouse overlapping
        virtual bool isMouseOverlaping(const vec2& point) const;

protected:
        /// @brief Shader used to draw all abstract shapes.
        /// @note This will need to be a pointer for custom shaders.
        Shader & shader;

        /// @brief The position of the shape
        vec2 pos;

        vec2 size;

        vec2 velocity;

        /// @brief The VAO of the shape
        struct color color;

        /// @brief The Vertex Array Object, Vertex Buffer Object, and Element Buffer Object of the shape.
        unsigned int VAO, VBO, EBO;

        /// @brief The vertices of the shape
        vector<float> vertices;

        /// @brief The indices of the shape
        vector<unsigned int> indices;
};

#endif //GRAPHICS_SHAPE_H

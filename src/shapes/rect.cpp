#include "rect.h"
#include "circle.h"

Rect::Rect(Shader & shader, vec2 pos, vec2 size, struct color color) : Shape(shader, pos, size, color) {
    initVectors();
    initVAO();
    initVBO();
    initEBO();
}

Rect::Rect(Shader &shader, vec2 pos, float width, struct color color)
    : Rect(shader, pos, vec2(width, width), color) {}

Rect::Rect(Shader &shader, vec2 pos, float width, vec4 color)
    : Rect(shader, pos, vec2(width, width), color) {}

Rect::Rect(Rect const& other) : Shape(other) {
    initVectors();
    initVAO();
    initVBO();
    initEBO();
}

Rect::~Rect() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Rect::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Rect::initVectors() {
    this->vertices.insert(vertices.end(), {
        -0.5f, 0.5f,   // Top left
        0.5f, 0.5f,   // Top right
        -0.5f, -0.5f,  // Bottom left
        0.5f, -0.5f   // Bottom right
    });

    this->indices.insert(indices.end(), {
        0, 1, 2, // First triangle
        1, 2, 3  // Second triangle
    });
}
// Overridden Getters from Shape
float Rect::getLeft() const        { return pos.x - (size.x / 2); }
float Rect::getRight() const       { return pos.x + (size.x / 2); }
float Rect::getTop() const         { return pos.y + (size.y / 2); }
float Rect::getBottom() const      { return pos.y - (size.y / 2); }

//Checks if Rect overlaps Shape (Circle)
bool Rect::isOverlapping(const Shape& c) const {
    if(const Circle* circl = dynamic_cast<const Circle*>(&c)) {
        return circl->isOverlapping(*this);
    }
    return false;
}

// Checks if Rect is overlapping Circle (based on circle class logic)
bool Rect::isOverlapping(const Circle& circle) const {
    return circle.isOverlapping(*this);
}
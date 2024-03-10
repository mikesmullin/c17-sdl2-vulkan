#version 450

// vertex attrs
layout(location = 0) in vec2 xy;

// instanced attrs
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 rot;
layout(location = 3) in vec3 scale;
layout(location = 4) in uint texId;

layout(binding = 0) uniform UBO1 {
    mat4 proj;
    mat4 view;
    vec2 user1;
    vec2 user2;
} ubo1;

layout(location = 0) out vec2 fragTexCoord;

// generate model matrix from position, rotation, and scale
mat4 generateModelMatrix(vec3 position, vec3 rotation, vec3 scale) {
    // Rotation matrices for each axis
    mat4 rotX = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, cos(rotation.x), -sin(rotation.x), 0.0,
        0.0, sin(rotation.x), cos(rotation.x), 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    mat4 rotY = mat4(
        cos(rotation.y), 0.0, sin(rotation.y), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(rotation.y), 0.0, cos(rotation.y), 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    mat4 rotZ = mat4(
        cos(rotation.z), -sin(rotation.z), 0.0, 0.0,
        sin(rotation.z), cos(rotation.z), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    // Combine rotation matrices
    mat4 rotationMatrix = rotX * rotY * rotZ;

    // Scale matrix
    mat4 scaleMatrix = mat4(
        scale.x, 0.0, 0.0, 0.0,
        0.0, scale.y, 0.0, 0.0,
        0.0, 0.0, scale.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    // Translation matrix
    mat4 translationMatrix = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        position.x, position.y, position.z, 1.0
    );

    // Combine translation, rotation, and scale
    mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

    return modelMatrix;
}

vec2 TEXTURE_WH = vec2(800,900);
float pixelsToUnitsX(uint pixels) {
    return pixels / TEXTURE_WH.x;
}
float pixelsToUnitsY(uint pixels) {
    return pixels / TEXTURE_WH.y;
}

uint GLYPH_X = 260;
uint GLYPH_Y = 822;
uint GLYPH_W = 4;
uint GLYPH_H = 6;

void main() {
    if (xy.x == 0.5 && xy.y == -0.5) { // top-left
        gl_Position = vec4(0.0-0.5, 0.0-0.5, 1.0, 1.0);
    }
    else if (xy.x == -0.5 && xy.y == -0.5) { // top-right
        gl_Position = vec4(1.0-0.5, 0.0-0.5, 1.0, 1.0);
    }
    else if (xy.x == -0.5 && xy.y == 0.5) { // bottom-right
        gl_Position = vec4(1.0-0.5, 1.0-0.5, 1.0, 1.0);
    }
    else if (xy.x == 0.5 && xy.y == 0.5) { // bottom-left
        gl_Position = vec4(0.0-0.5, 1.0-0.5, 1.0, 1.0);
    }
}
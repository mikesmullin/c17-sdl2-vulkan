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

uint ATLAS_W = 2632;
uint ATLAS_H = 1721;

vec2 TEXTURE_WH = vec2(ATLAS_W,ATLAS_H);
float pixelsToUnitsX(uint pixels) {
    return pixels / TEXTURE_WH.x;
}
float pixelsToUnitsY(uint pixels) {
    return pixels / TEXTURE_WH.y;
}

uint SPRITE_X = 0;
uint SPRITE_Y = 690;
uint SPRITE_W = 300;
uint SPRITE_H = 450;
uint SPRITE_IDX_OFFSET = 3;
uint SPRITE_ROW_LEN = 8;
uint WOOD_WALL_W = 350;
uint WOOD_WALL_H = 420;

void main() {
    mat4 model = generateModelMatrix(pos, rot, scale);
    gl_Position = ubo1.proj * ubo1.view * model * vec4(-xy.x, xy.y, 0.0, 1.0);

    // hard-coded map of texId to uvwh coords in texture atlas
    vec4 uvwh;
    if (0 == texId) { // background 0x0 1574x684
        uvwh = vec4(pixelsToUnitsX(0),pixelsToUnitsY(0),pixelsToUnitsX(1574),pixelsToUnitsY(684));
    }
    else if (1 == texId) { // wood-wall 1 1580x0 350x420
        uvwh = vec4(pixelsToUnitsX(1580),pixelsToUnitsY(0),pixelsToUnitsX(WOOD_WALL_W),pixelsToUnitsY(WOOD_WALL_H));
    }
    else if (2 == texId) { // wood-wall 1 1580x0 350x420
        uvwh = vec4(pixelsToUnitsX(1580 + WOOD_WALL_W),pixelsToUnitsY(0),pixelsToUnitsX(WOOD_WALL_W),pixelsToUnitsY(WOOD_WALL_H));
    }

    // sprites
    else if (texId >= SPRITE_IDX_OFFSET) {
        uint x = (texId - SPRITE_IDX_OFFSET);
        uint y = (x / SPRITE_ROW_LEN);
        x = x % SPRITE_ROW_LEN;
        uvwh = vec4(
            pixelsToUnitsX(SPRITE_X + (SPRITE_W * x)),
            pixelsToUnitsY(SPRITE_Y + (SPRITE_H * y)),
            pixelsToUnitsX(SPRITE_W),
            pixelsToUnitsY(SPRITE_H));
    }

    if (xy.x == 0.5 && xy.y == -0.5) {
        fragTexCoord = vec2(uvwh.x, uvwh.y); // top-left
    }
    else if (xy.x == -0.5 && xy.y == -0.5) {
        fragTexCoord = vec2(uvwh.x+uvwh.z, uvwh.y); // top-right
    }
    else if (xy.x == -0.5 && xy.y == 0.5) {
        fragTexCoord = vec2(uvwh.x+uvwh.z, uvwh.y+uvwh.w); // bottom-right
    }
    else if (xy.x == 0.5 && xy.y == 0.5) {
        fragTexCoord = vec2(uvwh.x, uvwh.y+uvwh.w); // bottom-left
    }
}
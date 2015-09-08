#version 150 core

in vec3 outCol;
out vec4 outColor;
void main() {
    outColor = vec4(outCol.r, outCol.g, outCol.b, 1.0);
}

#version 150 core

uniform sampler2D ParticleTex;

in vec3 outCol;
out vec4 outColor;

void main() {
    outColor = texture(ParticleTex, gl_PointCoord);
    outColor = vec4(outCol, outColor.a);
    //outColor = vec4(outCol.r, outCol.g, outCol.b, 1.0);
}

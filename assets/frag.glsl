#version 150 core

uniform sampler2D ParticleTex;
uniform sampler2D PotionTex;

in vec2 outPos;
in vec3 outCol;

out vec4 outColor;

void main() {
    vec2 texCoord;
    texCoord.x = (outPos.x  + 1.0f) / 2.0f;
    texCoord.y = (outPos.y + 1.0f) / 2.0f;

    vec4 logoCol = texture(PotionTex, texCoord);
    
    outColor = texture(ParticleTex, gl_PointCoord);
    //outColor = vec4(outCol, logoCol.a);
    
    outColor.a *= logoCol.a;
    
    outColor = vec4(outCol.r, outCol.g, outCol.b, outColor.a);
}

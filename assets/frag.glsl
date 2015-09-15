#version 150 core

uniform sampler2D ParticleTex;
uniform sampler2D BackgroundTex;

in vec2 outPos;
in vec3 outCol;

in vec3 dirCol;

out vec4 outColor;

//******************************************
//  lerp formula
//******************************************

float lerp(float start, float stop, float amt)
{
    if (start < stop) {
        return start + ((stop-start) * amt);
    }
//    else {
//        return stop + ((start-stop) * amt);
//    }
}

//******************************************
//  main
//******************************************

void main() {
    vec2 texCoord;
    texCoord.x = (outPos.x  + 1.0f) / 2.0f;
    texCoord.y = (outPos.y + 1.0f) / 2.0f;

    vec4 logoCol = texture(BackgroundTex, texCoord);
    
    outColor = texture(ParticleTex, gl_PointCoord);
    outColor = vec4(dirCol, outColor.a);
    
    vec3 potionBlue = vec3(72.0f/255.0f, 146.0f/255.0f, 207.0f/255.0f);
    
    if (logoCol.a > 0.0f) {
        // lerp to Potion blue
        outColor.r = lerp(outColor.r, potionBlue.r, logoCol.a);
        outColor.g = lerp(outColor.g, potionBlue.g, logoCol.a);
        outColor.b = lerp(outColor.b, potionBlue.b, logoCol.a);
        
        //outColor.a *= 1.0 - logoCol.a;
    }
//    else {
//        outColor = vec4(outCol.r, outCol.g, outCol.b, outColor.a);
//    }
}

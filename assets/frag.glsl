#version 150 core

uniform sampler2D ParticleTex;
uniform sampler2D BackgroundTex;

in vec2 vsPos;
in vec3 vsCol;

in vec3 vsDirCol;

out vec4 fsColor;

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
    texCoord.x = (vsPos.x  + 1.0f) / 2.0f;
    texCoord.y = (vsPos.y + 1.0f) / 2.0f;

    vec4 logoCol = texture(BackgroundTex, texCoord);
    
    fsColor = texture(ParticleTex, gl_PointCoord);
    fsColor = vec4(vsDirCol, fsColor.a);
    
    vec3 potionBlue = vec3(72.0f/255.0f, 146.0f/255.0f, 207.0f/255.0f);
    
    if (logoCol.a > 0.0f) {
        // lerp to Potion blue
        fsColor.r = lerp(fsColor.r, potionBlue.r, logoCol.a);
        fsColor.g = lerp(fsColor.g, potionBlue.g, logoCol.a);
        fsColor.b = lerp(fsColor.b, potionBlue.b, logoCol.a);
        
        //outColor.a *= 1.0 - logoCol.a;
    }
//    else {
//        outColor = vec4(outCol.r, outCol.g, outCol.b, outColor.a);
//    }
}

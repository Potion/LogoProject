#version 150 core

uniform sampler2D ParticleTex;
//uniform sampler2D BackgroundTex;
uniform float u_baseOpacity;

in vec2 vsPos;
in vec3 vsFragCol;
in float vsDecay;

out vec4 fsColor;

//******************************************
//  lerp formula
//******************************************

float lerp(float start, float stop, float amt)
{
//    if (start < stop) {
        return start + ((stop-start) * amt);
//    }
//    else {
//        return stop + ((start-stop) * amt);
//    }
}

//******************************************
//  main
//******************************************

void main() {
//    vsBaseCol = outBaseCol;
    vec2 texCoord;
    texCoord.x = (vsPos.x + 1.0f) / 2.0f;
    texCoord.y = (vsPos.y + 1.0f) / 2.0f;

    //vec4 bgCol = texture(BackgroundTex, texCoord);
    
    fsColor = texture(ParticleTex, gl_PointCoord);
    fsColor = vec4(vsFragCol, fsColor.a * vsDecay);
    
    //vec3 potionBlue = vec3(72.0f/255.0f, 146.0f/255.0f, 207.0f/255.0f);
    
//    if (bgCol.a > 0.0f) {
//        // lerp to Potion blue
//        fsColor.r = lerp(fsColor.r, potionBlue.r, bgCol.a * vsDecay);
//        fsColor.g = lerp(fsColor.g, potionBlue.g, bgCol.a * vsDecay);
//        fsColor.b = lerp(fsColor.b, potionBlue.b, bgCol.a * vsDecay);
//        
//        //outColor.a *= 1.0 - logoCol.a;
//    }
//    else {
        fsColor = vec4(vsFragCol.r, vsFragCol.g, vsFragCol.b, fsColor.a * u_baseOpacity);
//    }
    
}

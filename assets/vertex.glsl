//
//  vertex.glsl
//  LogoProject
//
//  Created by Jennifer Presto on 9/7/15.
//
//

#version 150 core

uniform vec2 mousePos;

in vec2 inPos;
in vec2 inVel;
in vec3 inCol;

out vec2 outPos;
out vec2 outVel;
out vec3 outCol;

// generate a pseudo random direction based on particle's current position
// http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float getRandomFloat(vec2 currentPos) {
    return fract(sin(dot(currentPos.xy, vec2(12.9898, 78.233))) * 43758.5453);
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt = dot(currentPos.xy, vec2(a, b));
    float sn = mod(dt, 3.14);
    return fract(sin(sn) * c);
}

vec2 getDir(float value)
{
    vec2 vel;
    value *= 3.1415926535897932384626433832795 * 2.0f;
    //vel.x = cos(randFloat) * 0.13;
    //vel.y = sin(randFloat) * 0.13;
    vel.x = cos(value);
    vel.y = sin(value);
    return vel;
}


float mapFloat(float value, float inputMin, float inputMax, float outputMin, float outputMax)
{
    const float Epsilon = 0.0000001;
    if (abs(inputMin - inputMax) < Epsilon) {
        return outputMin;
    }
    
    float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
    // clamp result
    if (outVal > outputMax) {
        outVal = outputMax;
    }
    if (outVal < outputMin) {
        outVal = outputMin;
    }
    return outVal;
}

void main() {
    vec2 gravity = vec2(0.0, -0.0005);
    float max = 0.05;
    float maxSquared = max * max;
    
    outPos = inPos;
    outVel = inVel;
    outVel += gravity;
    
    // limit speed
    float speedSquared = dot(outVel, outVel);
    if (speedSquared > maxSquared) {
        vec2 norm = normalize(outVel);
        outVel *= max;
    }
    
    outPos += outVel;
    
    // keep in frame
    if (inPos.y < -1.0) {
        outPos.x = 0.0;
        outPos.y = 0.0;
        
        // reset velocity and position
        
        // use position to generate random number for velocity direction
        float newFloat = getRandomFloat(inPos);
        
        // use a different vector to generate a random number for velocity length
        vec2 altVec;
        if (inPos.x != 0.0 && inPos.y != 0.0) {
            altVec = inPos + vec2(1.0/inPos.x, 1.0/inPos.y);
        } else {
            altVec = inPos;
        }
        
        float newSpeed = getRandomFloat(altVec);
        newSpeed = mapFloat(newSpeed, 0.0, 1.0, 0.06, 0.15); // make min output number smaller; why does that happen?
        
        outVel = getDir(newFloat);
        outVel *= newSpeed;
        outPos = mousePos;
    }
    
    outCol = inCol;
    gl_Position = vec4(outPos, 0.0, 1.0);
}
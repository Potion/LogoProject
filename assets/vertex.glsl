#version 150 core

const int numNewPositions = 100;

uniform vec2 mousePos;
uniform vec2 newPositions[numNewPositions];

in vec2 inPos;
in vec2 inVel;
in vec3 inCol;

out vec2 outPos;
out vec2 outVel;
out vec3 outCol;


//******************************************
//  generate a pseudo random direction based on particle's current position
//  http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
//  generates number between 0.0 and 1.0
//******************************************
float getRandomFloat(vec2 currentPos) {
    //return fract(sin(dot(currentPos.xy, vec2(12.9898, 78.233))) * 43758.5453);
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt = dot(currentPos.xy, vec2(a, b));
    float sn = mod(dt, 3.14);
    return fract(sin(sn) * c);
}

//******************************************
//  generate a direction based on a value between 0.0 and 1.0
//******************************************
vec2 getDir(float value)
{
    vec2 vel;
    value *= (3.1415926535897932384626433832795 * 2.0f);
    //vel.x = cos(randFloat) * 0.13;
    //vel.y = sin(randFloat) * 0.13;
    vel.x = cos(value);
    vel.y = sin(value);
    return vel;
}

//******************************************
//  map a floating point within a range
//******************************************
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


//******************************************
//  main
//******************************************
void main() {
    
    vec2 gravity = vec2(0.0, -0.0005);
    float max = 0.05;
    float maxSquared = max * max;
    
    //int number = numNewPositions;
    
    outPos = inPos;
    outVel = inVel;
    outVel += gravity;
    
    //outCol = vec3(0.0, 1.0, 0.0);
    
    // limit speed
    float speedSquared = dot(outVel, outVel);
    if (speedSquared > maxSquared) {
        vec2 norm = normalize(outVel);
        outVel *= max;
    }
    
    outPos += outVel;
    
    //  keep in frame
    if (outPos.y < -1.0) {
        //  reset velocity
        //  use last position to generate random number for velocity direction
        vec2 randomSeed = inPos + vec2(inCol.r, inCol.g);
        vec2 randomSeed2 = inPos + vec2(inCol.g, inCol.b);

        float newVelSeed = getRandomFloat(randomSeed);

        float newSpeed = getRandomFloat(randomSeed2);
        newSpeed = mapFloat(newSpeed, 0.0, 1.0, 0.06, 0.15); // make min output number smaller; why does that happen?
        
        outVel = getDir(newVelSeed);
        outVel *= newSpeed;

        //  reset position
        //  pick random new position within array of white pixels
        float topLimit = float(numNewPositions) - 0.1;
        float newNum = mapFloat(newVelSeed, 0.0, 1.0, 0.0, topLimit);
        int newIndex = int(newNum);
        outPos = newPositions[newIndex];
        //outPos = mousePos;
        //outPos = vec2(0.33, -1.73076);
    }
    
    outCol = inCol;
    gl_PointSize = 15.0f;
    gl_Position = vec4(outPos, 0.0, 1.0);
}

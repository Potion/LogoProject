#version 150 core

const int numNewPositions = 100;
const float MATH_PI = 3.1415926535897932384626433832795;

uniform vec2 mousePos;
uniform vec2 newPositions[numNewPositions];

in vec2 inPos;
in vec2 inVel;
in vec3 inCol;

in float inSize;

out vec2 outPos;
out vec2 outVel;
out vec3 outCol;

out float outSize;

out vec3 dirCol;


//******************************************
//  generate a pseudo random direction based on particle's current position
//  http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
//  generates number between 0.0 and 1.0
//******************************************
float getRandomFloat(vec2 currentPos)
{
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
    value *= (MATH_PI * 173.5f);
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
//  convert RGB to HSV
//******************************************
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

//******************************************
//  convert HSV to RGB
//******************************************
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

//******************************************
//  generate HSV color based on direction
//******************************************
vec3 getDirBasedColor(vec2 dir)
{
    vec2 normalizedVel = normalize(dir);
    //vec2 vertical = vec2(0.0, 1.0);
    float angle = atan(normalizedVel.y, normalizedVel.x);
    angle = mapFloat(angle, -MATH_PI, MATH_PI, 0.0f, 1.0f);
    vec3 color = hsv2rgb(vec3(angle, 1.0f, 1.0f));
    return color;
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
        newSpeed = mapFloat(newSpeed, 0.0, 1.0, 0.055, 0.15); // make min output number smaller; why does that happen?
        
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
    
    dirCol = getDirBasedColor(outVel);
    
    outCol = inCol;
    outSize = inSize;
    gl_PointSize = outSize;
    gl_Position = vec4(outPos, 0.0, 1.0);
}

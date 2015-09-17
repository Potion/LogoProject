#version 150 core

const int numNewPositions = 100;
const float MATH_PI = 3.1415926535897932384626433832795;
//const float lifespan = 1.5f;

//  hard-code the dots
const float scalar = 1.5f;

const vec2 dot0pos = vec2(-0.08125001 * scalar, 0.50416666 * .666666f * scalar);
const vec2 dot1pos = vec2(0.19062495 * scalar, -0.13333333 * .666666f * scalar);
const vec2 dot2pos = vec2(-0.15625 * scalar, -0.33749998 * .666666f * scalar);
const vec2 dot3pos = vec2(-0.021875024 * scalar, -0.6291667 * .666666f * scalar);

const float dot0radius = 0.2890625 * scalar;
const float dot1radius = 0.1625 * scalar;
const float dot2radius = 0.0875 * scalar;
const float dot3radius = 0.053125 * scalar;


uniform vec2 u_mousePos;
uniform vec2 u_newPositions[numNewPositions];
uniform float u_deltaTime; // in seconds
uniform float u_time;
uniform float u_hue;
uniform float u_gravityPull;
uniform bool u_shrinking;
uniform float u_particleLife;
uniform float u_slipperiness;

//uniform sampler2D BackgroundTex; // doesn't work in VS

//  vertex array (for ping-ponging)
in vec2 inPos;
in vec2 inVel;
in vec3 inBaseCol;
in float inCurrentHue;
in float inSize;
in float inBornTime;

out vec2 vsPos;
out vec2 vsVel;
out vec3 vsBaseCol;
out float vsCurrentHue;
out float vsSize;
out float vsBornTime;

//  passed only to fragment shader
out float vsDecay;
out vec3 vsFragCol;

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
    float currentTime = u_time;
    
    vec2 gravity = u_deltaTime * vec2(0.0, -u_gravityPull);
    //vec2 gravity = vec2(0.0f, -0.0005);
    float max = 0.05;
    float maxSquared = max * max;
    
    //int number = numNewPositions;
    
    //  set the ping-ponging variables
    vsPos = inPos;
    vsVel = inVel;
    vsBornTime = inBornTime;
    vsCurrentHue = inCurrentHue;
    
    ////////*******NOT WORKING********
    ////////  check to see if we're over the logo
    //vec2 texCoord;
    //texCoord.x = (vsPos.x + 1.0f) / 2.0f;
    //texCoord.y = (vsPos.y + 1.0f) / 2.0f;
    //
    //vec4 bgColor = textureLod(BackgroundTex, texCoord, 0.0);
    ////vsBGAlpha = bgColor.a;
    ////
    //////vec3 overLogo = texture(BackgroundTex, vsPos).xyz;
    //////  slow down if we're on the logo
    //if (bgColor.g > 0.5) {
    //    vsVel *= 0.5f;
    //} else {
    //    vsVel += gravity;
    //}
    ////////*******NOT WORKING********

    //**********************************
    //  STICKING ON THE DOTS
    //  see if close to the dots
    
    //  adjust for 4x3 aspect ratio of window
    vec2 hypotheticalPos;
    hypotheticalPos.x = vsPos.x * 1.33333333;
    hypotheticalPos.y = vsPos.y;
    
    bool isStuck = false;
    if (distance(hypotheticalPos, dot0pos) < dot0radius) {
        isStuck = true;
        vsVel *= u_slipperiness;
    } else if (distance(hypotheticalPos, dot1pos) < dot1radius) {
        isStuck = true;
        vsVel *= u_slipperiness;
    } else if (distance(hypotheticalPos, dot2pos) < dot2radius) {
        isStuck = true;
        vsVel *= u_slipperiness;
    } else if (distance(hypotheticalPos, dot3pos) < dot3radius) {
        isStuck = true;
        vsVel *= u_slipperiness;
    } else {
        vsVel += gravity;
    }

    // limit speed
    float speedSquared = dot(vsVel, vsVel);
    if (speedSquared > maxSquared) {
        vec2 norm = normalize(vsVel);
        vsVel *= max;
    }
    //***************************************
    
    //vsPos += vsVel;
    vsPos += vsVel * u_deltaTime * 59.0f;
    
    //  reset particles when offscreen or dead
    //  each particle has slightly different lifespan
    float lifespan = u_particleLife + (inBaseCol.b * 0.5); // use only as random value to vary lifespans
    
    //**********************************
    //  STICKING ON THE DOTS
    if (isStuck) lifespan += 4.0f;
    //**********************************
    
    float lifetime = u_time - inBornTime;
    
    if (vsPos.y < -1.0 || lifetime > lifespan) {
        //  reset velocity
        //  use last position to generate random number for velocity direction
        vec2 randomSeed = inPos + vec2(inBaseCol.r, inBaseCol.g);
        vec2 randomSeed2 = inPos + vec2(inBaseCol.g, inBaseCol.b);

        float newVelSeed = getRandomFloat(randomSeed);

        float newSpeed = getRandomFloat(randomSeed2);
        newSpeed = mapFloat(newSpeed, 0.0, 1.0, 0.055, 0.15); // make min output number smaller; why does that happen?
        
        vsVel = getDir(newVelSeed);
        vsVel *= newSpeed;

        //  reset position
        //  pick random new position within array of white pixels
        float topLimit = float(numNewPositions) - 0.1;
        float newNum = mapFloat(newVelSeed, 0.0, 1.0, 0.0, topLimit);
        int newIndex = int(newNum);
        vsPos = u_newPositions[newIndex];
        //vsPos = u_mousePos;
        
        //  new color
        vsCurrentHue = u_hue;
        
        //  reset life
        vsBornTime = u_time;
    }
    
    //dirCol = getDirBasedColor(outVel);
    //vsDirCol = hsv2rgb(vec3(inBaseCol.r, 1.0, 1.0));
    
    vsDecay = 1.0 - (lifetime / lifespan);
    vsFragCol = hsv2rgb(vec3(vsCurrentHue, 1.0f, 1.0f));
    
    vsBaseCol = inBaseCol; // recycle base color
    vsSize = inSize; // recycle size
    if (u_shrinking) {
        gl_PointSize = vsSize * vsDecay;
    } else {
        gl_PointSize = vsSize;
    }
    gl_Position = vec4(vsPos, 0.0, 1.0);
}

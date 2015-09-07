//
//  frag.glsl
//  LogoProject
//
//  Created by Jennifer Presto on 9/7/15.
//
//

#version 150 core

in vec3 outCol;
out vec4 outColor;
void main() {
    outColor = vec4(outCol.r, outCol.g, outCol.b, 1.0);
}

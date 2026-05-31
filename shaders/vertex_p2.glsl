#version 410 core
layout(location = 0) in vec3 aPos; // the position variable has attribute position 0
layout(location = 1) in vec3 aColor; // the color variable has attribute position 1

out vec3 outColor; // output a color to the fragment shader

void main()
{
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
        gl_PointSize = 15.0;
        outColor = aColor; // set outColor to the input color we got from the vertex data
}

#version 410 core
layout(location = 0) in vec3 aPos; // the position variable has attribute position 0
layout(location = 1) in vec3 aColor; // the color variable has attribute position 1

out vec3 outColor; // output a color to the fragment shader

uniform mat4 u_proj_view;
uniform mat4 u_model;

void main()
{
        gl_Position = u_proj_view * u_model * vec4(aPos, 1.0);
        // gl_PointSize = gl_Position.w * 1.0; // set the point size based on the distance from the camera
        outColor = aColor; // set outColor to the input color we got from the vertex data
}

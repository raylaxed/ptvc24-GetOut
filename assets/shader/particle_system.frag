#version 430 core

// Input from vertex shader
in vec4 color;

// Output to the framebuffer
out vec4 fragColor;

void main() {
    // Set the fragment color to the input color
    fragColor = color;
}

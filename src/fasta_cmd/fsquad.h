static const struct
{
    float x, y;
    float r, g, b;
} vertices[4] =
{
    { -1.f, -1.f, 1.f, 0.f, 0.f },
    {  1.f, -1.f, 0.f, 1.f, 0.f },
    {  1.f,  1.f, 0.f, 0.f, 1.f },
    { -1.f,  1.f, 1.f, 1.f, 1.f }
};
 
static const char* vertex_shader_text = R"(
#version 330 core

out vec2 uv;

void main() {
    float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u); 
    float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u); 

    gl_Position = vec4(-1.0f + x*2.0f, -1.0f+y*2.0f, 0.0f, 1.0f);
    uv = vec2(x, y);
}
)";
 
static const char* fragment_shader_text = R"(
#version 330 core

in vec2 uv;
out vec4 fragColor;

void main() {
    fragColor = vec4(uv, 0.0, 1.0);
}
)";
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
f16mat3x2 u = f16mat3x2(f16vec2(1.0hf, 2.0hf), f16vec2(3.0hf, 4.0hf), f16vec2(5.0hf, 6.0hf));
void f() {
  mat3x2 v = mat3x2(u);
}

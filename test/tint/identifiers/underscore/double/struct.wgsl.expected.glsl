#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct a {
  int b;
};

struct tint_symbol {
  int tint_symbol_1;
};

void f() {
  tint_symbol c = tint_symbol(0);
  int d = c.tint_symbol_1;
}


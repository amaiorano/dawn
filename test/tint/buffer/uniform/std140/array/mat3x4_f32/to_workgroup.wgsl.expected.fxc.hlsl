cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[12];
};
groupshared float3x4 w[4];

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

float3x4 tint_symbol_3(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
}

typedef float3x4 tint_symbol_2_ret[4];
tint_symbol_2_ret tint_symbol_2(uint4 buffer[12], uint offset) {
  float3x4 arr[4] = (float3x4[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = tint_symbol_3(buffer, (offset + (i_1 * 48u)));
    }
  }
  return arr;
}

void f_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      const uint i = idx;
      w[i] = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  w = tint_symbol_2(u, 0u);
  w[1] = tint_symbol_3(u, 96u);
  w[1][0] = asfloat(u[1]).ywxz;
  w[1][0].x = asfloat(u[1].x);
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
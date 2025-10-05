cbuffer stuff : register(b0) {
    float2 screen_size;
};
Texture2D atlas : register(t0);
SamplerState samp : register(s0);

struct VsInput {
    float2 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PsInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PsInput vs_main(VsInput input) {
    PsInput output;

    float2 screen_pos = 2.0f * (input.position / screen_size) - 1.0f;
    output.position = float4(screen_pos.x, -screen_pos.y, 0.0f, 1.0f);
    output.uv = input.uv;

    return output;
}

float4 ps_main(PsInput input) : SV_TARGET {
    float4 glyph_sample = atlas.Sample(samp, input.uv);
    return float4(0.4f, 0.7f, 0.0f, glyph_sample.x);
    //return atlas.Sample(samp, input.uv);
}
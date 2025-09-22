cbuffer stuff : register(b0) {
    row_major float4x4 view;
    row_major float4x4 projection;
    row_major float4x4 world;
};

struct VsInput {
    float4 position : POSITION;
    float4 color : COLOR;
};

struct PsInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PsInput vs_main(VsInput input) {
    PsInput output;
   
    output.position = mul(mul(projection, view), mul(world, input.position));
    output.color = input.color;

    return output;
}

float4 ps_main(PsInput input) : SV_TARGET {
    return input.color;
}
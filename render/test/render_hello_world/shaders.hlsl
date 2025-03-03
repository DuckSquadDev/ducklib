cbuffer stuff : register(b0) {
    float4x4 view;
    float4x4 projection;
};

struct VsInput {
    float3 position : POSITION;
};

struct VsOutput {
    float4 position : SV_POSITION;
};

VsOutput vs_main(VsInput input) {
    VsOutput output;
   
    float4 view_pos = mul(float4(input.position, 1.0), view);
    output.position = mul(view_pos, projection);

    return output;
}

struct PsInput {
    float4 position : SV_POSITION;
};

float4 ps_main(PsInput input) : SV_TARGET {
    return float4(1.0f, 1.0f, 0.0f, 1.0f);
}
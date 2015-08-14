
cbuffer cbChangeOnResize : register(b1)
{
};

cbuffer cbPerFrame : register(b2)
{
};

cbuffer cbPerObject: register(b3)
{
	float4x4 lightWVP;
	bool isInstancing;
	float3 padding;
};

struct VS_INPUT
{
	float3 PosL : POSITION;
	float3 NorL : NORMAL;
	row_major float4x4 World : WORLD;
	uint InstanceId : SV_InstanceID;
};

struct PS_INPUT
{
	float4 PosH : SV_POSITION;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	float3 posL = input.PosL; 
	if (isInstancing)
		posL = mul(float4(input.PosL, 1.0f), input.World).xyz;
	output.PosH = mul(float4(posL, 1.0f), lightWVP);
	return output;
}

void PS(PS_INPUT input) 
{
}

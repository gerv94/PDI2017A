struct a2v {
	float4 Position : POSITION;
};
struct v2p {
	float4 Position : POSITION;
};
void main(in a2v IN, out v2p OUT, uniform float4x4 ModelViewMatrix)
{
	OUT.Position = mul(IN.Position, ModelViewMatrix);
}
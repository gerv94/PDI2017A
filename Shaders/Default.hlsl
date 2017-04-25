Texture2D<float4> Input;
RWTexture2D<float4> Output;
[numthreads(16, 16,	1)] // Se comparte entre 16x16 hilos = 256 hilos.  //PROCESSOR POOL
void main(uint3 gid:SV_DispatchThreadID, uint3 lid:SV_GroupThreadID)
{ // Lid : Local ID
	//Output[gid.xy] = float4((int)gid.x - 100/*0.2*/, (int)gid.y - 100/*0.2*/, cos(gid.x*gid.y / 1000.0f)/*0/*0.6*/, 0) / float4(200, 200, 1, 1);
	/*float2 rot = float2(
	id.x*cos(3.141592 / 4) - id.y*sin(3.141592 / 4),
	id.x*sin(3.141592 / 4) + id.y*sin(3.141592 / 4));
	Output[gid.xy] = Input[int2(rot.xy)];*/
	//Output[gid.xy] = Input[gid.xy + int2(1, 0)] - Input[gid.xy] + 0.5;
	Output[gid.xy] = float4(Input[gid.xy].xy, 0.5, 1);
	/*float4 A,B;
	float3 C;
	A = B;
	A = C;//Esto no se puede
	A.xyz = C;//Esto si se puede
	A.zyx = C;
	A.xyzw = C.xxxy;//Permutacion y asignacion Difución
	A = (B.wzyx * 4) + 3
	A.rgba = B.xyzw;*/
}
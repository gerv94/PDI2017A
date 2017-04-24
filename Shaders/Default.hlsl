
RWTexture2D<float4> Output;

[numthreads(16, 16,	1)] // Se comparte entre 16x16 hilos = 256 hilos.  //PROCESSOR POOL
void main(uint3 gid:SV_DispatchThreadID, uint3 lid:SV_GroupThreadID)
{ // Lid : Local ID
	Output[gid.xy] = float4(
		(int)gid.x -100/*0.2*/, 
		(int)gid.y -100/*0.2*/, 
		cos(gid.x*gid.y/1000.0f)/*0/*0.6*/, 
		0) / float4(200, 200, 1, 1);
}

[numthreads(16,16,1)]
void Main() 
{

}
// Imprimir color azul
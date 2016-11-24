#include <iostream>
#include <cuda.h>
#define vect_len 16
using namespace std;

const int blocksize = 16;

// __global__ decorator signifies a kernel that can be called from the host
__global__ void vect_add(int *a, int *b)
{
	a[threadIdx.x] += b[threadIdx.x];
}

int main(){
	const int vect_size = vect_len*sizeof(int);
	int * vect1=(int*)malloc(vect_size);
	int * vect2=(int*)malloc(vect_size);
	int * result=(int*)malloc(vect_size);
 	bool flag;
	for(int i = 0; i < vect_len; i++)
	{
		vect1[i] = i;
		vect2[i] = 2 * i;
	}
	int *ad, *bd;
	// initialize device memory
	cudaMalloc( (void**)&ad, vect_size );
	cudaMalloc( (void**)&bd, vect_size );
	// copy data to device
	cudaMemcpy( ad, vect1, vect_size, cudaMemcpyHostToDevice );
	cudaMemcpy( bd, vect2, vect_size, cudaMemcpyHostToDevice );
	// setup block and grid size	
	dim3 dimBlock( blocksize, 1, 1);
	dim3 dimGrid( vect_len/blocksize, 1 , 1);
	// call device kernel
	//vect_add<<<dimGrid, dimBlock>>>(ad, bd);
	vect_add<<<dimGrid, dimBlock>>>(ad, bd);
	cudaMemcpy( result, ad, vect_size, cudaMemcpyDeviceToHost );

	//Verify
	flag = true;
	for(int i = 0; i < vect_len; i++)
	{
		if(result[i] != vect1[i] + vect2[i])
		{
			cout << "Verification fail at " << i << endl;
			flag = false;
			break;
		}
	}
	if(flag)
		cout << "Verification passes." <<endl;
	// free device memory
	cudaFree( ad );
	cudaFree( bd );
	free(vect1);
	free(vect2);
	free(result);
	return EXIT_SUCCESS;
}



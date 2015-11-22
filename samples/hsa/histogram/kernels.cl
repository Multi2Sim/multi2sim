/*!
 * Calculate a histogram of an image linearized in input with size pixels
 * one work group, and the number of work items is the number of output points
 */

__kernel void HIST(
  __global uint * input,
  __global volatile uint * output,
	uint colors,
  uint size){
#if 1
    uint tid = get_global_id(0);
    uint gsize = get_global_size(0);

		if (tid >= size)
		{
			return;
		}
    uint priv_hist[256];
    uint i=0;
    
    for (i=0; i<colors; i++) 
    {
			priv_hist[i] = 0;
    }
  	

    //Private histogram calculation
    uint index = tid;
    while (index < size)
    {
			uint color = input[index];
			priv_hist[color]++;
			index+=gsize;
    }
    	
		//Barrier in case priv_hist is mapped to global memory
		mem_fence(CLK_GLOBAL_MEM_FENCE);

    //Copy to global memory
	  for (i=0; i<colors; i++)
	  {
				if (priv_hist[i] > 0)
				{
					atom_add(&output[i], priv_hist[i]);
				}
	  }

#endif
}

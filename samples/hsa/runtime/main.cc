#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <hsa.h>
#include <hsa_ext_finalize.h>

hsa_status_t iterate_agent_callback(hsa_agent_t agent, void *data)
{
	printf("----- HSA Agent -----\n");

	printf("Agent Id: %lld\n", agent.handle);

	// Agent name and vendor name
	char buf[64];
	hsa_agent_get_info(agent, HSA_AGENT_INFO_NAME, buf);
	printf("Agent Name: %s\n", buf);	
	hsa_agent_get_info(agent, HSA_AGENT_INFO_VENDOR_NAME, buf);
	printf("Vendor Name: %s\n", buf);	
	
	// Device type
	unsigned int device_type;
	hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device_type);
	printf("Device type: ");
	switch (device_type)
	{
	case 0:
		printf("CPU\n");
		break;
	case 1:
		printf("GPU\n");
		*(unsigned long long *)data = agent.handle;
		break;
	case 2:
		printf("DSP\n");
		break;	
	}
	
	printf("----- --- ----- -----\n");
}


void print_system_info()
{
	printf("\n----- HSA System Info -----\n");

	unsigned major, minor;
	hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MAJOR, &major);
	hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MINOR, &minor);	
	printf("HSA System Version: {%d, %d}\n", major, minor);

	unsigned long long timestamp, timestamp_freq, max_signal_wait;
	hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP, &timestamp);
	hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY, &timestamp_freq);
	hsa_system_get_info(HSA_SYSTEM_INFO_SIGNAL_MAX_WAIT, &max_signal_wait);
	printf("HSA System Timestamp: %lld\n", timestamp);
	printf("HSA System Timestamp Frequency: %lld\n", timestamp_freq);
	printf("HSA System Singal Max Wait: %lld\n", max_signal_wait);

	printf("----- --- ------ ---- -----\n\n");	
}


int main()
{
	// Initialize the runtime
	hsa_init();

	// Get system info
	print_system_info();

	// Retrieve kernel agent
	hsa_agent_t kernel_agent;	
	printf("\n----- HSA Devices -----\n");
	hsa_iterate_agents(&iterate_agent_callback, &kernel_agent);
	printf("----- --- ------- -----\n\n");
	printf("Device selected %lld.\n", kernel_agent.handle);

	// Create queue
	hsa_queue_t *queue;
	hsa_queue_create(
		kernel_agent, 
		4, 
		HSA_QUEUE_TYPE_SINGLE, 
		NULL, 
		NULL, 
		10000, 
		100000,
		&queue);

	// Create HSA program
	char binary[] = "fir_Kernels.brig";
	hsa_ext_program_t program;
	hsa_ext_program_create(
			HSA_MACHINE_MODEL_LARGE, 
			HSA_PROFILE_FULL, 
			HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT, 
			NULL, 
			&program);
	printf("Program: 0x%016llx\n", program.handle);
	hsa_ext_program_add_module(
			program, 
			(hsa_ext_module_t)binary);

	// Finalize code
	hsa_code_object_t code_object;
	hsa_isa_t isa;	
	hsa_ext_control_directives_t control_directives;
	hsa_ext_program_finalize(
			program, 
			isa, 
			HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO, 
			control_directives, 
			NULL, 
			HSA_CODE_OBJECT_TYPE_PROGRAM, 
			&code_object);
	printf("Code object: 0x%016llx\n", code_object.handle);

	// Prepare the executable
	hsa_executable_t executable;
	hsa_executable_create(
			HSA_PROFILE_FULL, 
			HSA_EXECUTABLE_STATE_UNFROZEN, 
			NULL, 
			&executable);
	printf("Executable: 0x%016llx\n", executable.handle);
	hsa_executable_load_code_object(
			executable, 
			kernel_agent, 
			code_object, 
			NULL);
	hsa_executable_freeze(executable, NULL);

	// Retreive the kernel object
	hsa_executable_symbol_t symbol;
	hsa_executable_get_symbol(executable,
			"fir_Kernels",
			"&__OpenCL_FIR_kernel",
			kernel_agent,
			HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
			&symbol);
	printf("Symbol: 0x%016llx\n", symbol.handle);
	unsigned long long kernel_object;
	hsa_executable_symbol_get_info(symbol, 
			HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT,
			&kernel_object);
	printf("Kernel object: 0x%016llx\n", kernel_object);

	// Prepare kernel argument
	unsigned long long offset_0 = 0;
	unsigned long long offset_1 = 0;
	unsigned long long offset_2 = 0;
	unsigned long long printf_buffer = 0;
	unsigned long long vqueue_pointer = 0;
	unsigned long long aqlwrap_pointer = 0;
	float input[1024];
	float output[1024];
	float coeff[4];
	unsigned int numTap = 4;
	coeff[0] = 1;
	coeff[1] = 1;
	coeff[2] = 1;
	coeff[3] = 1;
	for (int i = 0; i < 1024; i++)
	{
		input[i] = 1;
		printf("%f\n", input[i]);
	}

	unsigned int args[19] = {0};
	memcpy(args + 0, &offset_0, 8);
	memcpy(args + 2, &offset_1, 8);
	memcpy(args + 4, &offset_2, 8);
	memcpy(args + 6, &printf_buffer, 8);
	memcpy(args + 8, &vqueue_pointer, 8);
	memcpy(args + 10, &aqlwrap_pointer, 8);
	args[12] = (unsigned)output;
	args[14] = (unsigned)coeff;
	args[16] = (unsigned)input;
	//memcpy(args + 12, &output, 4);
	//memcpy(args + 14, &coeff, 4);
//	memcpy(args + 16, &input, 4);
	memcpy(args + 18, &numTap, 4);

	
	// Read index and write index
	unsigned long long read_index;
	unsigned long long write_index;
	unsigned long long base_address;
	read_index = hsa_queue_load_read_index_relaxed(queue);
	write_index = hsa_queue_load_write_index_relaxed(queue);
	base_address = (unsigned long long)queue->base_address;
	printf("Read Index: %lld\n", read_index);
	printf("Write Index: %lld\n", write_index);
	printf("Base address 0x%016llx\n", base_address);

	// Create packet
	hsa_kernel_dispatch_packet_t *packet = 
		(hsa_kernel_dispatch_packet_t *)base_address + write_index;
	memset(packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
	packet->setup = 1;
	packet->workgroup_size_x = 201;
	packet->workgroup_size_y = 1;
	packet->workgroup_size_z = 1;
	packet->grid_size_x = 1024;
	packet->grid_size_y = 1;
	packet->grid_size_z = 1;
	packet->private_segment_size = 100;
	packet->group_segment_size = 1000;
	packet->kernel_object = kernel_object;
	packet->kernarg_address = args;
	//memcpy(&(packet->kernarg_address), &args, 4);
	printf("Kernal args: %p, packet->kernarg_address: %p\n", args, packet->kernarg_address);
	packet->completion_signal.handle = 0;

	// Release the packet
	*(unsigned int *)packet = 2;
	queue->doorbell_signal.handle = (unsigned long long)packet;
	hsa_queue_add_write_index_relaxed(queue, sizeof(hsa_kernel_dispatch_packet_t));

	// Wait the kernel execution finish
	while(1)
	{
		if (packet->completion_signal.handle == 1) break;
	}

	for (int i = 0; i < 1024; i++)
	{
		printf("%f\n", output[i]);
	}


	return 1;

}

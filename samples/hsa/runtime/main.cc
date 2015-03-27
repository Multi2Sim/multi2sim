#include <hsa.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>

hsa_status_t iterate_agent_callback(hsa_agent_t agent, void *data)
{
	printf("----- HSA Agent -----\n");

	printf("Agent Id: %lld\n", (unsigned long long)agent);

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
		*(unsigned long long *)data = agent;
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
	printf("Device selected %lld.\n", kernel_agent);

	// Create queue
	hsa_queue_t *queue;
	hsa_queue_create(kernel_agent, 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, &queue);

	// Load brig file
	FILE *file = fopen("fir_Kernels.brig", "r");
	if (file == NULL)
	{
		perror("Fail to open brig file.\n");
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	const size_t file_size = (size_t)ftell(file);
	fseek(file, 0, SEEK_SET);
	char *binary = (char *)malloc(file_size);
	fread(binary, 1, file_size, file);
	
	// Create HSA program
	hsa_ext_program_t program;
	hsa_ext_program_create(
			HSA_MACHINE_MODEL_LARGE, 
			HSA_PROFILE_FULL, 
			HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT, 
			NULL, 
			&program);
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

	// Prepare the executable
	hsa_executable_t executable;
	hsa_executable_create(
			HSA_PROFILE_FULL, 
			HSA_EXECUTABLE_STATE_UNFROZEN, 
			NULL, 
			&executable);
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
	unsigned long long kernel_object;
	hsa_executable_symbol_get_info(symbol, 
			HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT,
			&kernel_object);
	
	// Read index and write index
	unsigned long long read_index;
	unsigned long long write_index;
	unsigned long long base_address;
	read_index = hsa_queue_load_read_index_relaxed(queue);
	write_index = hsa_queue_load_write_index_relaxed(queue);
	base_address = queue->base_address;
	printf("Read Index: %lld\n", read_index);
	printf("Write Index: %lld\n", write_index);
	printf("Base address %lld\n", base_address);

	// Create packet
	hsa_dispatch_packet_s *packet = (hsa_dispatch_packet_s *)base_address + write_index;
	memset(packet, 0, sizeof(hsa_dispatch_packet_s));
	packet->dimensions = 1;
	packet->workgroup_size_x = 256;
	packet->workgroup_size_y = 1;
	packet->workgroup_size_z = 1;
	packet->grid_size_x = 1024;
	packet->grid_size_y = 1;
	packet->grid_size_z = 1;
	packet->private_segment_size = 100;
	packet->group_segment_size = 1000;
	packet->kernel_object_address = kernel_object;
	packet->kernarg_address = 0;	
	packet->completion_signal = 0;

	// Release the packet
	*(unsigned int *)packet = 2;
	queue->doorbell_signal = (unsigned long long)packet;
	hsa_queue_add_write_index_relaxed(queue, sizeof(hsa_dispatch_packet_s));

	// Wait the kernel execution finish
	while(1)
	{
		if (packet->completion_signal == 1) break;
	}

	return 1;

}

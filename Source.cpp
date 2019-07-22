#include <iostream>
#include <CL/cl2.hpp>
#include <fstream>
#include <cstdlib>
#include <string>

#define SIZE 262144
#define RANGE 256

void check_error(cl_int error, unsigned short int position);

int main() {
	unsigned int * vetor1 = new unsigned int[SIZE];
	unsigned int * vetor2 = new unsigned int[SIZE];
	unsigned int * vetor3 = new unsigned int[1024];
	unsigned int x;
	unsigned short int y;

	//POPULATING VETOR1 AND VETOR3 WITH FOO NUMBERS FOR COMPARISON
	for (x = 0; x < SIZE; x += RANGE) {
		for (y = 0; y < RANGE; y++) {
			vetor1[x + y] = y + 1; //ARRAY USED AS MATRIX
		}
	}

	for (x = 0; x < 1024; x++) {
		vetor3[x] = 0; //IN THE KERNEL, CHECKED WILL BE SET TO 1
	}

	for (x = 0; x < SIZE; x += RANGE) {
		srand(x);
		for (y = 0; y < RANGE; y++) {
			vetor2[x + y] = rand() % 256 + 1; //THE SAME THAT vetor1, BUT RANDOM.
		}
	}

	float tpo1, tpo2;

	std::cout << "Voce quer usar o OpenCL (na GPU) para comparar ou o programa normal (na CPU)?\n1 para OpenCL\n2 para normal\n";
	std::cin >> x;

	if (x == 1) {
		//CLOCK START
		tpo1 = clock();

		//START OF OPENCL PART
		cl_int err;

		//VECTORIZE THE PLATFORMS NUMBERS ACCONDING TO Platfom CLASS
		std::vector<cl::Platform> all_platforms;

		//INSTANTIATE THE PLATFORM
		cl::Platform::get(&all_platforms);

		//CHECK IF THERE IS ANY PLATFORM(S)
		if (all_platforms.size() == 0) {
			std::cout << " No platforms found. Check OpenCL installation!\n";
			system("PAUSE");
			exit(1);
		}
		//SET THE PLATFORM TO OPENCL
		cl::Platform default_platform = all_platforms[0];
		all_platforms.clear();

		//get default device of the default platform
		std::vector<cl::Device> all_devices;

		//HERE THE TYPE CAN BE _ALL, _CPU OR _GPU
		default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
		if (all_devices.size() == 0) {
			std::cout << " No devices found. Check OpenCL installation!\n";
			system("PAUSE");
			exit(1);
		}

		//SET THE DEVICE TO GPU
		cl::Device default_device = all_devices[0];

		//INSTANTIATE A CONTEXT IN THE GPU
		cl::Context context({ default_device });

		//INSTANTIATE SOURCES CODE TO DEVICE
		cl::Program::Sources sources;

		//OPEN CODE FILE
		std::ifstream sourceFile("simple_comparison.cl"); //IN THE FILE, for12 AND for123 VARIABLES IS TO SAVE MATH OPERATIONS

		//THE CODE HAS TO BE A STRING
		std::string kernel_code(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));

		//PUSH THE CODE STRING TO THE INSTANCE OF SOURCES
		sources.push_back({ kernel_code.c_str(),kernel_code.length() });

		//INSTANTIATE THE BINARY OF KERNEL / COMPILE THE SOURCE CODE
		cl::Program program(context, sources);
		if (program.build({ default_device }) != CL_SUCCESS) {
			std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
			system("PAUSE");
			exit(1);
		}

		//ALLOC DEVICE MEMORY
		cl::Buffer buffer_1(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * SIZE, NULL, &err);
		check_error(err, 1);
		cl::Buffer buffer_2(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * SIZE, NULL, &err);
		check_error(err, 2);
		cl::Buffer buffer_3(context, CL_MEM_WRITE_ONLY, sizeof(unsigned int) * 1024, NULL, &err);
		check_error(err, 3);

		//INSTANTIATE A QUEUE OF TASKS
		cl::CommandQueue queue(context, default_device, NULL, &err);
		check_error(err, 4);

		//METHOD EXPLAIN ITSELF
		err = queue.enqueueWriteBuffer(buffer_1, CL_TRUE, 0, sizeof(unsigned int) * SIZE, vetor1);
		check_error(err, 5);
		err = queue.enqueueWriteBuffer(buffer_2, CL_TRUE, 0, sizeof(unsigned int) * SIZE, vetor2);
		check_error(err, 6);

		//INSTANTIATE THE KERNEL
		cl::Kernel kernel_comparison(program, "simple_comparison", &err);
		check_error(err, 7);

		//SET ARGS OF KERNEL
		err = kernel_comparison.setArg(0, buffer_1);
		check_error(err, 8);
		err = kernel_comparison.setArg(1, buffer_2);
		check_error(err, 9);
		err = kernel_comparison.setArg(2, buffer_3);
		check_error(err, 10);

		//ENQUEUE THE KERNEL dimension[]
		err = queue.enqueueNDRangeKernel(kernel_comparison, cl::NullRange, cl::NDRange(RANGE), cl::NullRange);
		check_error(err, 11);

		std::cout << "antes de executar o kernel." << std::endl;

		//EXECUTE ALL ENQUEUED TASKS IN THE ORDER ONE BY ONE
		err = queue.finish();
		check_error(err, 12);
		//--------------------------------------------
		//printf("%u-%u\n", t1, t10); debug kernel

		//READ THE OUTPUT BUFFER
		err = queue.enqueueReadBuffer(buffer_3, CL_TRUE, 0, sizeof(unsigned int) * 1024, vetor3);
		check_error(err, 13);

		//WAIT FINISH READING
		err = queue.finish();
		check_error(err, 14);

		std::cout << "dps de ler buffer." << std::endl;
		/*
		//CLEANUP ALL OPENCL STUFF
		clReleaseCommandQueue(queue());
		clReleaseKernel(kernel_comparison());
		clReleaseMemObject(buffer_1());
		clReleaseMemObject(buffer_2());
		clReleaseMemObject(buffer_3());
		clReleaseProgram(program());
		clReleaseContext(context());
		clReleaseDevice(default_device());
		sourceFile.clear();
		sourceFile.close();
		all_devices.clear();
		//END OF OPENCL PART
		*/
		std::cout << "dps de ler buffer." << std::endl;
	}

	//SAME COMPARISON OPERATION OF KERNEL, ADAPTED TO CPU
	//UNCOMMENT AND COMMENT OPENCL PART TO CLOCK THIS
	else if (x == 2) {

		//CLOCK START
		tpo1 = clock();

		printf("comeco da comparacao\n");
		const unsigned short int match1 = RANGE, match2 = 1024;
		const unsigned int arraysize = SIZE;
		unsigned short int counter1 = 0, counter2 = 0;
		unsigned int for1, for2, for3, for4;
		for (for1 = 0; for1 < SIZE; for1 += RANGE) {
			for (for2 = 0; for2 < SIZE; for2 += RANGE) {
				for (for3 = 0; for3 < RANGE; for3++) {
					for (for4 = 0; for4 < RANGE; for4++) {
						if (vetor1[for1 + for3] == vetor2[for2 + for4]) {
							counter1++;
						}
					}
				}
				if (counter1 == match1) {
					counter2++;
					//std::cout << "ok1 - counter1: " << counter1 << "\n";
					counter1 = 0;
				}
			}
			if (counter2 == match2) {
				if (for1 > 0) {
					vetor3[for1 / 256] = 1;
					counter2 = 0;
				}
				else {
					vetor3[for1] = 1;
					counter2 = 0;
				}
			}
		}
		printf("end of comparison\n\n");
	}
	else {
		std::cout << "opcao invalida.";
		delete[] vetor1;
		delete[] vetor2;
		delete[] vetor3;
		system("PAUSE");
		return 0;
	}

	//CLOCK END
	tpo2 = clock() - tpo1;
	std::cout << "Este ciclo demorou: " << (tpo2 / 1000.0) << " segundos." << std::endl;
	system("PAUSE");

	//SHOW
	std::ofstream vetor3_log("vetor3_log.txt", std::ios::trunc);
	if (vetor3_log.is_open()) {
		for (x = 0; x < 1024; x++) {
			vetor3_log << "Posicao " << x << ": " << vetor3[x] << std::endl;
		}
		std::cout << "\nleia o arquivo vetor3_log.txt\n";
		system("PAUSE");
	}
	else {
		std::cerr << "erro criando vetor3_log.txt";
		system("PAUSE");
		exit(1);
	}
	vetor3_log.clear();
	vetor3_log.close();

	//ARRAYS CLEANUP
	delete[] vetor1;
	delete[] vetor2;
	delete[] vetor3;

	return 0;
}

void check_error(cl_int error, unsigned short int position) {
	std::string nomeDoErro;
	if (error != CL_SUCCESS) {
		switch (error) {
		case -1:
			nomeDoErro = "CL_DEVICE_NOT_FOUND	clGetDeviceIDs	if no OpenCL devices that matched device_type were found.";
		case -2:
			nomeDoErro = "CL_DEVICE_NOT_AVAILABLE	clCreateContext	if a device in devices is currently not available even though the device was returned by clGetDeviceIDs.";
		case -3:
			nomeDoErro = "CL_COMPILER_NOT _AVAILABLE	clBuildProgram	if program is created with clCreateProgramWithSource and a compiler is not available i.e. CL_DEVICE_COMPILER_AVAILABLE specified in the table of OpenCL Device Queries for clGetDeviceInfo is set to CL_FALSE.";
		case -4:
			nomeDoErro = "CL_MEM_OBJECT _ALLOCATION_FAILURE		if there is a failure to allocate memory for buffer object.";
		case -5:
			nomeDoErro = "CL_OUT_OF_RESOURCES		if there is a failure to allocate resources required by the OpenCL implementation on the device.";
		case -6:
			nomeDoErro = "CL_OUT_OF_HOST_MEMORY		if there is a failure to allocate resources required by the OpenCL implementation on the host.";
		case -7:
			nomeDoErro = "CL_PROFILING_INFO_NOT _AVAILABLE	clGetEventProfilingInfo	if the CL_QUEUE_PROFILING_ENABLE flag is not set for the command-queue, if the execution status of the command identified by event is not CL_COMPLETE or if event is a user event object.";
		case -8:
			nomeDoErro = "CL_MEM_COPY_OVERLAP	clEnqueueCopyBuffer, clEnqueueCopyBufferRect, clEnqueueCopyImage	if src_buffer and dst_buffer are the same buffer or subbuffer object and the source and destination regions overlap or if src_buffer and dst_buffer are different sub-buffers of the same associated buffer object and they overlap. The regions overlap if src_offset ≤ to dst_offset ≤ to src_offset + size – 1, or if dst_offset ≤ to src_offset ≤ to dst_offset + size – 1.";
		case -9:
			nomeDoErro = "CL_IMAGE_FORMAT _MISMATCH	clEnqueueCopyImage	if src_image and dst_image do not use the same image format.";
		case -10:
			nomeDoErro = "CL_IMAGE_FORMAT_NOT _SUPPORTED	clCreateImage	if the image_format is not supported.";
		case -11:
			nomeDoErro = "CL_BUILD_PROGRAM _FAILURE	clBuildProgram	if there is a failure to build the program executable. This error will be returned if clBuildProgram does not return until the build has completed.";
		case -12:
			nomeDoErro = "CL_MAP_FAILURE	clEnqueueMapBuffer, clEnqueueMapImage	if there is a failure to map the requested region into the host address space. This error cannot occur for image objects created with CL_MEM_USE_HOST_PTR or CL_MEM_ALLOC_HOST_PTR.";
		case -13:
			nomeDoErro = "CL_MISALIGNED_SUB _BUFFER_OFFSET		if a sub-buffer object is specified as the value for an argument that is a buffer object and the offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.";
		case -14:
			nomeDoErro = "CL_EXEC_STATUS_ERROR_ FOR_EVENTS_IN_WAIT_LIST		if the execution status of any of the events in event_list is a negative integer value.";
		case -15:
			nomeDoErro = "CL_COMPILE_PROGRAM _FAILURE	clCompileProgram	if there is a failure to compile the program source. This error will be returned if clCompileProgram does not return until the compile has completed.";
		case -16:
			nomeDoErro = "CL_LINKER_NOT_AVAILABLE	clLinkProgram	if a linker is not available i.e. CL_DEVICE_LINKER_AVAILABLE specified in the table of allowed values for param_name for clGetDeviceInfo is set to CL_FALSE.";
		case -17:
			nomeDoErro = "CL_LINK_PROGRAM_FAILURE	clLinkProgram	if there is a failure to link the compiled binaries and/or libraries.";
		case -18:
			nomeDoErro = "CL_DEVICE_PARTITION _FAILED	clCreateSubDevices	if the partition name is supported by the implementation but in_device could not be further partitioned.";
		case -19:
			nomeDoErro = "CL_KERNEL_ARG_INFO _NOT_AVAILABLE	clGetKernelArgInfo	if the argument information is not available for kernel.";
		case -30:
			nomeDoErro = "CL_INVALID_VALUE	clGetDeviceIDs, clCreateContext	This depends on the function: two or more coupled parameters had errors.";
		case -31:
			nomeDoErro = "CL_INVALID_DEVICE_TYPE	clGetDeviceIDs	if an invalid device_type is given";
		case -32:
			nomeDoErro = "CL_INVALID_PLATFORM	clGetDeviceIDs	if an invalid platform was given";
		case -33:
			nomeDoErro = "CL_INVALID_DEVICE	clCreateContext, clBuildProgram	if devices contains an invalid device or are not associated with the specified platform.";
		case -34:
			nomeDoErro = "CL_INVALID_CONTEXT		if context is not a valid context.";
		case -35:
			nomeDoErro = "CL_INVALID_QUEUE_PROPERTIES	clCreateCommandQueue	if specified command-queue-properties are valid but are not supported by the device.";
		case -36:
			nomeDoErro = "CL_INVALID_COMMAND_QUEUE		if command_queue is not a valid command-queue.";
		case -37:
			nomeDoErro = "CL_INVALID_HOST_PTR	clCreateImage, clCreateBuffer	This flag is valid only if host_ptr is not NULL. If specified, it indicates that the application wants the OpenCL implementation to allocate memory for the memory object and copy the data from memory referenced by host_ptr.CL_MEM_COPY_HOST_PTR and CL_MEM_USE_HOST_PTR are mutually exclusive.CL_MEM_COPY_HOST_PTR can be used with CL_MEM_ALLOC_HOST_PTR to initialize the contents of the cl_mem object allocated using host-accessible (e.g. PCIe) memory.";
		case -38:
			nomeDoErro = "CL_INVALID_MEM_OBJECT		if memobj is not a valid OpenCL memory object.";
		case -39:
			nomeDoErro = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR		if the OpenGL/DirectX texture internal format does not map to a supported OpenCL image format.";
		case -40:
			nomeDoErro = "CL_INVALID_IMAGE_SIZE		if an image object is specified as an argument value and the image dimensions (image width, height, specified or compute row and/or slice pitch) are not supported by device associated with queue.";
		case -41:
			nomeDoErro = "CL_INVALID_SAMPLER	clGetSamplerInfo, clReleaseSampler, clRetainSampler, clSetKernelArg	if sampler is not a valid sampler object.";
		case -42:
			nomeDoErro = "CL_INVALID_BINARY	clCreateProgramWithBinary, clBuildProgram	The provided binary is unfit for the selected device. if program is created with clCreateProgramWithBinary and devices listed in device_list do not have a valid program binary loaded.";
		case -43:
			nomeDoErro = "CL_INVALID_BUILD_OPTIONS	clBuildProgram	if the build options specified by options are invalid.";
		case -44:
			nomeDoErro = "CL_INVALID_PROGRAM		if program is a not a valid program object.";
		case -45:
			nomeDoErro = "CL_INVALID_PROGRAM_EXECUTABLE		if there is no successfully built program executable available for device associated with command_queue.";
		case -46:
			nomeDoErro = "CL_INVALID_KERNEL_NAME	clCreateKernel	if kernel_name is not found in program.";
		case -47:
			nomeDoErro = "CL_INVALID_KERNEL_DEFINITION	clCreateKernel	if the function definition for __kernel function given by kernel_name such as the number of arguments, the argument types are not the same for all devices for which the program executable has been built.";
		case -48:
			nomeDoErro = "CL_INVALID_KERNEL		if kernel is not a valid kernel object.";
		case -49:
			nomeDoErro = "CL_INVALID_ARG_INDEX	clSetKernelArg, clGetKernelArgInfo	if arg_index is not a valid argument index.";
		case -50:
			nomeDoErro = "CL_INVALID_ARG_VALUE	clSetKernelArg, clGetKernelArgInfo	if arg_value specified is not a valid value.";
		case -51:
			nomeDoErro = "CL_INVALID_ARG_SIZE	clSetKernelArg	if arg_size does not match the size of the data type for an argument that is not a memory object or if the argument is a memory object and arg_size != sizeof(cl_mem) or if arg_size is zero and the argument is declared with the __local qualifier or if the argument is a sampler and arg_size != sizeof(cl_sampler).";
		case -52:
			nomeDoErro = "CL_INVALID_KERNEL_ARGS		if the kernel argument values have not been specified.";
		case -53:
			nomeDoErro = "CL_INVALID_WORK_DIMENSION		if work_dim is not a valid value (i.e. a value between 1 and 3).";
		case -54:
			nomeDoErro = "CL_INVALID_WORK_GROUP_SIZE		if local_work_size is specified and number of work-items specified by global_work_size is not evenly divisable by size of work-group given by local_work_size or does not match the work-group size specified for kernel using the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier in program source.if local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] *… local_work_size[work_dim – 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in the table of OpenCL Device Queries for clGetDeviceInfo.if local_work_size is NULL and the __attribute__ ((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source.";
		case -55:
			nomeDoErro = "CL_INVALID_WORK_ITEM_SIZE		if the number of work-items specified in any of local_work_size[0], … local_work_size[work_dim – 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], …. CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim – 1].";
		case -56:
			nomeDoErro = "CL_INVALID_GLOBAL_OFFSET		if the value specified in global_work_size + the corresponding values in global_work_offset for any dimensions is greater than the sizeof(size_t) for the device on which the kernel execution will be enqueued.";
		case -57:
			nomeDoErro = "CL_INVALID_EVENT_WAIT_LIST		if event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events.";
		case -58:
			nomeDoErro = "CL_INVALID_EVENT		if event objects specified in event_list are not valid event objects.";
		case -59:
			nomeDoErro = "CL_INVALID_OPERATION		if interoperability is specified by setting CL_CONTEXT_ADAPTER_D3D9_KHR, CL_CONTEXT_ADAPTER_D3D9EX_KHR or CL_CONTEXT_ADAPTER_DXVA_KHR to a non-NULL value, and interoperability with another graphics API is also specified. (only if the cl_khr_dx9_media_sharing extension is supported).";
		case -60:
			nomeDoErro = "CL_INVALID_GL_OBJECT		if texture is not a GL texture object whose type matches texture_target, if the specified miplevel of texture is not defined, or if the width or height of the specified miplevel is zero.";
		case -61:
			nomeDoErro = "CL_INVALID_BUFFER_SIZE	clCreateBuffer, clCreateSubBuffer	if size is 0.Implementations may return CL_INVALID_BUFFER_SIZE if size is greater than the CL_DEVICE_MAX_MEM_ALLOC_SIZE value specified in the table of allowed values for param_name for clGetDeviceInfo for all devices in context.";
		case -62:
			nomeDoErro = "CL_INVALID_MIP_LEVEL	OpenGL-functions	if miplevel is greater than zero and the OpenGL implementation does not support creating from non-zero mipmap levels.";
		case -63:
			nomeDoErro = "CL_INVALID_GLOBAL_WORK_SIZE		if global_work_size is NULL, or if any of the values specified in global_work_size[0], …global_work_size [work_dim – 1] are 0 or exceed the range given by the sizeof(size_t) for the device on which the kernel execution will be enqueued.";
		case -64:
			nomeDoErro = "CL_INVALID_PROPERTY	clCreateContext	Vague error, depends on the function";
		case -65:
			nomeDoErro = "CL_INVALID_IMAGE_DESCRIPTOR	clCreateImage	if values specified in image_desc are not valid or if image_desc is NULL.";
		case -66:
			nomeDoErro = "CL_INVALID_COMPILER_OPTIONS	clCompileProgram	if the compiler options specified by options are invalid.";
		case -67:
			nomeDoErro = "CL_INVALID_LINKER_OPTIONS	clLinkProgram	if the linker options specified by options are invalid.";
		case -68:
			nomeDoErro = "CL_INVALID_DEVICE_PARTITION_COUNT	clCreateSubDevices	if the partition name specified in properties is CL_DEVICE_PARTITION_BY_COUNTS and the number of sub-devices requested exceeds CL_DEVICE_PARTITION_MAX_SUB_DEVICES or the total number of compute units requested exceeds CL_DEVICE_PARTITION_MAX_COMPUTE_UNITS for in_device, or the number of compute units requested for one or more sub-devices is less than zero or the number of sub-devices requested exceeds CL_DEVICE_PARTITION_MAX_COMPUTE_UNITS for in_device.";
		case -69:
			nomeDoErro = "CL_INVALID_PIPE_SIZE	clCreatePipe	if pipe_packet_size is 0 or the pipe_packet_size exceeds CL_DEVICE_PIPE_MAX_PACKET_SIZE value for all devices in context or if pipe_max_packets is 0.";
		case -70:
			nomeDoErro = "CL_INVALID_DEVICE_QUEUE	clSetKernelArg	when an argument is of type queue_t when it’s not a valid device queue object.";
		}
		std::cout << nomeDoErro << std::endl;
		system("PAUSE");
		std::ofstream log_error("logerros.txt", std::ios::app);
		if (log_error.is_open()) {
			log_error << nomeDoErro << " Found in " << position << " position." << std::endl;
			log_error.clear();
			log_error.close();
		}
		else {
			std::cout << "erro :" << nomeDoErro << std::endl;
			system("PAUSE");
			exit(1);
		}
		log_error.clear();
		log_error.close();
	}
}
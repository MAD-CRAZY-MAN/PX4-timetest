px4_add_module(

	MODULE examples__fmu_registers
	MAIN fmu_registers
	STACK_MAIN 2000
	SRCS
		fmu_registers.c
	DEPENDS
	)

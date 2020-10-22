px4_add_module(
	MODULE examples__timer_test
	MAIN timer_test
	STACK_MAIN 2000
	SRCS
		timer_test.cpp
	)

px4_add_module(
	MODULE examples__tim4
	MAIN tim4
	STACK_MAIN 2000
	SRCS
		tim4.cpp
	DEPENDS
	)

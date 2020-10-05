#include <nuttx/config.h>
#include <stdio.h>
#include <errno.h>

__EXPORT int px4_simple_app_main(int argc, char *argv[]);

int px4_simple_app_main(int argc, char *argv[])
{
	printf("Hello Sky~\n");
	return OK;
}

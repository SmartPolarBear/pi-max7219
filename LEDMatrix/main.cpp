#include <cstdio>
#include "led_matrix.h"
#include <cstring>
#include <cstdlib>
#include <bcm2835.h>

int main(int argc, char *argv[])
{
	size_t brightness = 3;
	char *msg = nullptr;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-b") == 0)
		{
			char *num = argv[i + 1];
			brightness = static_cast<decltype(brightness)>(atoi(num));
		}
		else if (strcmp(argv[i], "-m") == 0)
		{
			msg = argv[i + 1];
		}
	}

	if (msg == nullptr)
	{
		printf("E:Invalied options."); 
		return -1;
	}
	else
	{
		max7219<8, 8>::get_instance().brightness(brightness);
		max7219<8, 8>::get_instance().message_custom_speed<150>(msg);
	}

	return 0;
}

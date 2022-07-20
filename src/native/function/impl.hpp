#if __x86_64__
	#if _WIN64
		#include "x86_64_windows.hpp"
	#elif __gnu_linux__
		#include "x86_64_linux.hpp"
	#endif
#endif
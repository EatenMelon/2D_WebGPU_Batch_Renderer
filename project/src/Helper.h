#ifndef _HELPER
#define _HELPER

namespace helper
{
	static inline uint64_t Align(uint64_t value, uint64_t alignment)
	{
		uint64_t out{ value };

		if (value % alignment != 0)
		{
			out = (value / alignment + 1) * alignment;
		}

		return out;
	}
}

#endif
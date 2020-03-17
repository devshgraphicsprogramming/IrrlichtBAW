#include "irr/builtin/resuorces.h"

using namespace irr;
using namespace irr::builtin;

template std::pair<uint8_t*, size_t> get_resource<IRR_CORE_UNIQUE_STRING_LITERAL_TYPE("irr/builtin/X/Y/Z/something.type")>()
{
	static const uint8_t data[] = irr::builtin::somevariablename

	return { data,sizeof(data) };
}

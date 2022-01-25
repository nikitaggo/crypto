#include <iomanip>
#ifndef ECGOST_H_
#define ECGOST_H_


namespace ec
{
namespace ec_gost
{
void gost_34_12_2018_64(
		const uint8_t	*src_block,
		const uint8_t	*key,
		bool			reverse_key,
		uint8_t			*dst_block);
}
}


#endif /* ECGOST_H_ */

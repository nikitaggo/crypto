/*
 * ECC.h
 *
 *  Created on: 22 ����. 2021 �.
 *      Author: inter
 */

#ifndef ECC_H_
#define ECC_H_

#include <cstdint>

namespace ec {

namespace container
{
// MAGIC number = "N!GR"

constexpr uint32_t MAGIC =
		0x00000001 * 'N' +
		0x00000100 * '!' +
		0x00010000 * 'G' +
		0x01000000 * 'R' ;

enum payload_type
{
	RAW = 0,
	KEY_DATA,
	PRIVATE_KEY,
	PUBLIC_KEY,
	ENCRYPTED_DATA,
	DH_PARAMS,
};

constexpr uint32_t HEADER_SIZE_V1 = 12;
constexpr uint32_t FILE_METADATA_SIZE_V1_BASE = 24;

#pragma pack(push, 1)

struct header
{
	uint32_t magic;
	uint32_t header_size;

	union {
		struct {
			uint8_t payload;
			uint8_t padding[3];
		} v1;
	};
};

struct metadata
{
	uint32_t length;

	union {
		struct {
			uint64_t orig_length;
			uint64_t block_count;
			uint32_t block_size;

		} file;
		struct {
			uint64_t orig_length;
			uint64_t block_count;
			uint32_t block_size;
		} key;
		struct {
		} dh_params;
	};
};

#pragma pack(pop)

}

}



#endif /* ECC_H_ */

/*
 * main.cc
 *
 *  Created on: 22 сент. 2021 г.
 *      Author: inter
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <stdint.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <cstdlib>

#include <ECC.h>

const char * TEST_KEY_FILE = "test-key-file.nicc";
const char * TEST_FILE_NAME = "animelove.jpg";
const char * TEST_CONTAINER_NAME = "test-cont.nicc";
const uint32_t TEST_BLOCK_SIZE = 32;
const char * TEST_CRYPTOCONTAINER = "test-cryptocontainer.ncc";
const uint32_t T_SWAP[16] = {3, 5, 4, 8, 9, 1, 11, 13, 12, 0, 15, 2, 7, 6, 10, 14};

void test_create_container()
{
	std::ifstream src_file;
	std::ofstream dst_file;

	src_file.open(TEST_FILE_NAME, std::ios::binary | std::ios::ate);
	size_t filesize = src_file.tellg();
	src_file.seekg(0);

	dst_file.open(TEST_CONTAINER_NAME, std::ios::binary);

	using namespace ec::container;

	header hdr {};
	hdr.magic = MAGIC;
	//hdr.version = 1;
	hdr.header_size = HEADER_SIZE_V1;
	hdr.v1.payload = RAW;
	dst_file.write(reinterpret_cast<char*>(&hdr), HEADER_SIZE_V1);

	metadata md {};
	uint32_t name_length = strlen(TEST_FILE_NAME);
	md.length = FILE_METADATA_SIZE_V1_BASE + name_length + 1;
	md.file.orig_length = filesize;
	md.file.block_size = TEST_BLOCK_SIZE;
	md.file.block_count = filesize / (TEST_BLOCK_SIZE / 8);
	if (filesize % (TEST_BLOCK_SIZE / 8) > 0)
		md.file.block_count++;
	dst_file.write(reinterpret_cast<char*>(&md), FILE_METADATA_SIZE_V1_BASE);
	dst_file.write(TEST_FILE_NAME, name_length + 1);

	for (uint64_t block = 0; block <md.file.block_count; block++)
	{
		uint8_t buffer[TEST_BLOCK_SIZE / 8] {};
		src_file.read(reinterpret_cast<char*>(&buffer[0]),
				TEST_BLOCK_SIZE / 8);
		dst_file.write(reinterpret_cast<char*>(&buffer[0]),
				TEST_BLOCK_SIZE / 8);
	}

	src_file.close();
	dst_file.close();
}

void test_extract_container()
{
	std::ifstream src_file;
	std::ofstream dst_file;

	using namespace ec::container;

	src_file.open(TEST_CONTAINER_NAME, std::ios::binary);
	header hdr {};
	src_file.readsome(reinterpret_cast<char*>(&hdr),
			sizeof(header));
	if (hdr.magic != MAGIC) {
		std::cerr <<
				"Контейнер поломался"
				<< std::endl;
		return;
	}
	if (hdr.v1.payload != RAW) {
		std::cerr <<
				"Отсутствует RAW в контейнере"
				<< std::endl;
		return;
	}
	src_file.seekg(hdr.header_size);

	uint64_t pos_after_header = src_file.tellg();

	metadata md {};
	src_file.readsome(reinterpret_cast<char*>(&md),
			FILE_METADATA_SIZE_V1_BASE);
	std::string orig_file_name = "EXT_";
	char c;
	while ((c = src_file.get())){
		orig_file_name += c;
	}

	dst_file.open(orig_file_name.c_str(), std::ios::binary);
	src_file.seekg(pos_after_header + md.length);

	while(md.file.orig_length > 0)
	{
		uint8_t buffer[TEST_BLOCK_SIZE / 8] {};
		src_file.read(reinterpret_cast<char*>(&buffer[0]),
				TEST_BLOCK_SIZE / 8);
		uint64_t bytes_to_write = std::min<unsigned long>(4UL, md.file.orig_length);
		dst_file.write(
				reinterpret_cast<char*>(&buffer[0]),
				bytes_to_write);
		md.file.orig_length -= bytes_to_write;

	}

	dst_file.close();
	src_file.close();


}

void key_container(uint64_t key_length)
{
    std::ofstream src_file;
    using namespace ec::container;

    src_file.open(TEST_KEY_FILE, std::ios::binary);
    if(!src_file.is_open())
        {
            std::cerr <<
                    "File dont open!"
                    << std::endl;
            return;
        }

    header hdr{};
    hdr.magic = MAGIC;
    hdr.header_size = HEADER_SIZE_V1;
    hdr.v1.payload = RAW;
    src_file.write(reinterpret_cast<char*>(&hdr), HEADER_SIZE_V1);

    metadata md{};
    md.length = FILE_METADATA_SIZE_V1_BASE;
    md.key.orig_length = key_length;
    md.key.block_size = TEST_BLOCK_SIZE;
    md.key.block_count = key_length / (TEST_BLOCK_SIZE / 8);
    if (key_length % (TEST_BLOCK_SIZE / 8) > 0)
        md.key.block_count++;
    src_file.write(reinterpret_cast<char*>(&md), FILE_METADATA_SIZE_V1_BASE);
    std::random_device rd;
    std::mt19937 random_mt(rd());
    for (uint64_t block = 0; block < md.key.block_count; block++)
        {
            uint8_t buffer[TEST_BLOCK_SIZE / 8] {};
            //src_file.read(reinterpret_cast<char>(&bufffer[0]),
            //        BLOCK_SIZE / 8);
            srand (static_cast<unsigned int>(time(0)));
            for (uint8_t i = 0; i < TEST_BLOCK_SIZE / 8; i++)
            {
                buffer[i] = random_mt() % 32;
            }

            src_file.write(reinterpret_cast<char*>(&buffer[0]),
            		TEST_BLOCK_SIZE / 8);
        }

    src_file.close();
    std::cout << "Create key container" << std::endl;
}

void encry(uint64_t key_length){
	 std::ofstream src_file;
	 	 using namespace ec::container;

	    src_file.open(TEST_KEY_FILE, std::ios::binary);
	    if(!src_file.is_open())
	        {
	            std::cerr <<
	                    "File dont open!"
	                    << std::endl;
	            return;
	        }

	    header hdr{};
	    hdr.magic = MAGIC;
	    hdr.header_size = HEADER_SIZE_V1;
	    hdr.v1.payload = KEY_DATA;
	    src_file.write(reinterpret_cast<char*>(&hdr), HEADER_SIZE_V1);

	    metadata md{};
	    md.length = FILE_METADATA_SIZE_V1_BASE;
	    md.key.orig_length = key_length;
	    md.key.block_size = TEST_BLOCK_SIZE;
	    md.key.block_count = key_length / (TEST_BLOCK_SIZE / 8);
	    if (key_length % (TEST_BLOCK_SIZE / 8) > 0)
	        md.key.block_count++;
	    src_file.write(reinterpret_cast<char*>(&md), FILE_METADATA_SIZE_V1_BASE);
	    std::random_device rd;
	    std::mt19937 random_mt(rd());
	    for (uint64_t block = 0; block < md.key.block_count; block++)
	        {
	            uint8_t buffer[TEST_BLOCK_SIZE / 8] {};
	            //src_file.read(reinterpret_cast<char>(&bufffer[0]),
	            //        BLOCK_SIZE / 8);
	            srand (static_cast<unsigned int>(time(0)));
	            for (uint8_t i = 0; i < TEST_BLOCK_SIZE / 8; i++)
	            {
	                buffer[i] = random_mt() % 32;
	            }

	            src_file.write(reinterpret_cast<char*>(&buffer[0]),
	    				TEST_BLOCK_SIZE / 8);

	        }

	    src_file.close();
	    std::cout << "Create key container" << std::endl;
	}

	void encryption(uint64_t key_length){
		std::ifstream src_file;
		std::ofstream dst_file;
		src_file.open(TEST_FILE_NAME, std::ios::binary | std::ios::ate);

		if(!src_file.is_open())
		{
			std::cerr <<
					"File dont open!"
					<< std::endl;
			return;
		}

		size_t filesize = src_file.tellg();
		src_file.seekg(0);

		dst_file.open(TEST_CRYPTOCONTAINER, std::ios::binary);

		using namespace ec::container;

		header hdr{};
		hdr.magic = MAGIC;
		hdr.header_size = HEADER_SIZE_V1;
		hdr.v1.payload = ENCRYPTED_DATA;
		dst_file.write(reinterpret_cast<char*>(&hdr), HEADER_SIZE_V1);

		metadata md{};
		uint32_t name_length =strlen(TEST_FILE_NAME);
		md.length = FILE_METADATA_SIZE_V1_BASE + name_length + 1;
		md.file.orig_length = filesize;
		md.file.block_size = TEST_BLOCK_SIZE;
		md.file.block_count = filesize / (TEST_BLOCK_SIZE / 8);
		if (filesize % (TEST_BLOCK_SIZE / 8) > 0)
			md.file.block_count++;
		dst_file.write(reinterpret_cast<char*>(&md), FILE_METADATA_SIZE_V1_BASE);
		dst_file.write(TEST_FILE_NAME, name_length + 1);

		uint8_t key[16] {};
		{
			std::ifstream src2_file;
				src2_file.open(TEST_KEY_FILE, std::ios::binary);
				if(!src2_file.is_open())
				{
					std::cerr <<
							"File dont open!"
							<< std::endl;
					return;
				}
				using namespace ec::container;
				header hdr2 {};
				src2_file.read(reinterpret_cast<char*>(&hdr2),
							sizeof(header));
				if (hdr2.magic != MAGIC)
				{
						std::cerr <<
							"File dont open (MAGIC ERROR)!"
							<< std::endl;
						return;
				}
				if (hdr2.v1.payload != KEY_DATA) {
						std::cerr <<
							"File no KEY_DATA data!"
							<< std::endl;
						return;
				}
				src2_file.seekg(hdr2.header_size);

				uint64_t pos_after_header = src2_file.tellg();

				metadata md2 {};
				src2_file.readsome(reinterpret_cast<char*>(&md2),
						FILE_METADATA_SIZE_V1_BASE);
				src2_file.seekg(pos_after_header + md2.length);

					src2_file.read(reinterpret_cast<char*>(&key[0]),
							16);
					std::cout << reinterpret_cast<char*>(&key[0]) << std::endl;

				src2_file.close();
				std::cout << "Key read OK!" << std::endl;
		}

		for (uint64_t block = 0; block < md.file.block_count; block++)
			{
				uint8_t buffer[TEST_BLOCK_SIZE / 8] {};
				src_file.read(reinterpret_cast<char*>(&buffer[0]),
						TEST_BLOCK_SIZE / 8);
				uint8_t *forMerge = new uint8_t[2];
				forMerge[0] = buffer[0];
				forMerge[1] = buffer[1];
				uint16_t Li =  *((uint16_t*)forMerge);
				forMerge[0] = buffer[2];
				forMerge[1] = buffer[3];
				uint16_t Ri =  *((uint16_t*)forMerge);
				std::cout << " " <<std::endl;
				std::cout << Li << " " << Ri <<std::endl;

				for(uint16_t ttt=0; ttt<8; ttt++)
				{
				uint16_t a[4] {};
				uint32_t y = 0;
				for(uint32_t i = 0; i < 16; i=i+4)
				{
					a[y] = (((Li >> (i+3)) & 1) << 0) | (((Li >> (i+2)) & 1) << 1) | (((Li >> (i+1)) & 1) << 2) | (((Li >> i) & 1) << 3);
					y += 1;
				}

				uint16_t Sx = (T_SWAP[a[3]] << 12) | (T_SWAP[a[2]] << 8) | (T_SWAP[a[1]] << 4) | (T_SWAP[a[0]] << 0);

				uint8_t *forMerge2 = new uint8_t[2];
				forMerge2[0] = key[0];
				forMerge2[1] = key[1];
				uint16_t key_buff =  *((uint16_t*)forMerge2);

				Sx ^= key_buff;

				Sx = (Sx << 3) | (Sx >> (16-3));

				uint16_t oldLi = Li;
				Li = Ri ^ Sx;
				Ri = oldLi;
				std::cout << Li << " " << Ri <<std::endl;
				}
				//std::cout << reinterpret_cast<char*>(&Ri) << std::endl;
				//std::cout << reinterpret_cast<char*>(&Li) << std::endl;
				dst_file.write(reinterpret_cast<char*>(&Ri),
						TEST_BLOCK_SIZE / 16);
				dst_file.write(reinterpret_cast<char*>(&Li),
						TEST_BLOCK_SIZE / 16);
			}

		src_file.close();
		dst_file.close();
		std::cout << "Crypto container:" << std::endl;
}

int main (int argc, char ** argv)
{
	 std::cout << "Start" << std::endl;
	    uint64_t key_length;
	    std::cout << "Key length:" << std::endl;
	    std::cin >> key_length;
	    std::cout << "50%" << std::endl;
	    test_extract_container();
	    key_container(key_length);
	    std::cout << "100%" << std::endl;
	    std::cout << "End" << std::endl;

	return 0;
}




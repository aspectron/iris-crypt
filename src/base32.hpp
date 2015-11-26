//
// Copyright (c) 2015 ASPECTRON Inc.
// All Rights Reserved.
//
// This file is part of IrisCrypt (https://github.com/aspectron/iris-crypt) project.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE
//
#pragma once

#include <string>
#include <algorithm>

namespace base32 {

// RFC 4648 Base32 https://tools.ietf.org/html/rfc4648
struct rfc4648
{
	static char digit(unsigned number)
	{
		static char const digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
		return digits[number];
	}

	static unsigned char number(char digit)
	{
		if (digit >= 'A' && digit <= 'Z') return digit - 'A';
		if (digit >= 'a' && digit <= 'z') return digit - 'a';
		if (digit >= '2' && digit <= '7') return digit - '2' + 26;
		throw std::invalid_argument(std::string("invalid rfc4648 base32 digit: ") + digit);
	}
};

// Human-oriented Base32 encoding http://philzimmermann.com/docs/human-oriented-base-32-encoding.txt
struct zbase
{
	static char digit(unsigned number)
	{
		static char const digits[] = "YBNDRFG8EJKMCPQXOT1UWISZA345H769";
		return digits[number];
	}

	static unsigned char number(char digit)
	{
		switch (digit)
		{
		case 'Y': case 'y': return 0;
		case 'B': case 'b': return 1;
		case 'N': case 'n': return 2;
		case 'D': case 'd': return 3;
		case 'R': case 'r': return 4;
		case 'F': case 'f': return 5;
		case 'G': case 'g': return 6;
		case '8': return 7;
		case 'E': case 'e': return 8;
		case 'J': case 'j': return 9;
		case 'K': case 'k': return 10;
		case 'M': case 'm': return 11;
		case 'C': case 'c': return 12;
		case 'P': case 'p': return 13;
		case 'Q': case 'q': return 14;
		case 'X': case 'x': return 15;
		case 'O': case 'o': return 16;
		case 'T': case 't': return 17;
		case '1': return 18;
		case 'U': case 'u': return 19;
		case 'W': case 'w': return 20;
		case 'I': case 'i': return 21;
		case 'S': case 's': return 22;
		case 'Z': case 'z': return 23;
		case 'A': case 'a': return 24;
		case '3': return 25;
		case '4': return 26;
		case '5': return 27;
		case 'H': case 'h': return 28;
		case '7': return 29;
		case '6': return 30;
		case '9': return 31;
		case '=': return 0;
		}
		throw std::invalid_argument(std::string("invalid zbase32 digit: ") + digit);
	}
};

// Douglas Crockford Base32 encoding, see http://www.crockford.com/wrmg/base32.html
struct crockford
{
	static char digit(unsigned number)
	{
		static char const digits[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
		return digits[number];
	}

	static unsigned char number(char digit)
	{
		switch (digit)
		{
		case '0': case 'o': case 'O': return 0;
		case '1': case 'I': case 'i': case 'L': case 'l': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A': case 'a': return 10;
		case 'B': case 'b': return 11;
		case 'C': case 'c': return 12;
		case 'D': case 'd': return 13;
		case 'E': case 'e': return 14;
		case 'F': case 'f': return 15;
		case 'G': case 'g': return 16;
		case 'H': case 'h': return 17;
		case 'J': case 'j': return 18;
		case 'K': case 'k': return 19;
		case 'M': case 'm': return 20;
		case 'N': case 'n': return 21;
		case 'P': case 'p': return 22;
		case 'Q': case 'q': return 23;
		case 'R': case 'r': return 24;
		case 'S': case 's': return 25;
		case 'T': case 't': return 26;
		case 'V': case 'v': return 27;
		case 'W': case 'w': return 28;
		case 'X': case 'x': return 29;
		case 'Y': case 'y': return 30;
		case 'Z': case 'z': return 31;
		case '=': return 0;
		}
		throw std::invalid_argument(std::string("invalid crockford base32 digit: ") + digit);
	}
};

template<typename Dict>
std::string encode(std::string const& str)
{
	if (str.empty()) return "";

	auto encode_block = [](unsigned char const in[5], char out[8])
	{
		//11111|111_11|11111|1_1111|1111_1|11111|11_111|11111
		out[0] = Dict::digit(in[0] >> 3);
		out[1] = Dict::digit((in[0] & 0x07) << 2 | (in[1] >> 6));
		out[2] = Dict::digit((in[1] & 0x3E) >> 1);
		out[3] = Dict::digit((in[1] & 0x01) << 4 | (in[2] >> 4));
		out[4] = Dict::digit((in[2] & 0x0F) << 1 | (in[3] >> 7));
		out[5] = Dict::digit((in[3] & 0x7C) >> 2);
		out[6] = Dict::digit((in[3] & 0x03) << 3 | (in[4] >> 5));
		out[7] = Dict::digit(in[4] & 0x1F);
	};

	size_t const bits = std::max<size_t>(40, str.size() * 8);
	std::string result((bits / 40 + bool(bits % 40)) * 8, 0);

	unsigned char const* src = (unsigned char const*)str.data();
	char* dst = &result[0];

	// encode by 40-bit blocks
	for (size_t i = 0, count = str.size() / 5; i < count; ++i, src += 5, dst += 8)
	{
		encode_block(src, dst);
	}

	// encode rest bytes
	if (size_t const rest = str.size() % 5)
	{
		encode_block(src, dst); dst += 8;
		// padding
		size_t const num_pads = (5 - rest) * 8 / 5;
		std::fill_n(dst - num_pads, num_pads, '=');
	}

	return result;
}

template<typename Dict>
std::string decode(std::string const& str)
{
	if (str.empty()) return "";

	auto decode_block = [](unsigned char const in[8], char out[5])
	{
		//11111_111|11_11111_1|1111_1111|1_11111_11|111_11111
		out[0] = (Dict::number(in[0]) << 3) | (Dict::number(in[1]) >> 2);
		out[1] = ((Dict::number(in[1]) & 0x03) << 6) | (Dict::number(in[2]) << 1) | (Dict::number(in[3]) >> 4);
		out[2] = ((Dict::number(in[3]) & 0x0F) << 4) | (Dict::number(in[4]) >> 1);
		out[3] = ((Dict::number(in[4]) & 0x01) << 7) | (Dict::number(in[5]) << 2) | (Dict::number(in[6]) >> 3);
		out[4] = ((Dict::number(in[6]) & 0x07) << 5) | (Dict::number(in[7]));
	};

	auto final_length = [](std::string const& str)
	{
		size_t const num_pads = (str.size() % 8) ? -1 : std::count(str.rbegin(), str.rbegin() + 8, '=');
		switch (num_pads)
		{
		case 0: return 0;
		case 1: return 4;
		case 3: return 3;
		case 4: return 2;
		case 6: return 1;
		default: throw std::invalid_argument("invalid padding");
		}
	};

	size_t const rest = final_length(str);
	std::string result(str.size() * 5 / 8 , 0);

	unsigned char const* src = (unsigned char const*)str.data();
	char* dst = &result[0];

	// decode by 40-bit blocks
	for (size_t i = 0, count = str.size() / 8; i < count; ++i, src += 8, dst += 5)
	{
		decode_block(src, dst);
	}

	// decode rest bytes
	if (rest)
	{
		result.resize(result.size() - (5 - rest));
	}

	return result;
}

#if 0
inline void test()
{
	assert(encode<crockford>("\xAA\xAA\xAA\xAA\xAA") == "NANANANA");
	assert(decode<crockford>("NanANAna") == "\xAA\xAA\xAA\xAA\xAA");

	assert(encode<crockford>("123") == "64S36===");
	assert(decode<crockford>("64S36===") == "123");

	assert(encode<crockford>("\xAA\xAA\xAA\xAA\xAA\x6F") == "NANANANADW======");
	assert(decode<crockford>("NANANANADW======") == "\xAA\xAA\xAA\xAA\xAA\x6F");

	assert(encode<crockford>("\x3f\xea") == "7ZN0====");
	assert(decode<crockford>("7zn0====") == "\x3f\xea");

	for (size_t i = 0; i < 1000; ++i)
	{
		std::string str(i, 0);
		std::generate_n(str.begin(), i, std::rand);
		assert(decode<crockford>(encode<crockford>(str)) == str);
	}
}
#endif

} // namespace base32

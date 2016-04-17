#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "md5.h"
#include "base_function.h"

typedef unsigned ulong32;

struct md5_state {
	uint64_t length;
	unsigned long state[4], curlen;
	unsigned char buf[64];
};

typedef union Hash_state {
	struct md5_state    md5;
	void *data;
} hash_state;

extern  struct ltc_hash_descriptor {
	/** name of hash */
	char *name;
	/** internal ID */
	unsigned char ID;
	/** Size of digest in octets */
	unsigned long hashsize;
	/** Input block size in octets */
	unsigned long blocksize;
	/** ASN.1 DER identifier */
	unsigned char DER[64];
	/** Length of DER encoding */
	unsigned long DERlen;
	/** Init a hash state
	@param hash   The hash to initialize
	@return CRYPT_OK if successful
	*/
	int (*init)(hash_state *hash);
	/** Process a block of data 
	@param hash   The hash state
	@param in     The data to hash
	@param inlen  The length of the data (octets)
	@return CRYPT_OK if successful
	*/
	int (*process)(hash_state *hash, const unsigned char *in, unsigned long inlen);
	/** Produce the digest and store it
	@param hash   The hash state
	@param out    [out] The destination of the digest
	@return CRYPT_OK if successful
	*/
	int (*done)(hash_state *hash, unsigned char *out);
	/** Self-test
	@return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
	*/
	int (*test)(void);
} hash_descriptor[];

int md5_init(hash_state * md);
int md5_process(hash_state * md, const unsigned char *in, unsigned long inlen);
int md5_done(hash_state * md, unsigned char *out);
int md5_test(void);
extern const struct ltc_hash_descriptor md5_desc;

/* a simple macro for making hash "process" functions */
#define HASH_PROCESS(func_name, compress_name, state_var, block_size)                       \
	int func_name (hash_state * md, const unsigned char *in, unsigned long inlen)               \
{                                                                                           \
	unsigned long n;                                                                        \
	int           err;                                                                      \
	if (md-> state_var .curlen > sizeof(md-> state_var .buf)) {                             \
	return -1;                                                            \
	}                                                                                       \
	while (inlen > 0) {                                                                       \
	if (md-> state_var .curlen == 0 && inlen >= block_size) {                             \
	if ((err = compress_name (md, (unsigned char *)in)) != CRYPT_OK) { \
	return err;         \
	}                                        \
	md-> state_var .length += block_size * 8;                                        \
	in             += block_size;                                                   \
	inlen          -= block_size;                                                   \
	} else {                                                                            \
	n = __min(inlen, (block_size - md-> state_var .curlen));                             \
	memcpy(md-> state_var .buf + md-> state_var.curlen, in, (size_t)n);             \
	md-> state_var .curlen += n;                                                     \
	in             += n;                                                            \
	inlen          -= n;                                                            \
	if (md-> state_var .curlen == block_size) {                                      \
	if ((err = compress_name (md, md-> state_var .buf)) != CRYPT_OK) {\
	return err;                                      \
	} \
	md-> state_var .length += 8*block_size;                                       \
	md-> state_var .curlen = 0;                                                   \
	}                                                                                \
	}                                                                                    \
	}                                                                                       \
	return CRYPT_OK;                                                                        \
}


#define ROL(x, y) ( (((unsigned long)(x)<<(unsigned long)((y)&31)) | (((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define ROR(x, y) ( ((((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)((y)&31)) | ((unsigned long)(x)<<(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define ROLc(x, y) ( (((unsigned long)(x)<<(unsigned long)((y)&31)) | (((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define RORc(x, y) ( ((((unsigned long)(x)&0xFFFFFFFFUL)>>(unsigned long)((y)&31)) | ((unsigned long)(x)<<(unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)

#define STORE32L(x, y)                                                                     \
{ (y)[3] = (unsigned char)(((x)>>24)&255); (y)[2] = (unsigned char)(((x)>>16)&255);   \
	(y)[1] = (unsigned char)(((x)>>8)&255); (y)[0] = (unsigned char)((x)&255); }

#define LOAD32L(x, y)                            \
{ x = ((unsigned long)((y)[3] & 255)<<24) | \
	((unsigned long)((y)[2] & 255)<<16) | \
	((unsigned long)((y)[1] & 255)<<8)  | \
	((unsigned long)((y)[0] & 255)); }

#define STORE64L(x, y)                                                                     \
{ (y)[7] = (unsigned char)(((x)>>56)&255); (y)[6] = (unsigned char)(((x)>>48)&255);   \
	(y)[5] = (unsigned char)(((x)>>40)&255); (y)[4] = (unsigned char)(((x)>>32)&255);   \
	(y)[3] = (unsigned char)(((x)>>24)&255); (y)[2] = (unsigned char)(((x)>>16)&255);   \
	(y)[1] = (unsigned char)(((x)>>8)&255); (y)[0] = (unsigned char)((x)&255); }

#define LOAD64L(x, y)                                                       \
{ x = (((ulong64)((y)[7] & 255))<<56)|(((ulong64)((y)[6] & 255))<<48)| \
	(((ulong64)((y)[5] & 255))<<40)|(((ulong64)((y)[4] & 255))<<32)| \
	(((ulong64)((y)[3] & 255))<<24)|(((ulong64)((y)[2] & 255))<<16)| \
	(((ulong64)((y)[1] & 255))<<8)|(((ulong64)((y)[0] & 255))); }

#define STORE32H(x, y)                                                                     \
{ (y)[0] = (unsigned char)(((x)>>24)&255); (y)[1] = (unsigned char)(((x)>>16)&255);   \
	(y)[2] = (unsigned char)(((x)>>8)&255); (y)[3] = (unsigned char)((x)&255); }

#define LOAD32H(x, y)                            \
{ x = ((unsigned long)((y)[0] & 255)<<24) | \
	((unsigned long)((y)[1] & 255)<<16) | \
	((unsigned long)((y)[2] & 255)<<8)  | \
	((unsigned long)((y)[3] & 255)); }

#define STORE64H(x, y)                                                                     \
{ (y)[0] = (unsigned char)(((x)>>56)&255); (y)[1] = (unsigned char)(((x)>>48)&255);     \
	(y)[2] = (unsigned char)(((x)>>40)&255); (y)[3] = (unsigned char)(((x)>>32)&255);     \
	(y)[4] = (unsigned char)(((x)>>24)&255); (y)[5] = (unsigned char)(((x)>>16)&255);     \
	(y)[6] = (unsigned char)(((x)>>8)&255); (y)[7] = (unsigned char)((x)&255); }

#define LOAD64H(x, y)                                                      \
{ x = (((ulong64)((y)[0] & 255))<<56)|(((ulong64)((y)[1] & 255))<<48) | \
	(((ulong64)((y)[2] & 255))<<40)|(((ulong64)((y)[3] & 255))<<32) | \
	(((ulong64)((y)[4] & 255))<<24)|(((ulong64)((y)[5] & 255))<<16) | \
	(((ulong64)((y)[6] & 255))<<8)|(((ulong64)((y)[7] & 255))); }


#define CRYPT_OK 1

#define F(x,y,z)  (z ^ (x & (y ^ z)))
#define G(x,y,z)  (y ^ (z & (y ^ x)))
#define H(x,y,z)  (x^y^z)
#define I(x,y,z)  (y^(x|(~z)))


#define FF(a,b,c,d,M,s,t) \
	a = (a + F(b,c,d) + M + t); a = ROL(a, s) + b;

#define GG(a,b,c,d,M,s,t) \
	a = (a + G(b,c,d) + M + t); a = ROL(a, s) + b;

#define HH(a,b,c,d,M,s,t) \
	a = (a + H(b,c,d) + M + t); a = ROL(a, s) + b;

#define II(a,b,c,d,M,s,t) \
	a = (a + I(b,c,d) + M + t); a = ROL(a, s) + b;

static const unsigned char Worder[64] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
	1,6,11,0,5,10,15,4,9,14,3,8,13,2,7,12,
	5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2,
	0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9
};

static const unsigned char Rorder[64] = {
	7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
	5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
	4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
	6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
};

static const ulong32 Korder[64] = {
	0xd76aa478UL, 0xe8c7b756UL, 0x242070dbUL, 0xc1bdceeeUL, 0xf57c0fafUL, 0x4787c62aUL, 0xa8304613UL, 0xfd469501UL,
	0x698098d8UL, 0x8b44f7afUL, 0xffff5bb1UL, 0x895cd7beUL, 0x6b901122UL, 0xfd987193UL, 0xa679438eUL, 0x49b40821UL,
	0xf61e2562UL, 0xc040b340UL, 0x265e5a51UL, 0xe9b6c7aaUL, 0xd62f105dUL, 0x02441453UL, 0xd8a1e681UL, 0xe7d3fbc8UL,
	0x21e1cde6UL, 0xc33707d6UL, 0xf4d50d87UL, 0x455a14edUL, 0xa9e3e905UL, 0xfcefa3f8UL, 0x676f02d9UL, 0x8d2a4c8aUL,
	0xfffa3942UL, 0x8771f681UL, 0x6d9d6122UL, 0xfde5380cUL, 0xa4beea44UL, 0x4bdecfa9UL, 0xf6bb4b60UL, 0xbebfbc70UL,
	0x289b7ec6UL, 0xeaa127faUL, 0xd4ef3085UL, 0x04881d05UL, 0xd9d4d039UL, 0xe6db99e5UL, 0x1fa27cf8UL, 0xc4ac5665UL,
	0xf4292244UL, 0x432aff97UL, 0xab9423a7UL, 0xfc93a039UL, 0x655b59c3UL, 0x8f0ccc92UL, 0xffeff47dUL, 0x85845dd1UL,
	0x6fa87e4fUL, 0xfe2ce6e0UL, 0xa3014314UL, 0x4e0811a1UL, 0xf7537e82UL, 0xbd3af235UL, 0x2ad7d2bbUL, 0xeb86d391UL
};



static int  md5_compress(hash_state *md, unsigned char *buf)
{
	ulong32 i, W[16], a, b, c, d;
	ulong32 t;

	/* copy the state into 512-bits into W[0..15] */
	for (i = 0; i < 16; i++) {
		LOAD32L(W[i], buf + (4*i));
	}

	/* copy state */
	a = md->md5.state[0];
	b = md->md5.state[1];
	c = md->md5.state[2];
	d = md->md5.state[3];

	for (i = 0; i < 16; ++i) {
		FF(a,b,c,d,W[Worder[i]],Rorder[i],Korder[i]);
		t = d; d = c; c = b; b = a; a = t;
	}

	for (; i < 32; ++i) {
		GG(a,b,c,d,W[Worder[i]],Rorder[i],Korder[i]);
		t = d; d = c; c = b; b = a; a = t;
	}

	for (; i < 48; ++i) {
		HH(a,b,c,d,W[Worder[i]],Rorder[i],Korder[i]);
		t = d; d = c; c = b; b = a; a = t;
	}

	for (; i < 64; ++i) {
		II(a,b,c,d,W[Worder[i]],Rorder[i],Korder[i]);
		t = d; d = c; c = b; b = a; a = t;
	}

	md->md5.state[0] = md->md5.state[0] + a;
	md->md5.state[1] = md->md5.state[1] + b;
	md->md5.state[2] = md->md5.state[2] + c;
	md->md5.state[3] = md->md5.state[3] + d;

	return CRYPT_OK;
}

/**
Initialize the hash state
@param md   The hash state you wish to initialize
@return CRYPT_OK if successful
*/
int md5_init(hash_state * md)
{
	//   LTC_ARGCHK(md != nullptr);
	md->md5.state[0] = 0x67452301UL;
	md->md5.state[1] = 0xefcdab89UL;
	md->md5.state[2] = 0x98badcfeUL;
	md->md5.state[3] = 0x10325476UL;
	md->md5.curlen = 0;
	md->md5.length = 0;
	return CRYPT_OK;
}

/**
Process a block of memory though the hash
@param md     The hash state
@param in     The data to hash
@param inlen  The length of the data (octets)
@return CRYPT_OK if successful
*/
HASH_PROCESS(md5_process, md5_compress, md5, 64)

	/**
	Terminate the hash to get the digest
	@param md  The hash state
	@param out [out] The destination of the hash (16 bytes)
	@return CRYPT_OK if successful
	*/
	int md5_done(hash_state * md, unsigned char *out)
{
	int i;

	//    LTC_ARGCHK(md  != nullptr);
	//    LTC_ARGCHK(out != nullptr);

	if (md->md5.curlen >= sizeof(md->md5.buf)) {
		return -1;
	}


	/* increase the length of the message */
	md->md5.length += md->md5.curlen * 8;

	/* append the '1' bit */
	md->md5.buf[md->md5.curlen++] = (unsigned char)0x80;

	/* if the length is currently above 56 bytes we append zeros
	* then compress.  Then we can fall back to padding zeros and length
	* encoding like normal.
	*/
	if (md->md5.curlen > 56) {
		while (md->md5.curlen < 64) {
			md->md5.buf[md->md5.curlen++] = (unsigned char)0;
		}
		md5_compress(md, md->md5.buf);
		md->md5.curlen = 0;
	}

	/* pad upto 56 bytes of zeroes */
	while (md->md5.curlen < 56) {
		md->md5.buf[md->md5.curlen++] = (unsigned char)0;
	}

	/* store length */
	STORE64L(md->md5.length, md->md5.buf+56);
	md5_compress(md, md->md5.buf);

	/* copy output */
	for (i = 0; i < 4; i++) {
		STORE32L(md->md5.state[i], out+(4*i));
	}

	return CRYPT_OK;
}

/**
Self-test the hash
@return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
*/  
int  md5_test(void)
{ 
	static const struct {
		const char *msg;
		unsigned char hash[16];
	} tests[] = {
		{ "",
		{ 0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04, 
		0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e } },
		{ "a",
		{0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8, 
		0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61 } },
		{ "abc",
		{ 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 
		0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 } },
		{ "message digest", 
		{ 0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d, 
		0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0 } }, 
		{ "abcdefghijklmnopqrstuvwxyz",
		{ 0xc3, 0xfc, 0xd3, 0xd7, 0x61, 0x92, 0xe4, 0x00, 
		0x7d, 0xfb, 0x49, 0x6c, 0xca, 0x67, 0xe1, 0x3b } },
		{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
		{ 0xd1, 0x74, 0xab, 0x98, 0xd2, 0x77, 0xd9, 0xf5, 
		0xa5, 0x61, 0x1c, 0x2c, 0x9f, 0x41, 0x9d, 0x9f } },
		{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
		{ 0x57, 0xed, 0xf4, 0xa2, 0x2b, 0xe3, 0xc9, 0x55, 
		0xac, 0x49, 0xda, 0x2e, 0x21, 0x07, 0xb6, 0x7a } }, 
		{ nullptr, { 0 } }
	};

	int i;
	unsigned char tmp[16];
	hash_state md;

	for (i = 0; tests[i].msg != nullptr; i++) {
		md5_init(&md);
		md5_process(&md, (unsigned char *)tests[i].msg, (unsigned long)strlen(tests[i].msg));
		md5_done(&md, tmp);
		if (memcmp(tmp, tests[i].hash, 16) != 0) {
			return -1;
		}
	}
	return CRYPT_OK;
}

static char encoding_table[] = 
{	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', '+', '/' };

static char decoding_table[256] = {0};
static int mod_table[] = {0, 2, 1};

namespace base
{
	void md5( const char* szSrc, char* szOut, size_t nOutLen )
	{
		unsigned char tmp[16];
		hash_state md;

		md5_init(&md);
		md5_process(&md, (unsigned char *)szSrc, (unsigned long)strlen(szSrc));
		md5_done(&md, tmp);
		memcpy(szOut, tmp, nOutLen);
	}

	bool base64Encode( const char* szIn, size_t nInLen, char* szOut, size_t& nOutLen )
	{
		if( nullptr == szIn || nInLen <= 0 )
			return false;

		const size_t req_len = (size_t) (4.0 * ceil((double) nInLen / 3.0));
		if( nOutLen < req_len )
		{
			nOutLen = req_len;
			return false;
		}
		nOutLen = req_len;

		if( nullptr == szOut )
			return false;

		for( uint32_t i = 0, j = 0; i < nInLen; )
		{
			uint32_t octet_a = i < nInLen ? szIn[i++] : 0;
			uint32_t octet_b = i < nInLen ? szIn[i++] : 0;
			uint32_t octet_c = i < nInLen ? szIn[i++] : 0;

			uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

			szOut[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
			szOut[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
			szOut[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
			szOut[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
		}

		for( int32_t i = 0; i < mod_table[nInLen % 3]; ++i )
		{
			szOut[nOutLen - 1 - i] = '=';
		}

		return true;
	}

	bool base64Decode( const char* szIn, size_t nInLen, char* szOut, size_t& nOutLen )
	{
		if( nullptr == szIn || nInLen <= 0 )
			return false;

		if( decoding_table[(int32_t)encoding_table[0x40 - 1]] == 0 )
		{
			for( uint32_t i = 0; i < 0x40; ++i )
			{
				decoding_table[(int32_t)encoding_table[i]] = (char)i;
			}
		}

		if( nInLen % 4 != 0 )
			return nullptr;
		
		size_t req_out_len = nInLen / 4 * 3;
		if( szIn[nInLen - 1] == '=' ) req_out_len--;
		if( szIn[nInLen - 2] == '=' ) req_out_len--;

		if( nOutLen < req_out_len )
		{
			nOutLen = req_out_len;
			return false;
		}
		nOutLen = req_out_len;

		if( nullptr == szOut )
			return false;

		for( uint32_t i = 0, j = 0; i < nInLen; )
		{

			uint32_t sextet_a = szIn[i] == '=' ? 0 & i++ : decoding_table[(int32_t)szIn[i++]];
			uint32_t sextet_b = szIn[i] == '=' ? 0 & i++ : decoding_table[(int32_t)szIn[i++]];
			uint32_t sextet_c = szIn[i] == '=' ? 0 & i++ : decoding_table[(int32_t)szIn[i++]];
			uint32_t sextet_d = szIn[i] == '=' ? 0 & i++ : decoding_table[(int32_t)szIn[i++]];

			uint32_t triple = (sextet_a << 3 * 6)
				+ (sextet_b << 2 * 6)
				+ (sextet_c << 1 * 6)
				+ (sextet_d << 0 * 6);

			if( j < nOutLen ) szOut[j++] = (triple >> 2 * 8) & 0xFF;
			if( j < nOutLen ) szOut[j++] = (triple >> 1 * 8) & 0xFF;
			if( j < nOutLen ) szOut[j++] = (triple >> 0 * 8) & 0xFF;
		}

		return true;
	}

	static const char* s_hex_table = "0123456789ABCDEF";

	bool hexEncode( const char* szIn, size_t nInLen, char* szOut, size_t& nOutLen )
	{
		if( nullptr == szIn || nInLen <= 0 )
			return false;

		if( nOutLen < nInLen*2 )
		{
			nOutLen = nInLen*2;
			return false;
		}
		nOutLen = nInLen * 2;
		if( nullptr == szOut )
			return false;

		for( uint32_t i = 0; i < nInLen; ++i )
		{
			szOut[i*2]		=	s_hex_table[(szIn[i] >> 4) & 0xF];
			szOut[i*2+1]	=	s_hex_table[szIn[i]&0xF];
		}

		return true;
	}

	bool hexDecode(const char* szIn, size_t nInLen, char* szOut, size_t& nOutLen)
	{
		if( nullptr == szIn || nInLen <= 0 )
			return false;

		if( nInLen % 2 != 0 )
			return false;

		if( nOutLen < nInLen / 2 )
		{
			nOutLen = nInLen / 2;
			return false;
		}

		nOutLen = nInLen / 2;
		if( nullptr == szOut )
			return false;

		for( uint32_t i = 0; i < nInLen; ++i )
		{
			char ch = szIn[i];
			if (::isdigit(ch))
				ch -= '0';
			else if (ch >= 'A' || ch <= 'F')
				ch = 10 + (ch - 'A');
			else if (ch >= 'a' || ch <= 'f')
				ch = 10 + (ch - 'a');
			else
				return false;

			if( i % 2 == 0 )
				szOut[i / 2] = ch << 4;
			else
				szOut[i / 2] += ch;
		}
		return true;
	}
}
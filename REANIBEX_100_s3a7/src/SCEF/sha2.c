/*
 *  FIPS-180-2 compliant SHA-256 implementation
 *
 *  Copyright (C) 2006-2010, Brainspark B.V.
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 *  The SHA-256 Secure Hash Standard was published by NIST in 2002.
 *
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */
#define SSH_2_M
#include "SCEF/sha2.h"

//#include "osl.h"
#if !defined(WITHOUT_FILE_SYSTEM)
	#include <stdio.h>
#endif

//#define VERBOSE_FULL

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
	{                                                       \
		(n) = ( (unsigned long) (b)[(i)    ] << 24 )        \
		      | ( (unsigned long) (b)[(i) + 1] << 16 )        \
		      | ( (unsigned long) (b)[(i) + 2] <<  8 )        \
		      | ( (unsigned long) (b)[(i) + 3]       );       \
	}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
	{                                                       \
		(b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
		(b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
		(b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
		(b)[(i) + 3] = (unsigned char) ( (n)       );       \
	}
#endif

/*
 * SHA-256 context setup
 */
void sha2_starts( sha2_context *ctx, int is224 )
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	if( is224 == 0 )
	{
		/* SHA-256 */
		ctx->state[0] = 0x6A09E667;
		ctx->state[1] = 0xBB67AE85;
		ctx->state[2] = 0x3C6EF372;
		ctx->state[3] = 0xA54FF53A;
		ctx->state[4] = 0x510E527F;
		ctx->state[5] = 0x9B05688C;
		ctx->state[6] = 0x1F83D9AB;
		ctx->state[7] = 0x5BE0CD19;
	}
	else
	{
		/* SHA-224 */
		ctx->state[0] = 0xC1059ED8;
		ctx->state[1] = 0x367CD507;
		ctx->state[2] = 0x3070DD17;
		ctx->state[3] = 0xF70E5939;
		ctx->state[4] = 0xFFC00B31;
		ctx->state[5] = 0x68581511;
		ctx->state[6] = 0x64F98FA7;
		ctx->state[7] = 0xBEFA4FA4;
	}

	ctx->is224 = is224;
#if (defined(VERBOSE_FULL))
	{
		unsigned long a;
		unsigned char b[4];
		printf("init states:\r\n");
		printf("ctx->states= %lx,%lx,%lx,%lx,%lx,%lx,%lx,%lx\r\n", ctx->state[0], ctx->state[1], ctx->state[2], ctx->state[3], ctx->state[4], ctx->state[5], ctx->state[6], ctx->state[7]);
		b[0] = 0x61;
		b[1] = 0x62;
		b[2] = 0x63;
		b[3] = 0x80;
		GET_ULONG_BE( a, b,  0 );
		printf("GET_ULONG_BE(%0lx,%02x%02x%02x%02x,0)\r\n", a, b[0], b[1], b[2], b[3]);
	}
#endif
}

static void sha2_process( sha2_context *ctx, const unsigned char data[64] )
{
	unsigned long temp1, temp2, W[64];
	unsigned long A, B, C, D, E, F, G, H;

	GET_ULONG_BE( W[ 0], data,  0 );
	GET_ULONG_BE( W[ 1], data,  4 );
	GET_ULONG_BE( W[ 2], data,  8 );
	GET_ULONG_BE( W[ 3], data, 12 );
	GET_ULONG_BE( W[ 4], data, 16 );
	GET_ULONG_BE( W[ 5], data, 20 );
	GET_ULONG_BE( W[ 6], data, 24 );
	GET_ULONG_BE( W[ 7], data, 28 );
	GET_ULONG_BE( W[ 8], data, 32 );
	GET_ULONG_BE( W[ 9], data, 36 );
	GET_ULONG_BE( W[10], data, 40 );
	GET_ULONG_BE( W[11], data, 44 );
	GET_ULONG_BE( W[12], data, 48 );
	GET_ULONG_BE( W[13], data, 52 );
	GET_ULONG_BE( W[14], data, 56 );
	GET_ULONG_BE( W[15], data, 60 );

#define  SHR(x,n) (((long)x & 0xFFFFFFFF) >> n)
#define ROTR(x,n) (SHR(x,n) | ((long)x << (32 - n)))

#define S0(x) (ROTR(x, 7) ^ ROTR(x,18) ^  SHR(x, 3))
#define S1(x) (ROTR(x,17) ^ ROTR(x,19) ^  SHR(x,10))

#define S2(x) (ROTR(x, 2) ^ ROTR(x,13) ^ ROTR(x,22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x,11) ^ ROTR(x,25))

#define F0(x,y,z) (((long)x & (long)y) | ((long)z & (long)(x | (long)y)))
#define F1(x,y,z) ((long)z ^ ((long)x & ((long)y ^ (long)z)))

#define R(t)                                    \
	(                                               \
	        W[t] = S1(W[t -  2]) + W[t -  7] +          \
	               S0(W[t - 15]) + W[t - 16]            \
	)

#define P(a,b,c,d,e,f,g,h,x,K)                  \
	{                                               \
		temp1 = h + S3(e) + F1(e,f,g) + K + x;      \
		temp2 = S2(a) + F0(a,b,c);                  \
		d += temp1; h = temp1 + temp2;              \
	}
	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];
	F = ctx->state[5];
	G = ctx->state[6];
	H = ctx->state[7];

	P( A, B, C, D, E, F, G, H, W[ 0], 0x428A2F98 );
#if defined(VERBOSE_FULL)
	{
		int aux_i;
		printf("W[0]=%lx,W[1]=%lx,W[2]=%lx,W[3]=%lx,W[4]=%lx,W[5]=%lx,W[6]=%lx\r\n", W[0], W[1], W[2], W[3], W[4], W[5], W[6]);

		for(aux_i = 0; aux_i < 64; aux_i++)
		{
			if ((aux_i % 4) == 0) printf("\r\n");

			printf("\t[%d]:%02x", aux_i, data[aux_i]);
		}

		printf("\r\n");
		fflush(stdout);

		printf("states prev process 1:\r\n");
		printf("ctx->states= %lx,%lx,%lx,%lx,%lx,%lx,%lx,%lx\r\n", ctx->state[0], ctx->state[1], ctx->state[2], ctx->state[3], ctx->state[4], ctx->state[5], ctx->state[6], ctx->state[7]);
		printf("temp1=%lx,temp2=%lx,D=%lx,H=%lx,W[0]=%lx\r\n", temp1, temp2, D, H, W[0]);

		printf("SHR(0x1234500,16)=%lx\r\n", SHR(0x1234500, 16));
		printf("ROTR(0x1234578,16)=%lx\r\n", ROTR(0x1234578, 16));
		printf("S0(0x1234578)=%lx\r\n", S0(0x1234578));
		printf("S1(0x1234578)=%lx\r\n", S1(0x1234578));
		printf("S2(0x1234578)=%lx\r\n", S2(0x1234578));
		printf("S3(0x1234578)=%lx\r\n", S3(0x1234578));
		printf("F0(0x1234578,0xAAAAAA,0x555555)=%lx\r\n", F0(0x1234578, 0xAAAAAA, 0x555555));
		printf("F1(0x1234578,0xAAAAAA,0x555555)=%lx\r\n", F1(0x1234578, 0xAAAAAA, 0x555555));
		fflush(stdout);
	}
#endif
	P( H, A, B, C, D, E, F, G, W[ 1], 0x71374491 );
	P( G, H, A, B, C, D, E, F, W[ 2], 0xB5C0FBCF );
	P( F, G, H, A, B, C, D, E, W[ 3], 0xE9B5DBA5 );
	P( E, F, G, H, A, B, C, D, W[ 4], 0x3956C25B );
	P( D, E, F, G, H, A, B, C, W[ 5], 0x59F111F1 );
	P( C, D, E, F, G, H, A, B, W[ 6], 0x923F82A4 );
	P( B, C, D, E, F, G, H, A, W[ 7], 0xAB1C5ED5 );
	P( A, B, C, D, E, F, G, H, W[ 8], 0xD807AA98 );
	P( H, A, B, C, D, E, F, G, W[ 9], 0x12835B01 );
	P( G, H, A, B, C, D, E, F, W[10], 0x243185BE );
	P( F, G, H, A, B, C, D, E, W[11], 0x550C7DC3 );
	P( E, F, G, H, A, B, C, D, W[12], 0x72BE5D74 );
	P( D, E, F, G, H, A, B, C, W[13], 0x80DEB1FE );
	P( C, D, E, F, G, H, A, B, W[14], 0x9BDC06A7 );
	P( B, C, D, E, F, G, H, A, W[15], 0xC19BF174 );
	P( A, B, C, D, E, F, G, H, R(16), 0xE49B69C1 );
	P( H, A, B, C, D, E, F, G, R(17), 0xEFBE4786 );
	P( G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6 );
	P( F, G, H, A, B, C, D, E, R(19), 0x240CA1CC );
	P( E, F, G, H, A, B, C, D, R(20), 0x2DE92C6F );
	P( D, E, F, G, H, A, B, C, R(21), 0x4A7484AA );
	P( C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DC );
	P( B, C, D, E, F, G, H, A, R(23), 0x76F988DA );
	P( A, B, C, D, E, F, G, H, R(24), 0x983E5152 );
	P( H, A, B, C, D, E, F, G, R(25), 0xA831C66D );
	P( G, H, A, B, C, D, E, F, R(26), 0xB00327C8 );
	P( F, G, H, A, B, C, D, E, R(27), 0xBF597FC7 );
	P( E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3 );
	P( D, E, F, G, H, A, B, C, R(29), 0xD5A79147 );
	P( C, D, E, F, G, H, A, B, R(30), 0x06CA6351 );
	P( B, C, D, E, F, G, H, A, R(31), 0x14292967 );
	P( A, B, C, D, E, F, G, H, R(32), 0x27B70A85 );
	P( H, A, B, C, D, E, F, G, R(33), 0x2E1B2138 );
	P( G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFC );
	P( F, G, H, A, B, C, D, E, R(35), 0x53380D13 );
	P( E, F, G, H, A, B, C, D, R(36), 0x650A7354 );
	P( D, E, F, G, H, A, B, C, R(37), 0x766A0ABB );
	P( C, D, E, F, G, H, A, B, R(38), 0x81C2C92E );
	P( B, C, D, E, F, G, H, A, R(39), 0x92722C85 );
	P( A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1 );
	P( H, A, B, C, D, E, F, G, R(41), 0xA81A664B );
	P( G, H, A, B, C, D, E, F, R(42), 0xC24B8B70 );
	P( F, G, H, A, B, C, D, E, R(43), 0xC76C51A3 );
	P( E, F, G, H, A, B, C, D, R(44), 0xD192E819 );
	P( D, E, F, G, H, A, B, C, R(45), 0xD6990624 );
	P( C, D, E, F, G, H, A, B, R(46), 0xF40E3585 );
	P( B, C, D, E, F, G, H, A, R(47), 0x106AA070 );
	P( A, B, C, D, E, F, G, H, R(48), 0x19A4C116 );
	P( H, A, B, C, D, E, F, G, R(49), 0x1E376C08 );
	P( G, H, A, B, C, D, E, F, R(50), 0x2748774C );
	P( F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5 );
	P( E, F, G, H, A, B, C, D, R(52), 0x391C0CB3 );
	P( D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4A );
	P( C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4F );
	P( B, C, D, E, F, G, H, A, R(55), 0x682E6FF3 );
	P( A, B, C, D, E, F, G, H, R(56), 0x748F82EE );
	P( H, A, B, C, D, E, F, G, R(57), 0x78A5636F );
	P( G, H, A, B, C, D, E, F, R(58), 0x84C87814 );
	P( F, G, H, A, B, C, D, E, R(59), 0x8CC70208 );
	P( E, F, G, H, A, B, C, D, R(60), 0x90BEFFFA );
	P( D, E, F, G, H, A, B, C, R(61), 0xA4506CEB );
	P( C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7 );
	P( B, C, D, E, F, G, H, A, R(63), 0xC67178F2 );

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;
	ctx->state[4] += E;
	ctx->state[5] += F;
	ctx->state[6] += G;
	ctx->state[7] += H;
#if (defined(VERBOSE_FULL))
	{
		printf("states prev process 1:\r\n");
		printf("ctx->states= %lx,%lx,%lx,%lx,%lx,%lx,%lx,%lx\r\n", ctx->state[0], ctx->state[1], ctx->state[2], ctx->state[3], ctx->state[4], ctx->state[5], ctx->state[6], ctx->state[7]);
	}
#endif
}

/*
 * SHA-256 process buffer
 */
void sha2_update( sha2_context *ctx, const unsigned char *input, size_t ilen )
{
	size_t fill;
	unsigned long left;

	if( ilen <= 0 )
		return;

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += (unsigned long) ilen;
	ctx->total[0] &= 0xFFFFFFFF;

	if( ctx->total[0] < (unsigned long) ilen )
		ctx->total[1]++;

#if defined(VERBOSE_FULL)
	printf("ctx->total[0]=%ld,left=%ld fill=%ld\r\n", ctx->total[0], left, (unsigned long)fill);
	fflush(stdout);
#endif

	if( left && ilen >= fill )
	{
		memcpy( (void *) (ctx->buffer + left),
		        (void *) input, fill );
		sha2_process( ctx, ctx->buffer );
		input += fill;
		ilen  -= fill;
		left = 0;
#if defined(VERBOSE_FULL)
		printf("ilen=%ld,left=%ld fill=%ld\r\n", (unsigned long)ilen, left, (unsigned long)fill);
		fflush(stdout);
#endif
	}

	while( ilen >= 64 )
	{
		sha2_process( ctx, input );
		input += 64;
		ilen  -= 64;
#if defined(VERBOSE_FULL)
		printf("while ilen=%ld\r\n", (unsigned long)ilen);
		fflush(stdout);
#endif
	}

	if( ilen > 0 )
	{
		memcpy( (void *) (ctx->buffer + left),
		        (void *) input, ilen );
	}
}

static const unsigned char sha2_padding[64] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SHA-256 final digest
 */
void sha2_finish( sha2_context *ctx, unsigned char output[32] )
{
	unsigned long last, padn;
	unsigned long high, low;
	unsigned char msglen[8];

	high = ( ctx->total[0] >> 29 )
	       | ( ctx->total[1] <<  3 );
	low  = ( ctx->total[0] <<  3 );

	PUT_ULONG_BE( high, msglen, 0 );
	PUT_ULONG_BE( low,  msglen, 4 );

	last = ctx->total[0] & 0x3F;
	padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

	sha2_update( ctx, (unsigned char *) sha2_padding, padn );
	sha2_update( ctx, msglen, 8 );

	PUT_ULONG_BE( ctx->state[0], output,  0 );
	PUT_ULONG_BE( ctx->state[1], output,  4 );
	PUT_ULONG_BE( ctx->state[2], output,  8 );
	PUT_ULONG_BE( ctx->state[3], output, 12 );
	PUT_ULONG_BE( ctx->state[4], output, 16 );
	PUT_ULONG_BE( ctx->state[5], output, 20 );
	PUT_ULONG_BE( ctx->state[6], output, 24 );

	if( ctx->is224 == 0 )
		PUT_ULONG_BE( ctx->state[7], output, 28 );
}

/*
 * output = SHA-256( input buffer )
 */
void sha2( const unsigned char *input, size_t ilen,
           unsigned char output[32], int is224 )
{
	sha2_context ctx;

	sha2_starts( &ctx, is224 );
	sha2_update( &ctx, input, ilen );
	sha2_finish( &ctx, output );

	memset( &ctx, 0, sizeof( sha2_context ) );
}

#if !defined(WITHOUT_FILE_SYSTEM)
/*
 * output = SHA-256( file contents )
 */
/*
int sha2_file( const char *path, unsigned char output[32], int is224 )
{
	FILE *f;
	size_t n;
	sha2_context ctx;
	unsigned char buf[1024];

	if( ( f = fopen( path, "rb" ) ) == NULL )
		return( ERR_SHA2_FILE_IO_ERROR );

	sha2_starts( &ctx, is224 );

	while( ( n = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
		sha2_update( &ctx, buf, n );

	sha2_finish( &ctx, output );

	memset( &ctx, 0, sizeof( sha2_context ) );

	if( ferror( f ) != 0 )
	{
		fclose( f );
		return( ERR_SHA2_FILE_IO_ERROR );
	}

	fclose( f );
	return( 0 );
}*/
#endif //WITHOUT_FILE_SYSTEM

/*
 * SHA-256 HMAC context setup
 */
void sha2_hmac_starts( sha2_context *ctx, const unsigned char *key, size_t keylen,
                       int is224 )
{
	size_t i;
	unsigned char sum[32];

	if( keylen > 64 )
	{
		sha2( key, keylen, sum, is224 );
		keylen = ( is224 ) ? 28 : 32;
		key = sum;
	}

	memset( ctx->ipad, 0x36, 64 );
	memset( ctx->opad, 0x5C, 64 );

	for( i = 0; i < keylen; i++ )
	{
		ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
		ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
	}

	sha2_starts( ctx, is224 );
	sha2_update( ctx, ctx->ipad, 64 );

	memset( sum, 0, sizeof( sum ) );
}

/*
 * SHA-256 HMAC process buffer
 */
void sha2_hmac_update( sha2_context *ctx, const unsigned char *input, size_t ilen )
{
	sha2_update( ctx, input, ilen );
}

/*
 * SHA-256 HMAC final digest
 */
void sha2_hmac_finish( sha2_context *ctx, unsigned char output[32] )
{
	int is224, hlen;
	unsigned char tmpbuf[32];

	is224 = ctx->is224;
	hlen = ( is224 == 0 ) ? 32 : 28;

	sha2_finish( ctx, tmpbuf );
	sha2_starts( ctx, is224 );
	sha2_update( ctx, ctx->opad, 64 );
	sha2_update( ctx, tmpbuf, hlen );
	sha2_finish( ctx, output );

	memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * SHA-256 HMAC context reset
 */
void sha2_hmac_reset( sha2_context *ctx )
{
	sha2_starts( ctx, ctx->is224 );
	sha2_update( ctx, ctx->ipad, 64 );
}

/*
 * output = HMAC-SHA-256( hmac key, input buffer )
 */
void sha2_hmac( const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char output[32], int is224 )
{
	sha2_context ctx;

	sha2_hmac_starts( &ctx, key, keylen, is224 );
	sha2_hmac_update( &ctx, input, ilen );
	sha2_hmac_finish( &ctx, output );

	memset( &ctx, 0, sizeof( sha2_context ) );
}




/*
 * Checkup routine
 */
/*
int sha2_self_test( int verbose )
{
	int i, j, k, buflen;
	static unsigned char buf[1024];
	unsigned char sha2sum[32];
	sha2_context ctx;

	for( i = 0; i < 6; i++ )
	{
		j = i % 3;
		k = i < 3;

#if !defined(WITHOUT_FILE_SYSTEM)

		if( verbose != 0 )
			printf( "  SHA-%d test #%d: ", 256 - k * 32, j + 1 );

#endif // !WITHOUT_FILE_SYSTEM 
		sha2_starts( &ctx, k );

		if( j == 2 )
		{
			memset( buf, 'a', buflen = 1000 );

			for( j = 0; j < 1000; j++ )
				sha2_update( &ctx, buf, buflen );
		}
		else
			sha2_update( &ctx, sha2_test_buf[j],
			             sha2_test_buflen[j] );

		sha2_finish( &ctx, sha2sum );
#if defined(VERBOSE_FULL)
		{
			int aux_i;
			printf("test=%s\r\n", sha2_test_buf[j]);
			printf("ctx.states= %lx,%lx,%lx,%lx,%lx,%lx,%lx,%lx\r\n", ctx.state[0], ctx.state[1], ctx.state[2], ctx.state[3], ctx.state[4], ctx.state[5], ctx.state[6], ctx.state[7]);
			printf("sha2sum vs sha2_test_sum[%d]:", i);

			for(aux_i = 0; aux_i < (32 - (k * 4)); aux_i++)
			{
				if ((aux_i % 4) == 0) printf("\r\n");

				printf("\t[%d]:%02x - %02x", aux_i, sha2sum[aux_i], sha2_test_sum[i][aux_i]);
			}

			printf("\r\n");
			fflush(stdout);
		}
#endif

		if( memcmp( sha2sum, sha2_test_sum[i], 32 - k * 4 ) != 0 )
		{
#if !defined(WITHOUT_FILE_SYSTEM)

			if( verbose != 0 )
				printf( "failed\n" );

#endif // !WITHOUT_FILE_SYSTEM 

			return( 1 );
		}

#if !defined(WITHOUT_FILE_SYSTEM)

		if( verbose != 0 )
			printf( "passed\n" );

#endif // !WITHOUT_FILE_SYSTEM 
	}

#if !defined(WITHOUT_FILE_SYSTEM)

	if( verbose != 0 )
		printf( "\n" );

#endif // !WITHOUT_FILE_SYSTEM 

	for( i = 0; i < 14; i++ )
	{
		j = i % 7;
		k = i < 7;

#if !defined(WITHOUT_FILE_SYSTEM)

		if( verbose != 0 )
			printf( "  HMAC-SHA-%d test #%d: ", 256 - k * 32, j + 1 );

#endif // !WITHOUT_FILE_SYSTEM 

		if( j == 5 || j == 6 )
		{
			memset( buf, '\xAA', buflen = 131 );
			sha2_hmac_starts( &ctx, buf, buflen, k );
		}
		else
			sha2_hmac_starts( &ctx, sha2_hmac_test_key[j],
			                  sha2_hmac_test_keylen[j], k );

		sha2_hmac_update( &ctx, sha2_hmac_test_buf[j],
		                  sha2_hmac_test_buflen[j] );

		sha2_hmac_finish( &ctx, sha2sum );

		buflen = ( j == 4 ) ? 16 : 32 - k * 4;

		if( memcmp( sha2sum, sha2_hmac_test_sum[i], buflen ) != 0 )
		{
#if !defined(WITHOUT_FILE_SYSTEM)

			if( verbose != 0 )
				printf( "failed\n" );

#endif // !WITHOUT_FILE_SYSTEM 

			return( 1 );
		}

#if !defined(WITHOUT_FILE_SYSTEM)

		if( verbose != 0 )
			printf( "passed\n" );

#endif // !WITHOUT_FILE_SYSTEM 
	}

#if !defined(WITHOUT_FILE_SYSTEM)

	if( verbose != 0 )
		printf( "\n" );

#endif // !WITHOUT_FILE_SYSTEM 

	return( 0 );
}
*/

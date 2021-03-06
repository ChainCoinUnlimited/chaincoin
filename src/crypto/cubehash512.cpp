// Copyright (c) 2007-2010  Projet RNRT SAPHIR
// Copyright (c) 2019 PM-Tech
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <crypto/cubehash512.h>

#include <stddef.h>
#include <string.h>
#include <limits.h>

static const sph_u32 IV512[] = {
    SPH_C32(0x2AEA2A61), SPH_C32(0x50F494D4), SPH_C32(0x2D538B8B),
    SPH_C32(0x4167D83E), SPH_C32(0x3FEE2313), SPH_C32(0xC701CF8C),
    SPH_C32(0xCC39968E), SPH_C32(0x50AC5695), SPH_C32(0x4D42C787),
    SPH_C32(0xA647A8B3), SPH_C32(0x97CF0BEF), SPH_C32(0x825B4537),
    SPH_C32(0xEEF864D2), SPH_C32(0xF22090C4), SPH_C32(0xD0E5CD33),
    SPH_C32(0xA23911AE), SPH_C32(0xFCD398D9), SPH_C32(0x148FE485),
    SPH_C32(0x1B017BEF), SPH_C32(0xB6444532), SPH_C32(0x6A536159),
    SPH_C32(0x2FF5781C), SPH_C32(0x91FA7934), SPH_C32(0x0DBADEA9),
    SPH_C32(0xD65C8A2B), SPH_C32(0xA5A70E75), SPH_C32(0xB1C62456),
    SPH_C32(0xBC796576), SPH_C32(0x1921C8F7), SPH_C32(0xE7989AF1),
    SPH_C32(0x7795D246), SPH_C32(0xD43E3B44)
};

#define T32      SPH_T32
#define ROTL32   SPH_ROTL32

#define DECL_STATE \
    sph_u32 x0, x1, x2, x3, x4, x5, x6, x7; \
    sph_u32 x8, x9, xa, xb, xc, xd, xe, xf; \
    sph_u32 xg, xh, xi, xj, xk, xl, xm, xn; \
    sph_u32 xo, xp, xq, xr, xs, xt, xu, xv;

#define READ_STATE(cc)   do { \
        x0 = cc.state[ 0]; \
        x1 = cc.state[ 1]; \
        x2 = cc.state[ 2]; \
        x3 = cc.state[ 3]; \
        x4 = cc.state[ 4]; \
        x5 = cc.state[ 5]; \
        x6 = cc.state[ 6]; \
        x7 = cc.state[ 7]; \
        x8 = cc.state[ 8]; \
        x9 = cc.state[ 9]; \
        xa = cc.state[10]; \
        xb = cc.state[11]; \
        xc = cc.state[12]; \
        xd = cc.state[13]; \
        xe = cc.state[14]; \
        xf = cc.state[15]; \
        xg = cc.state[16]; \
        xh = cc.state[17]; \
        xi = cc.state[18]; \
        xj = cc.state[19]; \
        xk = cc.state[20]; \
        xl = cc.state[21]; \
        xm = cc.state[22]; \
        xn = cc.state[23]; \
        xo = cc.state[24]; \
        xp = cc.state[25]; \
        xq = cc.state[26]; \
        xr = cc.state[27]; \
        xs = cc.state[28]; \
        xt = cc.state[29]; \
        xu = cc.state[30]; \
        xv = cc.state[31]; \
    } while (0)

#define WRITE_STATE(cc)   do { \
        cc.state[ 0] = x0; \
        cc.state[ 1] = x1; \
        cc.state[ 2] = x2; \
        cc.state[ 3] = x3; \
        cc.state[ 4] = x4; \
        cc.state[ 5] = x5; \
        cc.state[ 6] = x6; \
        cc.state[ 7] = x7; \
        cc.state[ 8] = x8; \
        cc.state[ 9] = x9; \
        cc.state[10] = xa; \
        cc.state[11] = xb; \
        cc.state[12] = xc; \
        cc.state[13] = xd; \
        cc.state[14] = xe; \
        cc.state[15] = xf; \
        cc.state[16] = xg; \
        cc.state[17] = xh; \
        cc.state[18] = xi; \
        cc.state[19] = xj; \
        cc.state[20] = xk; \
        cc.state[21] = xl; \
        cc.state[22] = xm; \
        cc.state[23] = xn; \
        cc.state[24] = xo; \
        cc.state[25] = xp; \
        cc.state[26] = xq; \
        cc.state[27] = xr; \
        cc.state[28] = xs; \
        cc.state[29] = xt; \
        cc.state[30] = xu; \
        cc.state[31] = xv; \
    } while (0)

#define INPUT_BLOCK   do { \
        x0 ^= sph_dec32le_aligned(buf +  0); \
        x1 ^= sph_dec32le_aligned(buf +  4); \
        x2 ^= sph_dec32le_aligned(buf +  8); \
        x3 ^= sph_dec32le_aligned(buf + 12); \
        x4 ^= sph_dec32le_aligned(buf + 16); \
        x5 ^= sph_dec32le_aligned(buf + 20); \
        x6 ^= sph_dec32le_aligned(buf + 24); \
        x7 ^= sph_dec32le_aligned(buf + 28); \
    } while (0)

#define ROUND_EVEN   do { \
        xg = T32(x0 + xg); \
        x0 = ROTL32(x0, 7); \
        xh = T32(x1 + xh); \
        x1 = ROTL32(x1, 7); \
        xi = T32(x2 + xi); \
        x2 = ROTL32(x2, 7); \
        xj = T32(x3 + xj); \
        x3 = ROTL32(x3, 7); \
        xk = T32(x4 + xk); \
        x4 = ROTL32(x4, 7); \
        xl = T32(x5 + xl); \
        x5 = ROTL32(x5, 7); \
        xm = T32(x6 + xm); \
        x6 = ROTL32(x6, 7); \
        xn = T32(x7 + xn); \
        x7 = ROTL32(x7, 7); \
        xo = T32(x8 + xo); \
        x8 = ROTL32(x8, 7); \
        xp = T32(x9 + xp); \
        x9 = ROTL32(x9, 7); \
        xq = T32(xa + xq); \
        xa = ROTL32(xa, 7); \
        xr = T32(xb + xr); \
        xb = ROTL32(xb, 7); \
        xs = T32(xc + xs); \
        xc = ROTL32(xc, 7); \
        xt = T32(xd + xt); \
        xd = ROTL32(xd, 7); \
        xu = T32(xe + xu); \
        xe = ROTL32(xe, 7); \
        xv = T32(xf + xv); \
        xf = ROTL32(xf, 7); \
        x8 ^= xg; \
        x9 ^= xh; \
        xa ^= xi; \
        xb ^= xj; \
        xc ^= xk; \
        xd ^= xl; \
        xe ^= xm; \
        xf ^= xn; \
        x0 ^= xo; \
        x1 ^= xp; \
        x2 ^= xq; \
        x3 ^= xr; \
        x4 ^= xs; \
        x5 ^= xt; \
        x6 ^= xu; \
        x7 ^= xv; \
        xi = T32(x8 + xi); \
        x8 = ROTL32(x8, 11); \
        xj = T32(x9 + xj); \
        x9 = ROTL32(x9, 11); \
        xg = T32(xa + xg); \
        xa = ROTL32(xa, 11); \
        xh = T32(xb + xh); \
        xb = ROTL32(xb, 11); \
        xm = T32(xc + xm); \
        xc = ROTL32(xc, 11); \
        xn = T32(xd + xn); \
        xd = ROTL32(xd, 11); \
        xk = T32(xe + xk); \
        xe = ROTL32(xe, 11); \
        xl = T32(xf + xl); \
        xf = ROTL32(xf, 11); \
        xq = T32(x0 + xq); \
        x0 = ROTL32(x0, 11); \
        xr = T32(x1 + xr); \
        x1 = ROTL32(x1, 11); \
        xo = T32(x2 + xo); \
        x2 = ROTL32(x2, 11); \
        xp = T32(x3 + xp); \
        x3 = ROTL32(x3, 11); \
        xu = T32(x4 + xu); \
        x4 = ROTL32(x4, 11); \
        xv = T32(x5 + xv); \
        x5 = ROTL32(x5, 11); \
        xs = T32(x6 + xs); \
        x6 = ROTL32(x6, 11); \
        xt = T32(x7 + xt); \
        x7 = ROTL32(x7, 11); \
        xc ^= xi; \
        xd ^= xj; \
        xe ^= xg; \
        xf ^= xh; \
        x8 ^= xm; \
        x9 ^= xn; \
        xa ^= xk; \
        xb ^= xl; \
        x4 ^= xq; \
        x5 ^= xr; \
        x6 ^= xo; \
        x7 ^= xp; \
        x0 ^= xu; \
        x1 ^= xv; \
        x2 ^= xs; \
        x3 ^= xt; \
    } while (0)

#define ROUND_ODD   do { \
        xj = T32(xc + xj); \
        xc = ROTL32(xc, 7); \
        xi = T32(xd + xi); \
        xd = ROTL32(xd, 7); \
        xh = T32(xe + xh); \
        xe = ROTL32(xe, 7); \
        xg = T32(xf + xg); \
        xf = ROTL32(xf, 7); \
        xn = T32(x8 + xn); \
        x8 = ROTL32(x8, 7); \
        xm = T32(x9 + xm); \
        x9 = ROTL32(x9, 7); \
        xl = T32(xa + xl); \
        xa = ROTL32(xa, 7); \
        xk = T32(xb + xk); \
        xb = ROTL32(xb, 7); \
        xr = T32(x4 + xr); \
        x4 = ROTL32(x4, 7); \
        xq = T32(x5 + xq); \
        x5 = ROTL32(x5, 7); \
        xp = T32(x6 + xp); \
        x6 = ROTL32(x6, 7); \
        xo = T32(x7 + xo); \
        x7 = ROTL32(x7, 7); \
        xv = T32(x0 + xv); \
        x0 = ROTL32(x0, 7); \
        xu = T32(x1 + xu); \
        x1 = ROTL32(x1, 7); \
        xt = T32(x2 + xt); \
        x2 = ROTL32(x2, 7); \
        xs = T32(x3 + xs); \
        x3 = ROTL32(x3, 7); \
        x4 ^= xj; \
        x5 ^= xi; \
        x6 ^= xh; \
        x7 ^= xg; \
        x0 ^= xn; \
        x1 ^= xm; \
        x2 ^= xl; \
        x3 ^= xk; \
        xc ^= xr; \
        xd ^= xq; \
        xe ^= xp; \
        xf ^= xo; \
        x8 ^= xv; \
        x9 ^= xu; \
        xa ^= xt; \
        xb ^= xs; \
        xh = T32(x4 + xh); \
        x4 = ROTL32(x4, 11); \
        xg = T32(x5 + xg); \
        x5 = ROTL32(x5, 11); \
        xj = T32(x6 + xj); \
        x6 = ROTL32(x6, 11); \
        xi = T32(x7 + xi); \
        x7 = ROTL32(x7, 11); \
        xl = T32(x0 + xl); \
        x0 = ROTL32(x0, 11); \
        xk = T32(x1 + xk); \
        x1 = ROTL32(x1, 11); \
        xn = T32(x2 + xn); \
        x2 = ROTL32(x2, 11); \
        xm = T32(x3 + xm); \
        x3 = ROTL32(x3, 11); \
        xp = T32(xc + xp); \
        xc = ROTL32(xc, 11); \
        xo = T32(xd + xo); \
        xd = ROTL32(xd, 11); \
        xr = T32(xe + xr); \
        xe = ROTL32(xe, 11); \
        xq = T32(xf + xq); \
        xf = ROTL32(xf, 11); \
        xt = T32(x8 + xt); \
        x8 = ROTL32(x8, 11); \
        xs = T32(x9 + xs); \
        x9 = ROTL32(x9, 11); \
        xv = T32(xa + xv); \
        xa = ROTL32(xa, 11); \
        xu = T32(xb + xu); \
        xb = ROTL32(xb, 11); \
        x0 ^= xh; \
        x1 ^= xg; \
        x2 ^= xj; \
        x3 ^= xi; \
        x4 ^= xl; \
        x5 ^= xk; \
        x6 ^= xn; \
        x7 ^= xm; \
        x8 ^= xp; \
        x9 ^= xo; \
        xa ^= xr; \
        xb ^= xq; \
        xc ^= xt; \
        xd ^= xs; \
        xe ^= xv; \
        xf ^= xu; \
    } while (0)

#define SIXTEEN_ROUNDS   do { \
        ROUND_EVEN; \
        ROUND_ODD; \
        ROUND_EVEN; \
        ROUND_ODD; \
        ROUND_EVEN; \
        ROUND_ODD; \
        ROUND_EVEN; \
        ROUND_ODD; \
        ROUND_EVEN; \
        ROUND_ODD; \
        ROUND_EVEN; \
        ROUND_ODD; \
        ROUND_EVEN; \
        ROUND_ODD; \
        ROUND_EVEN; \
        ROUND_ODD; \
    } while (0)

////// CCUBEHASH512

// Internal implementation code.
namespace
{
/// Internal CUBEHASH512 implementation.
namespace cubehash512
{

void inline Initialize(cubehash_context *sc, const sph_u32 *iv)
{
    memcpy(sc->state, iv, sizeof sc->state);
    sc->ptr = 0;
}

} // namespace cubehash512

} // namespace

CCUBEHASH512::CCUBEHASH512()
{
    cubehash512::Initialize(&s, IV512);
}

CCUBEHASH512& CCUBEHASH512::Write(const unsigned char* data, size_t len)
{
    unsigned char *buf;
    size_t ptr;
    DECL_STATE

    buf = s.buf;
    ptr = s.ptr;
    if (len < (sizeof s.buf) - ptr) {
        memcpy(buf + ptr, data, len);
        ptr += len;
        s.ptr = ptr;
        return *this;
    }

    READ_STATE(s);
    while (len > 0) {
        size_t clen;

        clen = (sizeof s.buf) - ptr;
        if (clen > len)
            clen = len;
        memcpy(buf + ptr, data, clen);
        ptr += clen;
        data = (const unsigned char *)data + clen;
        len -= clen;
        if (ptr == sizeof s.buf) {
            INPUT_BLOCK;
            SIXTEEN_ROUNDS;
            ptr = 0;
        }
    }
    WRITE_STATE(s);
    s.ptr = ptr;
    return *this;
}

void CCUBEHASH512::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    unsigned char *buf;
    size_t ptr;
    unsigned z;
    int i;
    DECL_STATE

    buf = s.buf;
    ptr = s.ptr;
    z = 0x80 >> 0;
    buf[ptr ++] = ((0 & -z) | z) & 0xFF;
    memset(buf + ptr, 0, (sizeof s.buf) - ptr);
    READ_STATE(s);
    INPUT_BLOCK;
    for (i = 0; i < 11; i ++) {
        SIXTEEN_ROUNDS;
        if (i == 0)
            xv ^= SPH_C32(1);
    }
    WRITE_STATE(s);
    for (z = 0; z < 16; z ++)
        sph_enc32le(hash + (z << 2), s.state[z]);
    Reset();
}

CCUBEHASH512& CCUBEHASH512::Reset()
{
    cubehash512::Initialize(&s, IV512);
    return *this;
}

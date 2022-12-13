/* libs/graphics/sgl/SkFilterProc.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkFilterProc.h"

/*  [1-x 1-y] [x 1-y]
    [1-x   y] [x   y]
*/

static unsigned bilerp00(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return a00; }
static unsigned bilerp01(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * a00 + a01) >> 2; }
static unsigned bilerp02(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + a01) >> 1; }
static unsigned bilerp03(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + 3 * a01) >> 2; }

static unsigned bilerp10(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * a00 + a10) >> 2; }
static unsigned bilerp11(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (9 * a00 + 3 * (a01 + a10) + a11) >> 4; }
static unsigned bilerp12(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * (a00 + a01) + a10 + a11) >> 3; }
static unsigned bilerp13(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (9 * a01 + 3 * (a00 + a11) + a10) >> 4; }

static unsigned bilerp20(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + a10) >> 1; }
static unsigned bilerp21(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * (a00 + a10) + a01 + a11) >> 3; }
static unsigned bilerp22(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + a01 + a10 + a11) >> 2; }
static unsigned bilerp23(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * (a01 + a11) + a00 + a10) >> 3; }

static unsigned bilerp30(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (a00 + 3 * a10) >> 2; }
static unsigned bilerp31(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (9 * a10 + 3 * (a00 + a11) + a01) >> 4; }
static unsigned bilerp32(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (3 * (a10 + a11) + a00 + a01) >> 3; }
static unsigned bilerp33(unsigned a00, unsigned a01, unsigned a10, unsigned a11) { return (9 * a11 + 3 * (a01 + a10) + a00) >> 4; }

static const SkFilterProc gBilerpProcs[4 * 4] = {
    bilerp00, bilerp01, bilerp02, bilerp03,
    bilerp10, bilerp11, bilerp12, bilerp13,
    bilerp20, bilerp21, bilerp22, bilerp23,
    bilerp30, bilerp31, bilerp32, bilerp33
};

const SkFilterProc* SkGetBilinearFilterProcTable()
{
    return gBilerpProcs;
}

#define MASK            0xFF00FF
#define LO_PAIR(x)      ((x) & MASK)
#define HI_PAIR(x)      (((x) >> 8) & MASK)
#define COMBINE(lo, hi) (((lo) & ~0xFF00) | (((hi) & ~0xFF00) << 8))

static unsigned bilerptr00(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    return *a00;
}
static unsigned bilerptr01(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;   
    uint32_t lo = (3 * LO_PAIR(c00) + LO_PAIR(c01)) >> 2;
    uint32_t hi = (3 * HI_PAIR(c00) + HI_PAIR(c01)) >> 2;
    return COMBINE(lo, hi);
}
static unsigned bilerptr02(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;   
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c01)) >> 1;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c01)) >> 1;
    return COMBINE(lo, hi);
}
static unsigned bilerptr03(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t lo = (LO_PAIR(c00) + 3 * LO_PAIR(c01)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + 3 * HI_PAIR(c01)) >> 2;
    return COMBINE(lo, hi);
}

static unsigned bilerptr10(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c10 = *a10;
    uint32_t lo = (3 * LO_PAIR(c00) + LO_PAIR(c10)) >> 2;
    uint32_t hi = (3 * HI_PAIR(c00) + HI_PAIR(c10)) >> 2;
    return COMBINE(lo, hi);
}
static unsigned bilerptr11(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (9 * LO_PAIR(c00) + 3 * (LO_PAIR(c01) + LO_PAIR(c10)) + LO_PAIR(c11)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c00) + 3 * (HI_PAIR(c01) + HI_PAIR(c10)) + HI_PAIR(c11)) >> 4;
    return COMBINE(lo, hi);
}
static unsigned bilerptr12(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (3 * (LO_PAIR(c00) + LO_PAIR(c01)) + LO_PAIR(c10) + LO_PAIR(c11)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c00) + HI_PAIR(c01)) + HI_PAIR(c10) + HI_PAIR(c11)) >> 3;
    return COMBINE(lo, hi);
}
static unsigned bilerptr13(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (9 * LO_PAIR(c01) + 3 * (LO_PAIR(c00) + LO_PAIR(c11)) + LO_PAIR(c10)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c01) + 3 * (HI_PAIR(c00) + HI_PAIR(c11)) + HI_PAIR(c10)) >> 4;
    return COMBINE(lo, hi);
}

static unsigned bilerptr20(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c10 = *a10;
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c10)) >> 1;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c10)) >> 1;
    return COMBINE(lo, hi);
}
static unsigned bilerptr21(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (3 * (LO_PAIR(c00) + LO_PAIR(c10)) + LO_PAIR(c01) + LO_PAIR(c11)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c00) + HI_PAIR(c10)) + HI_PAIR(c01) + HI_PAIR(c11)) >> 3;
    return COMBINE(lo, hi);
}
static unsigned bilerptr22(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (LO_PAIR(c00) + LO_PAIR(c01) + LO_PAIR(c10) + LO_PAIR(c11)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + HI_PAIR(c01) + HI_PAIR(c10) + HI_PAIR(c11)) >> 2;
    return COMBINE(lo, hi);
}
static unsigned bilerptr23(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (3 * (LO_PAIR(c01) + LO_PAIR(c11)) + LO_PAIR(c00) + LO_PAIR(c10)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c01) + HI_PAIR(c11)) + HI_PAIR(c00) + HI_PAIR(c10)) >> 3;
    return COMBINE(lo, hi);
}

static unsigned bilerptr30(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c10 = *a10;
    uint32_t lo = (LO_PAIR(c00) + 3 * LO_PAIR(c10)) >> 2;
    uint32_t hi = (HI_PAIR(c00) + 3 * HI_PAIR(c10)) >> 2;
    return COMBINE(lo, hi);
}
static unsigned bilerptr31(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (9 * LO_PAIR(c10) + 3 * (LO_PAIR(c00) + LO_PAIR(c11)) + LO_PAIR(c01)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c10) + 3 * (HI_PAIR(c00) + HI_PAIR(c11)) + HI_PAIR(c01)) >> 4;
    return COMBINE(lo, hi);
}
static unsigned bilerptr32(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (3 * (LO_PAIR(c10) + LO_PAIR(c11)) + LO_PAIR(c00) + LO_PAIR(c01)) >> 3;
    uint32_t hi = (3 * (HI_PAIR(c10) + HI_PAIR(c11)) + HI_PAIR(c00) + HI_PAIR(c01)) >> 3;
    return COMBINE(lo, hi);
}
static unsigned bilerptr33(const uint32_t* a00, const uint32_t* a01, const uint32_t* a10, const uint32_t* a11) {
    uint32_t c00 = *a00;
    uint32_t c01 = *a01;
    uint32_t c10 = *a10;
    uint32_t c11 = *a11;
    uint32_t lo = (9 * LO_PAIR(c11) + 3 * (LO_PAIR(c01) + LO_PAIR(c10)) + LO_PAIR(c00)) >> 4;
    uint32_t hi = (9 * HI_PAIR(c11) + 3 * (HI_PAIR(c01) + HI_PAIR(c10)) + HI_PAIR(c00)) >> 4;
    return COMBINE(lo, hi);
}

static const SkFilterPtrProc gBilerpPtrProcs[4 * 4] = {
    bilerptr00, bilerptr01, bilerptr02, bilerptr03,
    bilerptr10, bilerptr11, bilerptr12, bilerptr13,
    bilerptr20, bilerptr21, bilerptr22, bilerptr23,
    bilerptr30, bilerptr31, bilerptr32, bilerptr33
};

const SkFilterPtrProc* SkGetBilinearFilterPtrProcTable()
{
    return gBilerpPtrProcs;
}


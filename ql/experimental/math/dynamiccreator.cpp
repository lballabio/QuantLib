/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*
  Copyright (C) 2001-2009 Makoto Matsumoto and Takuji Nishimura.
  Copyright (C) 2009 Mutsuo Saito
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ql/experimental/math/dynamiccreator.hpp>
#include <cstring>

namespace QuantLib {

namespace mt_detail {
static int proper_mersenne_exponent(int p);
static mt_struct *alloc_mt_struct(int n);
}

MersenneTwisterDynamicRng::~MersenneTwisterDynamicRng() {
    mt_detail::free_mt_struct(m_);
}

void MersenneTwisterDynamicRng::resetSeed(const uint32_t seed) {
    uint32_t tmpSeed = seed;
    if (tmpSeed == 0)
        tmpSeed = SeedGenerator::instance().get();
    sgenrand_mt(tmpSeed, m_);
}

MersenneTwisterDynamicRng::sample_type MersenneTwisterDynamicRng::next() {
    return sample_type(nextReal(), 1.0);
}

Real MersenneTwisterDynamicRng::nextReal() {
    return (Real(nextInt32()) + 0.5) / (w_ == 32 ? 4294967296.0 : 2147483648.0);
}

unsigned long MersenneTwisterDynamicRng::nextInt32() {
    return static_cast<unsigned long>(genrand_mt(m_));
}

void MersenneTwisterDynamicRng::discard(uint64_t z) {
    for(;z!=0ULL;--z)
        genrand_mt(m_);
}

MersenneTwisterDynamicRng::MersenneTwisterDynamicRng(const int w, const int p,
                                                     const uint32_t creatorSeed,
                                                     const uint16_t id, const uint32_t seed)
    : w_(w), p_(p) {
    QL_REQUIRE(mt_detail::proper_mersenne_exponent(p) == 1,
               "exponent (" << p << ") not allowed");
    QL_REQUIRE(w == 31 || w == 32, "word size (" << w << ") must be 31 or 32");
    m_ = mt_detail::get_mt_parameter_id_st(w, p, id, creatorSeed);
    QL_REQUIRE(m_ != NULL, "could not create mt parameter");
    resetSeed(seed);
}

MersenneTwisterDynamicRng::MersenneTwisterDynamicRng(
    const MersenneTwisterDynamicRngDescription &desc, const uint32_t seed)
    : w_(desc.w), p_(desc.p) {
    m_ = mt_detail::alloc_mt_struct( (desc.p / desc.w +1) );
    m_->aaa = desc.aaa;
    m_->mm = desc.mm;
    m_->nn = desc.nn;
    m_->rr = desc.rr;
    m_->ww = desc.ww;
    m_->wmask = desc.wmask;
    m_->umask = desc.umask;
    m_->lmask = desc.lmask;
    m_->shift0 = desc.shift0;
    m_->shift1 = desc.shift1;
    m_->shiftB = desc.shiftB;
    m_->shiftC = desc.shiftC;
    m_->maskB = desc.maskB;
    m_->maskC = desc.maskC;
    m_->i = desc.i;
   resetSeed(seed);
}

MersenneTwisterDynamicRngDescription
MersenneTwisterDynamicRng::description() const {
    QL_REQUIRE(m_ != NULL,"not a valid mt instance");
    MersenneTwisterDynamicRngDescription des = {
        w_,         p_,         m_->aaa,   m_->mm,    m_->nn,     m_->rr,
        m_->ww,     m_->wmask,  m_->umask, m_->lmask, m_->shift0, m_->shift1,
        m_->shiftB, m_->shiftC, m_->maskB, m_->maskC, m_->i};
    return des;
}

// original code

namespace mt_detail {

// mt19937.c

/* Period parameters */
/* #define N 624 */
#define dcmt_M 397
#define MATRIX_A UINT32_C(0x9908b0df)   /* constant vector a */
#define UPPER_MASK UINT32_C(0x80000000) /* most significant w-r bits */
#define LOWER_MASK UINT32_C(0x7fffffff) /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B UINT32_C(0x9d2c5680)
#define TEMPERING_MASK_C UINT32_C(0xefc60000)
#define TEMPERING_SHIFT_U(y) (y >> 11)
#define TEMPERING_SHIFT_S(y) (y << 7)
#define TEMPERING_SHIFT_T(y) (y << 15)
#define TEMPERING_SHIFT_L(y) (y >> 18)

/* Initializing the array with a seed */
void _sgenrand_dc(_org_state *st, uint32_t seed) {
    int i;

    for (i = 0; i < dcmt_N; i++) {
        st->mt[i] = seed;
        seed = (UINT32_C(1812433253) * (seed ^ (seed >> 30))) + i + 1;
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
    }
    st->mti = dcmt_N;
}

uint32_t _genrand_dc(_org_state *st) {
    uint32_t y;
    static const uint32_t mag01[2] = {0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (st->mti >= dcmt_N) { /* generate N words at one time */
        int kk;

        for (kk = 0; kk < dcmt_N - dcmt_M; kk++) {
            y = (st->mt[kk] & UPPER_MASK) | (st->mt[kk + 1] & LOWER_MASK);
            st->mt[kk] = st->mt[kk + dcmt_M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (; kk < dcmt_N - 1; kk++) {
            y = (st->mt[kk] & UPPER_MASK) | (st->mt[kk + 1] & LOWER_MASK);
            st->mt[kk] = st->mt[kk + (dcmt_M - dcmt_N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (st->mt[dcmt_N - 1] & UPPER_MASK) | (st->mt[0] & LOWER_MASK);
        st->mt[dcmt_N - 1] = st->mt[dcmt_M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

        st->mti = 0;
    }

    y = st->mt[st->mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y;
}

// check32.c

#define LSB 0x1
#define WORDLEN 32

// #if defined(DEBUG)
// /* period parameters */
// #define NNN 17
// #define MMM 9
// #define WWW 32
// #define RRR 23
// #define NREPEAT 1000
// /********************/
// static uint32_t next_A(check32_t *ck, _org_state *st, int w);

// int main(int argc, char **argv)
// {
//     int i;
//     uint32_t a;
//     _org_state st;
//     check32_t ck;

//     _sgenrand_dc(&st, 3249);

//     for(i=0; i<NREPEAT; i++) {
//  a = next_A(&ck, &st, WWW);
//  if (IRRED == _CheckPeriod_dc(&ck, &st, a,MMM,NNN,RRR,WWW)) {
//      printf ("%x\n",a);
//  }
//     }

//     return 0;
// }
// #endif

void _InitCheck32_dc(check32_t *ck, int r, int w)
{
    int i;

    /* word_mask (least significant w bits) */
    ck->word_mask = 0xFFFFFFFF;
    ck->word_mask <<= WORDLEN - w;
    ck->word_mask >>= WORDLEN - w;
    /* lower_mask (least significant r bits) */
    for (ck->lower_mask=0,i=0; i<r; ++i) {
    ck->lower_mask <<= 1;
    ck->lower_mask |= LSB;
    }
    /* upper_mask (most significant (w-r) bits */
    ck->upper_mask = (~ck->lower_mask) & ck->word_mask;
}

// #if defined(DEBUG)
// static uint32_t next_A(check32_t *ck, _org_state *st, int w)
// {
//     uint32_t x;

//     x = _genrand_dc(st);

//     x &= ck->word_mask;
//     x |= (LSB << (w-1));

// /*  printf ("AAA %8x\n", x);
//   getchar();
// */
//     return x;
// }
// #endif

int _CheckPeriod_dc(check32_t *ck, _org_state *st,
            uint32_t a, int m, int n, int r, int w)
{
    int i, j, p, pp;
    uint32_t y, *x, *init, mat[2];


    p = n*w-r;
    x = (uint32_t*) malloc (2*p*sizeof(uint32_t));
    if (NULL==x) {
        //replace printf by ql exception
        QL_FAIL("malloc error in \"_CheckPeriod_dc()\"");
    //printf("malloc error in \"_CheckPeriod_dc()\"\n");
    // exit(1);
    }

    init = (uint32_t*) malloc (n*sizeof(uint32_t));
    if (NULL==init) {
        free(x);
        //replace printf by ql exception
        QL_FAIL("malloc error \"_CheckPeriod_dc()\"");
    //printf("malloc error \"_CheckPeriod_dc()\"\n");
    // free(x);
    // exit(1);
    }

    /* set initial values */
    for (i=0; i<n; ++i)
    x[i] = init[i] = (ck->word_mask & _genrand_dc(st));
    /* it is better that LSBs of x[2] and x[3] are different */
    if ( (x[2]&LSB) == (x[3]&LSB) ) {
    x[3] ^= 1;
    init[3] ^= 1;
    }

    pp = 2*p-n;
    mat[0] = 0; mat[1] = a;
    for (j=0; j<p; ++j) {

    /* generate */
    for (i=0; i<pp; ++i){
        y = (x[i]&ck->upper_mask) | (x[i+1]&ck->lower_mask);
        x[i+n] = x[i+m] ^ ( (y>>1) ^ mat[y&LSB] );
    }

    /* pick up odd subscritpt elements */
    for (i=2; i<=p; ++i)
        x[i] = x[(i<<1)-1];

    /* reverse generate */
    for (i=p-n; i>=0; --i) {
        y = x[i+n] ^ x[i+m] ^ mat[ x[i+1]&LSB ];
        y <<=1; y |= x[i+1]&LSB;

        x[i+1] = (x[i+1]&ck->upper_mask) | (y&ck->lower_mask);
        x[i] = (y&ck->upper_mask) | (x[i]&ck->lower_mask);
    }

    }

    if ((x[0]&ck->upper_mask)==(init[0]&ck->upper_mask)) {
    for (i=1; i<n; ++i) {
        if (x[i] != init[i])
        break;
    }
    if (i==n) {
        free(x); free(init);
        return IRRED;
    }
    }


    free(x); free(init);
    return REDU;
}

// eqdeg.c

/**************************************/
#define SSS 7
#define TTT 15
/* #define S00 11 */
#define S00 12
#define S01 18
/**************************************/

/** for get_tempering_parameter_hard **/
#define LIMIT_V_BEST_OPT 15
/**************************************/

#define WORD_LEN 32
#define MIN_INFINITE (-2147483647-1)

typedef struct {
    uint32_t *cf;  /* fraction part */              // status
    int start;     /* beginning of fraction part */ // idx
    int count;     /* maximum (degree) */
    uint32_t next; /* (bp) rm (shifted&bitmasked) at the maximum degree */
} Vector;

typedef struct mask_node{
    uint32_t b,c;
    int v,leng;
    struct mask_node *next;
} MaskNode;

static inline uint32_t trnstmp(eqdeg_t *eq, uint32_t tmp) {
    tmp ^= (tmp >> eq->shift_0) & eq->greal_mask;
    return tmp;
}

static inline uint32_t masktmp(eqdeg_t *eq, uint32_t tmp) {
    tmp ^= (tmp << eq->shift_s) & eq->mask_b;
    tmp ^= (tmp << eq->shift_t) & eq->mask_c;
    return tmp;
}

static inline uint32_t lsb(eqdeg_t *eq, uint32_t x) {
    return (x >> eq->ggap) & 1;
}

static const uint8_t pivot_calc_tbl[256] = {
    0, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    2, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    1, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    2, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
    4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
};

static int calc_pivot(uint32_t v);
static int push_stack(eqdeg_t *eq, uint32_t b, uint32_t c,
              int v, uint32_t *bbb, uint32_t *ccc);
static int push_mask(eqdeg_t * eq, int l, int v,
             uint32_t b, uint32_t c, uint32_t *bbb, uint32_t *ccc);
static int pivot_reduction(eqdeg_t *eq, int v);
static void init_tempering(eqdeg_t *eq, mt_struct *mts);
static void free_Vector( Vector *v );
static void free_lattice( Vector **lattice, int v);
static void add(int nnn, Vector *u, Vector *v);
static void optimize_v(eqdeg_t *eq, uint32_t b, uint32_t c, int v);
static MaskNode *optimize_v_hard(eqdeg_t *eq, int v, MaskNode *prev);
static Vector *new_Vector(int nnn);
static Vector **make_lattice(eqdeg_t *eq, int v);
static void delete_MaskNodes(MaskNode *head);
static MaskNode *delete_lower_MaskNodes(MaskNode *head, int l);
static MaskNode *cons_MaskNode(MaskNode *head, uint32_t b, uint32_t c, int leng);
/* static void count_MaskNodes(MaskNode *head); */
static void next_state(eqdeg_t *eq, Vector *v, int *count);

#if defined(DEBUG)
static void show_distrib(eqdeg_t *eq, mt_struct *mts);
#endif

#if defined(DEBUG)
int main(int argc, char **argv)
{
    mt_struct mt = {AAA,MMM,NNN,RRR,WWW,0,0,0,S00,SSS,TTT,0,0,0,NULL};

    get_tempering_parameter(&mt);

    return 0;
}
#endif

void _get_tempering_parameter_dc(mt_struct *mts)
{
    eqdeg_t eq;
    init_tempering(&eq, mts);
    optimize_v(&eq, 0, 0, 0);
    mts->shift0 = eq.shift_0;
    mts->shift1 = eq.shift_1;
    mts->shiftB = eq.shift_s;
    mts->shiftC = eq.shift_t;
    mts->maskB = eq.mask_b >> eq.ggap;
    mts->maskC = eq.mask_c >> eq.ggap;
}

void _get_tempering_parameter_hard_dc(mt_struct *mts)
{
    int i;
    MaskNode mn0, *cur, *next;
    eqdeg_t eq;

    init_tempering(&eq, mts);

    for (i=0; i<eq.www; i++)
    eq.gcur_maxlengs[i] = -1;

    mn0.b = mn0.c = mn0.leng = 0;
    mn0.next = NULL;

    cur = &mn0;
    for (i=0; i<LIMIT_V_BEST_OPT; i++) {
    next = optimize_v_hard(&eq, i, cur);
    if (i > 0)
        delete_MaskNodes(cur);
    cur = next;
    }
    delete_MaskNodes(cur);

    optimize_v(&eq, eq.gmax_b, eq.gmax_c,i);
    mts->shift0 = eq.shift_0;
    mts->shift1 = eq.shift_1;
    mts->shiftB = eq.shift_s;
    mts->shiftC = eq.shift_t;
    mts->maskB = eq.mask_b >> eq.ggap;
    mts->maskC = eq.mask_c >> eq.ggap;

    /* show_distrib(mts); */
}

static int calc_pivot(uint32_t v) {
    int p1, p2, p3, p4;

    p1 = pivot_calc_tbl[v & 0xff];
    if (p1) {
    return p1 + 24 - 1;
    }
    p2 = pivot_calc_tbl[(v >> 8) & 0xff];
    if (p2) {
    return p2 + 16 - 1;
    }
    p3 = pivot_calc_tbl[(v >> 16) & 0xff];
    if (p3) {
    return p3 + 8 - 1;
    }
    p4 = pivot_calc_tbl[(v >> 24) & 0xff];
    if (p4) {
    return p4 - 1;
    }
    return -1;
}

static int is_zero(int size, Vector *v) {
    if (v->cf[0] != 0) {
    return 0;
    } else {
    return (memcmp(v->cf, v->cf + 1, sizeof(uint32_t) * (size - 1)) == 0);
    }
}

static void init_tempering(eqdeg_t *eq, mt_struct *mts)
{
    int i;

    eq->mmm = mts->mm;
    eq->nnn = mts->nn;
    eq->rrr = mts->rr;
    eq->www = mts->ww;
    eq->shift_0 = S00;
    eq->shift_1 = S01;
    eq->shift_s = SSS;
    eq->shift_t = TTT;
    eq->ggap = WORD_LEN - eq->www;
    /* bits are filled in mts->aaa from MSB */
    eq->aaa[0] = 0; eq->aaa[1] = (mts->aaa) << eq->ggap;


    for( i=0; i<WORD_LEN; i++)
        eq->bitmask[i] = UINT32_C(0x80000000) >> i;

    for( i=0, eq->glower_mask=0; i<eq->rrr; i++)
    eq->glower_mask = (eq->glower_mask<<1)| 0x1;

    eq->gupper_mask = ~eq->glower_mask;
    eq->gupper_mask <<= eq->ggap;
    eq->glower_mask <<= eq->ggap;

    eq->greal_mask = (eq->gupper_mask | eq->glower_mask);

// comment out debug code
// #if defined(DEBUG)
//     printf ("n=%d m=%d r=%d w=%d\n", eq->nnn, eq->mmm, eq->rrr, eq->www);
//     printf ("nw-r=%d\n", eq->nnn * eq->www - eq->rrr);
//     printf ("a=%x(%x << %d)\n", eq->aaa[1],mts->aaa,eq->ggap);
//     printf ("upper (w-r) bit mask = %x\n", eq->gupper_mask);
//     printf ("lower r bit mask     = %x\n", eq->glower_mask);
//     printf ("w bit mask           = %x\n", eq->greal_mask);
//     fflush(stdout);
// #endif
}

/* (v-1) bitmasks of b,c */
static MaskNode *optimize_v_hard(eqdeg_t *eq, int v, MaskNode *prev_masks)
{
    int i, ll, t;
    uint32_t bbb[8], ccc[8];
    MaskNode *cur_masks;

    cur_masks = NULL;

    while (prev_masks != NULL) {

    ll = push_stack(eq, prev_masks->b,prev_masks->c,v,bbb,ccc);

    for (i=0; i<ll; ++i) {
        eq->mask_b = bbb[i];
        eq->mask_c = ccc[i];
        t = pivot_reduction(eq, v+1);
        if (t >= eq->gcur_maxlengs[v]) {
        eq->gcur_maxlengs[v] = t;
        eq->gmax_b = eq->mask_b;
        eq->gmax_c = eq->mask_c;
        cur_masks = cons_MaskNode(cur_masks, eq->mask_b, eq->mask_c, t);
        }
    }
    prev_masks = prev_masks->next;
    }

    cur_masks = delete_lower_MaskNodes(cur_masks, eq->gcur_maxlengs[v]);

    return cur_masks;
}


/* (v-1) bitmasks of b,c */
static void optimize_v(eqdeg_t *eq, uint32_t b, uint32_t c, int v)
{
    int i, max_len, max_i, ll, t;
    uint32_t bbb[8], ccc[8];

    ll = push_stack(eq, b,c,v,bbb,ccc);

    max_len = max_i = 0;
    if (ll > 1) {
    for (i=0; i<ll; ++i) {
        eq->mask_b = bbb[i];
        eq->mask_c = ccc[i];
        t = pivot_reduction(eq, v+1);
        if (t > max_len) {
        max_len = t;
        max_i = i;
        }
    }
    }

    if ( v >= eq->www-1 ) {
    eq->mask_b = bbb[max_i];
    eq->mask_c = ccc[max_i];
    return;
    }

    optimize_v(eq, bbb[max_i], ccc[max_i], v+1);
}

static int push_stack(eqdeg_t *eq, uint32_t b, uint32_t c, int v,
              uint32_t *bbb, uint32_t *ccc)
{
    int i, ll, ncv;
    uint32_t cv_buf[2];

    ll = 0;

    if( (v+eq->shift_t) < eq->www ){
        ncv = 2; cv_buf[0] = c | eq->bitmask[v]; cv_buf[1] = c;
    }
    else {
        ncv = 1; cv_buf[0] = c;
    }

    for( i=0; i<ncv; ++i)
        ll += push_mask(eq, ll, v, b, cv_buf[i], bbb, ccc);

    return ll;
}

static int push_mask(eqdeg_t *eq, int l, int v, uint32_t b, uint32_t c,
             uint32_t *bbb, uint32_t *ccc)
{
    int i, j, k, nbv, nbvt;
    uint32_t bmask, bv_buf[2], bvt_buf[2];

    k = l;
    if( (eq->shift_s+v) >= eq->www ){
        nbv = 1; bv_buf[0] = 0;
    }
    else if( (v>=eq->shift_t) && (c&eq->bitmask[v-eq->shift_t] ) ){
        nbv = 1; bv_buf[0] = b&eq->bitmask[v];
    }
    else {
        nbv = 2; bv_buf[0] = eq->bitmask[v]; bv_buf[1] = 0;
    }

    if( ((v+eq->shift_t+eq->shift_s) < eq->www) && (c&eq->bitmask[v]) ){
        nbvt = 2; bvt_buf[0] = eq->bitmask[v+eq->shift_t]; bvt_buf[1] = 0;
    }
    else {
        nbvt = 1; bvt_buf[0] = 0;
    }

    bmask = eq->bitmask[v];
    if( (v+eq->shift_t) < eq->www )
        bmask |= eq->bitmask[v+eq->shift_t];
    bmask = ~bmask;
    for( i=0; i<nbvt; ++i){
        for( j=0; j<nbv; ++j){
            bbb[k] = (b&bmask) | bv_buf[j] | bvt_buf[i];
            ccc[k] = c;
            ++k;
        }
    }

    return k-l;
}


/**********************************/
/****  subroutines for lattice ****/
/**********************************/
static int pivot_reduction(eqdeg_t *eq, int v)
{
    Vector **lattice, *ltmp;
    int i;
    int pivot;
    int count;
    int min;

    eq->upper_v_bits = 0;
    for( i=0; i<v; i++) {
        eq->upper_v_bits |= eq->bitmask[i];
    }

    lattice = make_lattice(eq, v );

    for (;;) {
    pivot = calc_pivot(lattice[v]->next);
    if (lattice[pivot]->count < lattice[v]->count) {
        ltmp = lattice[pivot];
        lattice[pivot] = lattice[v];
        lattice[v] = ltmp;
    }
    add(eq->nnn, lattice[v], lattice[pivot]);
    if (lattice[v]->next == 0) {
        count = 0;
        next_state(eq, lattice[v], &count);
        if (lattice[v]->next == 0) {
        if (is_zero(eq->nnn, lattice[v])) {
            break;
        }
        while (lattice[v]->next == 0) {
            count++;
            next_state(eq, lattice[v], &count);
            if (count > eq->nnn * (eq->www-1) - eq->rrr) {
            break;
            }
        }
        if (lattice[v]->next == 0) {
            break;
        }
        }
    }
    }

    min = lattice[0]->count;
    for (i = 1; i < v; i++) {
    if (min > lattice[i]->count) {
        min = lattice[i]->count;
    }
    }
    free_lattice( lattice, v );
    return min;
}




/********************************/
/** allocate momory for Vector **/
/********************************/
static Vector *new_Vector(int nnn)
{
    Vector *v;

    v = (Vector *)malloc( sizeof( Vector ) );
    if( v == NULL ){
        // replace meesage by ql exception
        QL_FAIL("malloc error in \"new_Vector()\"\n");
        //printf("malloc error in \"new_Vector()\"\n");
        exit(1);
    }

    v->cf = (uint32_t *)calloc( nnn, sizeof( uint32_t ) );
    if( v->cf == NULL ){
        QL_FAIL("calloc error in \"new_Vector()\"\n");
        //printf("calloc error in \"new_Vector()\"\n");
        exit(1);
    }

    v->start = 0;

    return v;
}


/************************************************/
/* frees *v which was allocated by new_Vector() */
/************************************************/
static void free_Vector( Vector *v )
{
    if( NULL != v->cf ) free( v->cf );
    if( NULL != v ) free( v );
}

static void free_lattice( Vector **lattice, int v)
{
    int i;

    for( i=0; i<=v; i++)
        free_Vector( lattice[i] );
    free( lattice );
}

/* adds v to u (then u will change) */
static void add(int nnn, Vector *u, Vector *v)
{
    int i;
    int diff = (v->start - u->start + nnn) % nnn;
    for (i = 0; i < nnn - diff; i++) {
    u->cf[i] ^= v->cf[i + diff];
    }
    diff = diff - nnn;
    for (; i < nnn; i++) {
    u->cf[i] ^= v->cf[i + diff];
    }
    u->next ^=  v->next;
}

/* makes a initial lattice */
static Vector **make_lattice(eqdeg_t *eq, int v)
{
    int i;
    int count;
    Vector **lattice, *bottom;

    lattice = (Vector **)malloc( (v+1) * sizeof( Vector *) );
    if( NULL == lattice ){
        //replace message by ql exception
        QL_FAIL("malloc error in \"make_lattice\"\n");
        //printf("malloc error in \"make_lattice\"\n");
        exit(1);
    }

    for( i=0; i<v; i++){ /* from 0th row to v-1-th row */
        lattice[i] = new_Vector(eq->nnn);
        lattice[i]->next = eq->bitmask[i];
        lattice[i]->start = 0;
        lattice[i]->count = 0;
    }

    bottom = new_Vector(eq->nnn); /* last row */
    for(i=0; i< eq->nnn; i++) {
    bottom->cf[i] = 0;
    }
    bottom->cf[eq->nnn -1] = 0xc0000000 & eq->greal_mask;
    bottom->start = 0;
    bottom->count = 0;
    count = 0;
    do {
    next_state(eq, bottom, &count);
    } while (bottom->next == 0);
//    degree_of_vector(eq, top );
    lattice[v] = bottom;

    return lattice;
}

static void next_state(eqdeg_t *eq, Vector *v, int *count) {
    uint32_t tmp;

    do {
    tmp = ( v->cf[v->start] & eq->gupper_mask )
        | ( v->cf[(v->start + 1) % eq->nnn] & eq->glower_mask );
    v->cf[v->start] = v->cf[(v->start + eq->mmm) % eq->nnn]
        ^ ( (tmp>>1) ^ eq->aaa[lsb(eq, tmp)] );
    v->cf[v->start] &= eq->greal_mask;
    tmp = v->cf[v->start];
    v->start = (v->start + 1) % eq->nnn;
    v->count++;
    tmp = trnstmp(eq, tmp);
    tmp = masktmp(eq, tmp);
    v->next = tmp & eq->upper_v_bits;
    (*count)++;
    if (*count > eq->nnn * (eq->www-1) - eq->rrr) {
        break;
    }
    } while (v->next == 0);
}

/***********/
static MaskNode *cons_MaskNode(MaskNode *head, uint32_t b, uint32_t c, int leng)
{
    MaskNode *t;

    t = (MaskNode*)malloc(sizeof(MaskNode));
    if (t == NULL) {
    //replcae message by ql exception
    QL_FAIL("malloc error in \"cons_MaskNode\"\n");
    //printf("malloc error in \"cons_MaskNode\"\n");
        exit(1);
    }

    t->b = b;
    t->c = c;
    t->leng = leng;
    t->next = head;

    return t;
}

static void delete_MaskNodes(MaskNode *head)
{
    MaskNode *t;

    while(head != NULL) {
    t = head->next;
    free(head);
    head = t;
    }
}

static MaskNode *delete_lower_MaskNodes(MaskNode *head, int l)
{
    MaskNode *s, *t, *tail;

    s = head;
    while(1) { /* heading */
    if (s == NULL)
        return NULL;
    if (s->leng >= l)
        break;
    t = s->next;
    free(s);
    s = t;
    }

    head = tail = s;

    while (head != NULL) {
    t = head->next;
    if (head->leng < l) {
        free(head);
    }
    else {
        tail->next = head;
        tail = head;
    }
    head = t;
    }

    tail->next = NULL;
    return s;
}

// disable debug output
// #if defined(DEBUG)
// static void count_MaskNodes(MaskNode *head)
// {
//     int c;

//     c = 0;
//     while(head != NULL) {
//  head = head->next;
//  c++;
//     }
//     printf ("---> number of nodes = %d\n",c);
// }

// static void show_distrib(eqdeg_t *eq, mt_struct *mts)
// {
//     int i, lim, diff, t;
//     double per;

//     init_tempering(eq, mts);

//     eq->mask_b = (mts->maskB) << eq->ggap;
//     eq->mask_c = (mts->maskC) << eq->ggap;
//     for (i=0; i< eq->www; i++) {
//  t = lenstra(eq, i+1);
//  lim = (eq->nnn * eq->www - eq->rrr)/(i+1);
//  diff = lim  - t;
//  per = (double)t / (double)lim;
//  printf ("%d %d %d %d %4.2f\n", i+1, t,  lim, diff, per);
//     }
// }
// #endif

// genmtrand.c

#define SHIFT1 18

void sgenrand_mt(uint32_t seed, mt_struct *mts)
{
    int i;

    for (i=0; i<mts->nn; i++) {
    mts->state[i] = seed;
        seed = (UINT32_C(1812433253) * (seed  ^ (seed >> 30))) + i + 1;
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
    }
    mts->i = mts->nn;

    for (i=0; i<mts->nn; i++)
    mts->state[i] &= mts->wmask;
}


uint32_t genrand_mt(mt_struct *mts)
{
    uint32_t *st, uuu, lll, aa, x;
    int k,n,m,lim;

    if ( mts->i >= mts->nn ) {
    n = mts->nn; m = mts->mm;
    aa = mts->aaa;
    st = mts->state;
    uuu = mts->umask; lll = mts->lmask;

    lim = n - m;
    for (k=0; k<lim; k++) {
        x = (st[k]&uuu)|(st[k+1]&lll);
        st[k] = st[k+m] ^ (x>>1) ^ (x&1U ? aa : 0U);
    }
    lim = n - 1;
    for (; k<lim; k++) {
        x = (st[k]&uuu)|(st[k+1]&lll);
        st[k] = st[k+m-n] ^ (x>>1) ^ (x&1U ? aa : 0U);
    }
    x = (st[n-1]&uuu)|(st[0]&lll);
    st[n-1] = st[m-1] ^ (x>>1) ^ (x&1U ? aa : 0U);
    mts->i=0;
    }

    x = mts->state[mts->i];
    mts->i += 1;
    x ^= x >> mts->shift0;
    x ^= (x << mts->shiftB) & mts->maskB;
    x ^= (x << mts->shiftC) & mts->maskC;
    x ^= x >> mts->shift1;

    return x;
}

// prescr.c


#define LIMIT_IRRED_DEG 31
#define NIRREDPOLY 127
#define MAX_IRRED_DEG 9

/* list of irreducible polynomials whose degrees are less than 10 */
static const int irredpolylist[NIRREDPOLY][MAX_IRRED_DEG+1] = {
    {0,1,0,0,0,0,0,0,0,0,},{1,1,0,0,0,0,0,0,0,0,},{1,1,1,0,0,0,0,0,0,0,},
    {1,1,0,1,0,0,0,0,0,0,},{1,0,1,1,0,0,0,0,0,0,},{1,1,0,0,1,0,0,0,0,0,},
    {1,0,0,1,1,0,0,0,0,0,},{1,1,1,1,1,0,0,0,0,0,},{1,0,1,0,0,1,0,0,0,0,},
    {1,0,0,1,0,1,0,0,0,0,},{1,1,1,1,0,1,0,0,0,0,},{1,1,1,0,1,1,0,0,0,0,},
    {1,1,0,1,1,1,0,0,0,0,},{1,0,1,1,1,1,0,0,0,0,},{1,1,0,0,0,0,1,0,0,0,},
    {1,0,0,1,0,0,1,0,0,0,},{1,1,1,0,1,0,1,0,0,0,},{1,1,0,1,1,0,1,0,0,0,},
    {1,0,0,0,0,1,1,0,0,0,},{1,1,1,0,0,1,1,0,0,0,},{1,0,1,1,0,1,1,0,0,0,},
    {1,1,0,0,1,1,1,0,0,0,},{1,0,1,0,1,1,1,0,0,0,},{1,1,0,0,0,0,0,1,0,0,},
    {1,0,0,1,0,0,0,1,0,0,},{1,1,1,1,0,0,0,1,0,0,},{1,0,0,0,1,0,0,1,0,0,},
    {1,0,1,1,1,0,0,1,0,0,},{1,1,1,0,0,1,0,1,0,0,},{1,1,0,1,0,1,0,1,0,0,},
    {1,0,0,1,1,1,0,1,0,0,},{1,1,1,1,1,1,0,1,0,0,},{1,0,0,0,0,0,1,1,0,0,},
    {1,1,0,1,0,0,1,1,0,0,},{1,1,0,0,1,0,1,1,0,0,},{1,0,1,0,1,0,1,1,0,0,},
    {1,0,1,0,0,1,1,1,0,0,},{1,1,1,1,0,1,1,1,0,0,},{1,0,0,0,1,1,1,1,0,0,},
    {1,1,1,0,1,1,1,1,0,0,},{1,0,1,1,1,1,1,1,0,0,},{1,1,0,1,1,0,0,0,1,0,},
    {1,0,1,1,1,0,0,0,1,0,},{1,1,0,1,0,1,0,0,1,0,},{1,0,1,1,0,1,0,0,1,0,},
    {1,0,0,1,1,1,0,0,1,0,},{1,1,1,1,1,1,0,0,1,0,},{1,0,1,1,0,0,1,0,1,0,},
    {1,1,1,1,1,0,1,0,1,0,},{1,1,0,0,0,1,1,0,1,0,},{1,0,1,0,0,1,1,0,1,0,},
    {1,0,0,1,0,1,1,0,1,0,},{1,0,0,0,1,1,1,0,1,0,},{1,1,1,0,1,1,1,0,1,0,},
    {1,1,0,1,1,1,1,0,1,0,},{1,1,1,0,0,0,0,1,1,0,},{1,1,0,1,0,0,0,1,1,0,},
    {1,0,1,1,0,0,0,1,1,0,},{1,1,1,1,1,0,0,1,1,0,},{1,1,0,0,0,1,0,1,1,0,},
    {1,0,0,1,0,1,0,1,1,0,},{1,0,0,0,1,1,0,1,1,0,},{1,0,1,1,1,1,0,1,1,0,},
    {1,1,0,0,0,0,1,1,1,0,},{1,1,1,1,0,0,1,1,1,0,},{1,1,1,0,1,0,1,1,1,0,},
    {1,0,1,1,1,0,1,1,1,0,},{1,1,1,0,0,1,1,1,1,0,},{1,1,0,0,1,1,1,1,1,0,},
    {1,0,1,0,1,1,1,1,1,0,},{1,0,0,1,1,1,1,1,1,0,},{1,1,0,0,0,0,0,0,0,1,},
    {1,0,0,0,1,0,0,0,0,1,},{1,1,1,0,1,0,0,0,0,1,},{1,1,0,1,1,0,0,0,0,1,},
    {1,0,0,0,0,1,0,0,0,1,},{1,0,1,1,0,1,0,0,0,1,},{1,1,0,0,1,1,0,0,0,1,},
    {1,1,0,1,0,0,1,0,0,1,},{1,0,0,1,1,0,1,0,0,1,},{1,1,1,1,1,0,1,0,0,1,},
    {1,0,1,0,0,1,1,0,0,1,},{1,0,0,1,0,1,1,0,0,1,},{1,1,1,1,0,1,1,0,0,1,},
    {1,1,1,0,1,1,1,0,0,1,},{1,0,1,1,1,1,1,0,0,1,},{1,1,1,0,0,0,0,1,0,1,},
    {1,0,1,0,1,0,0,1,0,1,},{1,0,0,1,1,0,0,1,0,1,},{1,1,0,0,0,1,0,1,0,1,},
    {1,0,1,0,0,1,0,1,0,1,},{1,1,1,1,0,1,0,1,0,1,},{1,1,1,0,1,1,0,1,0,1,},
    {1,0,1,1,1,1,0,1,0,1,},{1,1,1,1,0,0,1,1,0,1,},{1,0,0,0,1,0,1,1,0,1,},
    {1,1,0,1,1,0,1,1,0,1,},{1,0,1,0,1,1,1,1,0,1,},{1,0,0,1,1,1,1,1,0,1,},
    {1,0,0,0,0,0,0,0,1,1,},{1,1,0,0,1,0,0,0,1,1,},{1,0,1,0,1,0,0,0,1,1,},
    {1,1,1,1,1,0,0,0,1,1,},{1,1,0,0,0,1,0,0,1,1,},{1,0,0,0,1,1,0,0,1,1,},
    {1,1,0,1,1,1,0,0,1,1,},{1,0,0,1,0,0,1,0,1,1,},{1,1,1,1,0,0,1,0,1,1,},
    {1,1,0,1,1,0,1,0,1,1,},{1,0,0,0,0,1,1,0,1,1,},{1,1,0,1,0,1,1,0,1,1,},
    {1,0,1,1,0,1,1,0,1,1,},{1,1,0,0,1,1,1,0,1,1,},{1,1,1,1,1,1,1,0,1,1,},
    {1,0,1,0,0,0,0,1,1,1,},{1,1,1,1,0,0,0,1,1,1,},{1,0,0,0,0,1,0,1,1,1,},
    {1,0,1,0,1,1,0,1,1,1,},{1,0,0,1,1,1,0,1,1,1,},{1,1,1,0,0,0,1,1,1,1,},
    {1,1,0,1,0,0,1,1,1,1,},{1,0,1,1,0,0,1,1,1,1,},{1,0,1,0,1,0,1,1,1,1,},
    {1,0,0,1,1,0,1,1,1,1,},{1,1,0,0,0,1,1,1,1,1,},{1,0,0,1,0,1,1,1,1,1,},
    {1,1,0,1,1,1,1,1,1,1,},
};

static void MakepreModPolys(prescr_t *pre, int mm, int nn, int rr, int ww);
static Polynomial *make_tntm( int n, int m);
static Polynomial *PolynomialDup(Polynomial *pl);
static void PolynomialMod(Polynomial *wara, const Polynomial *waru);
static Polynomial *PolynomialMult(Polynomial *p0, Polynomial *p1);
static void FreePoly( Polynomial *p);
static Polynomial *NewPoly(int degree);
static int IsReducible(prescr_t *pre, uint32_t aaa, uint32_t *polylist);
static uint32_t word2bit(Polynomial *pl);
static void makemodlist(prescr_t *pre, Polynomial *pl, int nPoly);
static void NextIrredPoly(Polynomial *pl, int nth);

// disable debugging

// #if defined(DEBUG)
// /******* debuging functions ********/
// static void printPoly(Polynomial *p);
// static void printPoly2(Polynomial *p);
// static void printuint32(uint32_t x);
// static void show_modlist(prescr_t *pre);
// static Polynomial *PolynomialSum( Polynomial *p0, Polynomial *p1);
// /***********************************/
// #endif

/*************************************************/
/*************************************************/
int _prescreening_dc(prescr_t *pre, uint32_t aaa)
{

    int i;

    for (i=0; i<NIRREDPOLY; i++) {
    if (IsReducible(pre, aaa,pre->modlist[i])==REDU)
        return REJECTED;
    }
    return NOT_REJECTED;
}

void _InitPrescreening_dc(prescr_t *pre, int m, int n, int r, int w)
{
    int i;
    Polynomial *pl;

    pre->sizeofA = w;

    pre->preModPolys = (Polynomial **)malloc(
    (pre->sizeofA+1)*(sizeof(Polynomial*)));
    if (NULL == pre->preModPolys) {
    //replace message by al exception
    QL_FAIL("malloc error in \"InitPrescreening\"\n");
    //printf ("malloc error in \"InitPrescreening\"\n");
    exit(1);
    }
    MakepreModPolys(pre, m,n,r,w);

    pre->modlist = (uint32_t**)malloc(NIRREDPOLY * sizeof(uint32_t*));
    if (NULL == pre->modlist) {
    //replace message by al exception
    QL_FAIL("malloc error in \"InitPrescreening()\"\n");
    //printf ("malloc error in \"InitPrescreening()\"\n");
    exit(1);
    }
    for (i=0; i<NIRREDPOLY; i++) {
    pre->modlist[i]
        = (uint32_t*)malloc( (pre->sizeofA + 1) * (sizeof(uint32_t)) );
    if (NULL == pre->modlist[i]) {
        //replace message by al exception
        QL_FAIL("malloc error in \"InitPrescreening()\"\n");
        //printf ("malloc error in \"InitPrescreening()\"\n");
        exit(1);
    }
    }


    for (i=0; i<NIRREDPOLY; i++) {
    pl = NewPoly(MAX_IRRED_DEG);
    NextIrredPoly(pl,i);
    makemodlist(pre, pl, i);
    FreePoly(pl);
    }

    for (i=pre->sizeofA; i>=0; i--)
    FreePoly(pre->preModPolys[i]);
    free(pre->preModPolys);

}

void _EndPrescreening_dc(prescr_t *pre)
{
    int i;

    for (i=0; i<NIRREDPOLY; i++)
      free(pre->modlist[i]);
    free(pre->modlist);
}

/*************************************************/
/******          static functions           ******/
/*************************************************/

void NextIrredPoly(Polynomial *pl, int nth)
{
    int i, max_deg;

    for (max_deg=0,i=0; i<=MAX_IRRED_DEG; i++) {
    if ( irredpolylist[nth][i] )
        max_deg = i;
    pl->x[i] = irredpolylist[nth][i];
    }

    pl->deg = max_deg;

}

static void makemodlist(prescr_t *pre, Polynomial *pl, int nPoly)
{
    Polynomial *tmpPl;
    int i;

    for (i=0; i<=pre->sizeofA; i++) {
    tmpPl = PolynomialDup(pre->preModPolys[i]);
    PolynomialMod(tmpPl,pl);
    pre->modlist[nPoly][i] = word2bit(tmpPl);
    FreePoly(tmpPl);
    }
}

/* Pack Polynomial into a word */
static uint32_t word2bit(Polynomial *pl)
{
    int i;
    uint32_t bx;

    bx = 0;
    for (i=pl->deg; i>0; i--) {
    if (pl->x[i]) bx |= 0x1;
    bx <<= 1;
    }
    if (pl->x[0]) bx |= 0x1;

    return bx;
}

/* REDU -- reducible */
/* aaa = (a_{w-1}a_{w-2}...a_1a_0 */
static int IsReducible(prescr_t *pre, uint32_t aaa, uint32_t *polylist)
{
    int i;
    uint32_t x;

    x = polylist[pre->sizeofA];
    for (i=pre->sizeofA-1; i>=0; i--) {
    if (aaa&0x1)
        x ^= polylist[i];
    aaa >>= 1;
    }

    if ( x == 0 ) return REDU;
    else return NONREDU;
}


/***********************************/
/**   functions for polynomial    **/
/***********************************/
static Polynomial *NewPoly(int degree)
{
    Polynomial *p;

    p = (Polynomial *)calloc( 1, sizeof(Polynomial));
    if( p==NULL ){
    //replace message by ql exception
    QL_FAIL("calloc error in \"NewPoly()\"\n");
    //printf("calloc error in \"NewPoly()\"\n");
    exit(1);
    }
    p->deg = degree;

    if (degree < 0) {
    p->x = NULL;
    return p;
    }

    p->x = (int *)calloc( degree + 1, sizeof(int));
    if( p->x == NULL ){
    //replace message by ql exception
    QL_FAIL("calloc error\n");
    //printf("calloc error\n");
    exit(1);
    }

    return p;
}

static void FreePoly( Polynomial *p)
{
    if (p->x != NULL)
    free( p->x );
    free( p );
}


/** multiplication **/
static Polynomial *PolynomialMult(Polynomial *p0,Polynomial *p1)
{
    int i, j;
    Polynomial *p;

    /* if either p0 or p1 is 0, return 0 */
    if ( (p0->deg < 0) || (p1->deg < 0) ) {
    p = NewPoly(-1);
    return p;
    }

    p = NewPoly(p0->deg + p1->deg);
    for( i=0; i<=p1->deg; i++){
    if( p1->x[i] ){
        for( j=0; j<=p0->deg; j++){
        p->x[i+j] ^= p0->x[j];
        }
    }
    }

    return p;
}

/** wara mod waru **/
/** the result is stored in wara ********/
static void PolynomialMod( Polynomial *wara, const Polynomial *waru)
{
    int i;
    int deg_diff;

    while( wara->deg >= waru->deg  ){
    deg_diff = wara->deg - waru->deg;
    for( i=0; i<=waru->deg; i++){
        wara->x[ i+deg_diff ] ^= waru->x[i];
    }

    for( i=wara->deg; i>=0; i--){
        if( wara->x[i] ) break;
    }
    wara->deg=i;

    }
}

static Polynomial *PolynomialDup(Polynomial *pl)
{
    Polynomial *pt;
    int i;

    pt = NewPoly(pl->deg);
    for (i=pl->deg; i>=0; i--)
    pt->x[i] = pl->x[i];

    return pt;
}

/** make the polynomial  "t**n + t**m"  **/
static Polynomial *make_tntm( int n, int m)
{
    Polynomial *p;

    p = NewPoly(n);
    p->x[n] = p->x[m] = 1;

    return p;
}

static void MakepreModPolys(prescr_t *pre, int mm, int nn, int rr, int ww)
{
    Polynomial *t, *t0, *t1, *s, *s0, *s1;
    int i,j;

    j = 0;
    t = NewPoly(0);
    t->deg = 0;
    t->x[0] = 1;
    pre->preModPolys[j++] = t;

    t = make_tntm (nn, mm);
    t0 = make_tntm (nn, mm);
    s = make_tntm (nn-1, mm-1);

    for( i=1; i<(ww - rr); i++){
    pre->preModPolys[j++] = PolynomialDup(t0);
    t1 = t0;
    t0 = PolynomialMult(t0, t);
    FreePoly(t1);
    }

    pre->preModPolys[j++] = PolynomialDup(t0);

    s0 =PolynomialMult( t0, s);
    FreePoly(t0);	FreePoly(t);
    for( i=(rr-2); i>=0; i--){
    pre->preModPolys[j++] = PolynomialDup(s0);
    s1 = s0;
    s0 = PolynomialMult( s0, s);
    FreePoly(s1);
    }

    pre->preModPolys[j++] = PolynomialDup(s0);

    FreePoly(s0); FreePoly(s);
}

    // disable debugging

/********************************/

/* following functions are used for debuging */
// #if defined(DEBUG)
// static void printPoly(Polynomial *p)
// {
//     int i;
//     for (i=0; i<=p->deg; i++) {
//  if (p->x[i] == 1) printf ("1");
//  else if (p->x[i] == 0) printf ("0");
//  else printf ("*");
//     }
//     printf("\n");
// }

// static void printPoly2(Polynomial *p)
// {
//     int i;
//     for (i=0; i<=p->deg; i++) {
//  if (p->x[i] == 1) printf ("%d ", i);
//     }
//     printf("\n");
// }

// static void printPoly3(Polynomial *p)
// {
//     int i,cnt;
//     int startf;

//     startf = 0;
//     cnt = 0;
//     for (i=0; i<=p->deg; i++) {
//  if (p->x[i] == 1) {
//      if (startf) {
//      if (i==1)
//          printf ("x");
//      else
//          printf ("+x^%d", i);
//      }
//      else {
//      if (i==0) printf ("1");
//      else if (i==1) printf ("x");
//      else printf ("x^%d", i);
//      startf = 1;
//      }
//      cnt++;
//      if (cnt==10) {printf("\n");cnt=0;}
//  }
//     }
//     printf("\n");
// }

// static void printuint32(uint32_t x)
// {
//     int i;

//     for (i=0; i<32; i++) {
//  if ( x & UINT32_C(0x80000000) ) printf ("1");
//  else printf ("0");
//  x <<= 1;
//     }
//     printf ("\n");
// }

// static void show_modlist(prescr_t *pre)
// {
//     int i,j;

//     for (i=0; i<NIRREDPOLY; i++)  {
//  for (j=0; j<=pre->sizeofA; j++)
//      printuint32(pre->modlist[i][j]);
//  getchar();
//     }
// }

// /** addition **/
// static Polynomial *PolynomialSum( Polynomial *p0, Polynomial *p1)
// {
//     Polynomial *p, *pmin, *pmax;
//     int i, maxdeg, mindeg;

//     if ( p0->deg > p1->deg ) {
//  pmax = p0;
//  pmin = p1;
//     }
//     else {
//  pmax = p1;
//  pmin = p0;
//     }
//     maxdeg = pmax->deg;
//     mindeg = pmin->deg;

//     p = NewPoly(maxdeg);
//     for (i=0; i<=maxdeg; i++)
//  p->x[i] = pmax->x[i];
//     for( i=0; i<=mindeg; i++)
//  p->x[i] ^= pmin->x[i];

//     for( i=p->deg; i>=0; i--){
//  if( p->x[i] ) break;
//     }
//     p->deg=i;

//     return p;
// }

// static Polynomial *chPoly(prescr_t *pre, uint32_t a)
// {
//     Polynomial *pl, *tmpP;
//     int i;

//     pl = PolynomialDup(pre->preModPolys[pre->sizeofA]);
//     for (i=pre->sizeofA-1; i>=0; i--) {
//  if (a&1U) {
//      tmpP = PolynomialSum(pl, pre->preModPolys[i]);
//      FreePoly(pl);
//      pl = tmpP;
//  }
//  a >>= 1;
//     }

//     return pl;
// }


// int main(void)
// {
//     int i,j,cnt;
//     uint32_t aaa;
//     prescr_t pre;

//     for (j=0; j<1000;j++) {
//  _InitPrescreening_dc(&pre, 11, 17, 23, 32);

//  for (cnt=0,i=0; i<1000; i++) {
//      aaa = random();
//      aaa |= UINT32_C(0x80000000);
//      if (NOT_REJECTED == _prescreening_dc(&pre, aaa)) {
//      cnt++;
//      }
//  }
//  printf ("%d\n",cnt);

//  _EndPrescreening_dc(&pre);
//     }

//     return 0;
// }

//#endif

// seive.c

#define WORDLEN 32
#define LSB 0x1
#define MAX_SEARCH 10000


_org_state global_mt19937;
/*******************************************************************/
static uint32_t nextA(_org_state *org, int w);
static uint32_t nextA_id(_org_state *org, int w, int id, int idw);
static void make_masks(int r, int w, mt_struct *mts);
static int get_irred_param(check32_t *ck, prescr_t *pre, _org_state *org,
               mt_struct *mts,int id, int idw);
static mt_struct *alloc_mt_struct(int n);
static mt_struct *init_mt_search(check32_t *ck, prescr_t *pre, int w, int p);
static void end_mt_search(prescr_t *pre);
static void copy_params_of_mt_struct(mt_struct *src, mt_struct *dst);
static int proper_mersenne_exponent(int p);
/*******************************************************************/

/* When idw==0, id is not embedded into "a" */
#define FOUND 1
#define NOT_FOUND 0
static int get_irred_param(check32_t *ck, prescr_t *pre, _org_state *org,
               mt_struct *mts, int id, int idw)
{
    int i;
    uint32_t a;

    for (i=0; i<MAX_SEARCH; i++) {
    if (idw == 0)
        a = nextA(org, mts->ww);
    else
        a = nextA_id(org, mts->ww, id, idw);
    if (NOT_REJECTED == _prescreening_dc(pre, a) ) {
        if (IRRED
        == _CheckPeriod_dc(ck, org, a,mts->mm,mts->nn,mts->rr,mts->ww)) {
        mts->aaa = a;
        break;
        }
    }
    }

    if (MAX_SEARCH == i) return NOT_FOUND;
    return FOUND;
}


static uint32_t nextA(_org_state *org, int w)
{
    uint32_t x, word_mask;

    word_mask = 0xFFFFFFFF;
    word_mask <<= WORDLEN - w;
    word_mask >>= WORDLEN - w;

    x = _genrand_dc(org);
    x &= word_mask;
    x |= (LSB << (w-1));

    return x;
}

static uint32_t nextA_id(_org_state *org, int w, int id, int idw)
{
    uint32_t x, word_mask;

    word_mask = 0xFFFFFFFF;
    word_mask <<= WORDLEN - w;
    word_mask >>= WORDLEN - w;
    word_mask >>= idw;
    word_mask <<= idw;

    x = _genrand_dc(org);
    x &= word_mask;
    x |= (LSB << (w-1));
    x |= (uint32_t)id; /* embedding id */

    return x;
}

static void make_masks(int r, int w, mt_struct *mts)
{
    int i;
    uint32_t ut, wm, um, lm;

    wm = 0xFFFFFFFF;
    wm >>= (WORDLEN - w);

    ut = 0;
    for (i=0; i<r; i++) {
    ut <<= 1;
    ut |= LSB;
    }

    lm = ut;
    um = (~ut) & wm;

    mts->wmask = wm;
    mts->umask = um;
    mts->lmask = lm;
}

static mt_struct *init_mt_search(check32_t *ck, prescr_t *pre, int w, int p)
{
    int n, m, r;
    mt_struct *mts;

    if ( (w>32) || (w<31) ) {
        //we cehck this in the wrapper already
        //printf ("Sorry, currently only w = 32 or 31 is allowded.\n");
    return NULL;
    }

    if ( !proper_mersenne_exponent(p) ) {
    if (p<521) {
        // we do not need the message since we check in the wrapper already
        //printf ("\"p\" is too small.\n");
        return NULL;
    }
    else if (p>44497){
        //printf ("\"p\" is too large.\n");
        return NULL;
    }
    else {
        //printf ("\"p\" is not a Mersenne exponent.\n");
        return NULL;
    }
    }

    n = p/w + 1; /* since p is Mersenne Exponent, w never divids p */
    mts = alloc_mt_struct(n);
    if (NULL == mts) return NULL;

    m = n/2;
    if (m < 2) m = n-1;
    r = n * w - p;

    make_masks(r, w, mts);
    _InitPrescreening_dc(pre, m, n, r, w);
    _InitCheck32_dc(ck, r, w);

    mts->mm = m;
    mts->nn = n;
    mts->rr = r;
    mts->ww = w;

    return mts;
}

static void end_mt_search(prescr_t *pre)
{
    _EndPrescreening_dc(pre);
}

/*
   w -- word size
   p -- Mersenne Exponent
   seed -- seed for original mt19937 to generate parameter.
*/
mt_struct *get_mt_parameter_st(int w, int p, uint32_t seed)
{
    mt_struct *mts;
    prescr_t pre;
    _org_state org;
    check32_t ck;

    _sgenrand_dc(&org, seed);
    mts = init_mt_search(&ck, &pre, w, p);
    if (mts == NULL) return NULL;

    if ( NOT_FOUND == get_irred_param(&ck, &pre, &org, mts,0,0) ) {
    free_mt_struct(mts);
    return NULL;
    }
    _get_tempering_parameter_hard_dc(mts);
    end_mt_search(&pre);

    return mts;
}

/*
   w -- word size
   p -- Mersenne Exponent
*/
mt_struct *get_mt_parameter(int w, int p)
{
    mt_struct *mts;
    prescr_t pre;
    check32_t ck;

    mts = init_mt_search(&ck, &pre, w, p);
    if (mts == NULL) return NULL;

    if ( NOT_FOUND == get_irred_param(&ck, &pre, &global_mt19937, mts,0,0) ) {
    free_mt_struct(mts);
    return NULL;
    }
    _get_tempering_parameter_hard_dc(mts);
    end_mt_search(&pre);

    return mts;
}

/*
   w -- word size
   p -- Mersenne Exponent
*/
#if 0
mt_struct *get_mt_parameter_opt_temper(int w, int p, uint32_t seed)
{
    mt_struct *mts;
    prescr_t pre;
    _org_state org;
    check32_t ck;

    _sgenrand_dc(&org, seed);
    mts = init_mt_search(&ck, &pre, w, p);
    if (mts == NULL) return NULL;

    if ( NOT_FOUND == get_irred_param(&ck, &pre, &org, mts,0,0) ) {
    free_mt_struct(mts);
    return NULL;
    }
    _get_tempering_parameter_hard_dc(mts);
    end_mt_search(&pre);

    return mts;
}
#endif
/*
   w -- word size
   p -- Mersenne Exponent
*/
#define DEFAULT_ID_SIZE 16
/* id <= 0xffff */
mt_struct *get_mt_parameter_id_st(int w, int p, int id, uint32_t seed)
{
    mt_struct *mts;
    prescr_t pre;
    _org_state org;
    check32_t ck;

    _sgenrand_dc(&org, seed);
    // implicitly ensured in the wrapper, do not need the messages here
    if (id > 0xffff) {
        //printf("\"id\" must be less than 65536\n");
    return NULL;
    }
    if (id < 0) {
        //printf("\"id\" must be positive\n");
    return NULL;
    }

    mts = init_mt_search(&ck, &pre, w, p);
    if (mts == NULL) return NULL;

    if ( NOT_FOUND == get_irred_param(&ck, &pre, &org,
                      mts, id, DEFAULT_ID_SIZE) ) {
    free_mt_struct(mts);
    return NULL;
    }
    _get_tempering_parameter_hard_dc(mts);
    end_mt_search(&pre);

    return mts;
}

mt_struct *get_mt_parameter_id(int w, int p, int id)
{
    mt_struct *mts;
    prescr_t pre;
    check32_t ck;

    // implicitly ensured in the wrapper, do not need the messages here
    if (id > 0xffff) {
        //printf("\"id\" must be less than 65536\n");
    return NULL;
    }
    if (id < 0) {
        //printf("\"id\" must be positive\n");
    return NULL;
    }

    mts = init_mt_search(&ck, &pre, w, p);
    if (mts == NULL) return NULL;

    if ( NOT_FOUND == get_irred_param(&ck, &pre, &global_mt19937,
                      mts, id, DEFAULT_ID_SIZE) ) {
    free_mt_struct(mts);
    return NULL;
    }
    _get_tempering_parameter_hard_dc(mts);
    end_mt_search(&pre);

    return mts;
}

mt_struct **get_mt_parameters_st(int w, int p, int start_id,
                 int max_id, uint32_t seed, int *count)
{
    mt_struct **mtss, *template_mts;
    int i;
    prescr_t pre;
    _org_state org;
    check32_t ck;

    // delete message, we do not use this function in the wrapper
    if ((start_id > max_id) || (max_id > 0xffff) || (start_id < 0)) {
        //printf("\"id\" error\n");
    return NULL;
    }

    _sgenrand_dc(&org, seed);
    mtss = (mt_struct**)malloc(sizeof(mt_struct*)*(max_id-start_id+1));
    if (NULL == mtss) return NULL;

    template_mts = init_mt_search(&ck, &pre, w, p);
    if (template_mts == NULL) {
    free(mtss);
    return NULL;
    }
    *count = 0;
    for (i=0; i<=max_id-start_id; i++) {
    mtss[i] = alloc_mt_struct(template_mts->nn);
    if (NULL == mtss[i]) {
        break;
    }

    copy_params_of_mt_struct(template_mts, mtss[i]);

    if ( NOT_FOUND == get_irred_param(&ck, &pre, &org, mtss[i],
                      i+start_id,DEFAULT_ID_SIZE) ) {
        free_mt_struct(mtss[i]);
        break;
    }
    _get_tempering_parameter_hard_dc(mtss[i]);
    ++(*count);
    }

    free_mt_struct(template_mts);
    end_mt_search(&pre);
    if (*count > 0) {
    return mtss;
    } else {
    free(mtss);
    return NULL;
    }
}

mt_struct **get_mt_parameters(int w, int p, int max_id, int *count)
{
    mt_struct **mtss, *template_mts;
    int i;
    prescr_t pre;
    check32_t ck;
    int start_id = 0;

    // delete message, we do not use the function in the wrapper
    if ((start_id > max_id) || (max_id > 0xffff) || (start_id < 0)) {
        //printf("\"id\" error\n");
    return NULL;
    }

    mtss = (mt_struct**)malloc(sizeof(mt_struct*)*(max_id-start_id+1));
    if (NULL == mtss) return NULL;

    template_mts = init_mt_search(&ck, &pre, w, p);
    if (template_mts == NULL) {
    free(mtss);
    return NULL;
    }
    *count = 0;
    for (i=0; i<=max_id-start_id; i++) {
    mtss[i] = alloc_mt_struct(template_mts->nn);
    if (NULL == mtss[i]) {
        break;
    }

    copy_params_of_mt_struct(template_mts, mtss[i]);

    if ( NOT_FOUND == get_irred_param(&ck, &pre, &global_mt19937, mtss[i],
                      i+start_id,DEFAULT_ID_SIZE) ) {
        free_mt_struct(mtss[i]);
        break;
    }
    _get_tempering_parameter_hard_dc(mtss[i]);
    ++(*count);
    }

    free_mt_struct(template_mts);
    end_mt_search(&pre);
    if (*count > 0) {
    return mtss;
    } else {
    free(mtss);
    return NULL;
    }
}

/* n : sizeof state vector */
static mt_struct *alloc_mt_struct(int n)
{
    mt_struct *mts;

    mts = (mt_struct*)malloc(sizeof(mt_struct));
    if (NULL == mts) return NULL;
    mts->state = (uint32_t*)malloc(n*sizeof(uint32_t));
    if (NULL == mts->state) {
    free(mts);
    return NULL;
    }

    return mts;
}

void free_mt_struct(mt_struct *mts)
{
    free(mts->state);
    free(mts);
}

void free_mt_struct_array(mt_struct **mtss, int count)
{
    int i;

    if (mtss == NULL) {
    return;
    }
    for (i=0; i < count; i++) {
    free_mt_struct(mtss[i]);
    }
    free(mtss);
}

static void copy_params_of_mt_struct(mt_struct *src, mt_struct *dst)
{
    dst->nn = src->nn;
    dst->mm = src->mm;
    dst->rr = src->rr;
    dst->ww = src->ww;
    dst->wmask = src->wmask;
    dst->umask = src->umask;
    dst->lmask = src->lmask;
}

static int proper_mersenne_exponent(int p)
{
    switch(p) {
    case 521:
    case 607:
    case 1279:
    case 2203:
    case 2281:
    case 3217:
    case 4253:
    case 4423:
    case 9689:
    case 9941:
    case 11213:
    case 19937:
    case 21701:
    case 23209:
    case 44497:
    return 1;
    default:
    return 0;
    }
}

} // namespace mt_detail
} // namespace QuantLib

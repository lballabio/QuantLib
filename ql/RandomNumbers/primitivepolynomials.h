//
// PrimitivePolynomialsModuloTwoUpToDegree27.h
//
// © 2002 "Monte Carlo Methods in Finance"
//
//
// ===========================================================================
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================
//
//
// This file is provided for the use with Sobol' sequences of higher
// dimensions. For the construction of Sobol' numbers, I recommend the
// algorithms published in "Numerical Recipes in C", W. H. Press,
// S. A. Teukolsky, W. t. Vetterling, B. P. Flannery, second edition,
// section 7.7, page 309, ISBN 0-521-43108-5.
//
// The dimensionality of the Sobol' sequence can be extended to virtually any
// size you ever might need by the aid of the table of primitive polynomials
// modulo two as provided ready for compilation in the directory
// "PrimitivePolynomialsModuloTwo" on the CD accompanying the book
// "Monte Carlo Methods in Finance" by Peter Jäckel. You should initialise
// them as described in the section "The initialisation of Sobol' numbers".
// Note that you should always use Sobol' sequences in conjunction with the
// Brownian Bridge.
//

//	PPMT : Primitive Polynomials Modulo Two
//
//
// The encoding is as follows:
//
//  The coefficients of each primitive polynomial are the bits of the given
//  integer. The leading and trailing coefficients, which are 1 for all of the
//  polynomials, have been omitted.
//
//  Example: The polynomial
//
//      4    2
//     x  + x  + 1
//
// is encoded as  2  in the array of polynomials of degree 4 because the
// binary sequence of coefficients
//
//   10101
//
// becomes
//
//    0101
//
// after stripping off the top bit, and this is converted to
//
//     010
//
// by right-shifting and losing the rightmost bit. Similarly, we have
//
//   5    4    3
//  x  + x  + x  + x + 1
//
// encoded as  13  [ (1)1101(1) ]  in the array for degree 5.
//
//
//
// It is up to you to define a macro PPMT_MAX_DIM to a positive integer
// less than or equal to 21200. If you don't define it, it will be set
// below to N_PRIMITIVES_UP_TO_DEGREE_18 which is 21200. That's how many
// primitive polynomials will be compiled into a static array. Should you
// need more, files are available for download from quantlib.org which can 
// be substituted for these versions of primitivepolynomial.h and 
// primitivepolynomial.c and which provide polynomials up to degree 27 for
// a grand total of 8129334 dimensions. Since 8129334 longs compile into 
// an object file of at least 32517336 byte size (in fact,
// gcc -c -O0 PrimitivePolynomialsModuloTwoUpToDegree27.c produced a the file
// PrimitivePolynomialsModuloTwoUpToDegree27.o with 32519920 bytes), it
// is recommended to only compile as many as you may ever need. Worse even
// than the  output file size is the virtual memory requirement for the
// compilation, so really only take the maximum of what you think you might
// ever need.
// After all, you can always recompile with more, should you need it.
//
// Note that PPMT_MAX_DIM will be redefined to be the nearest equal or larger
// number of polynomials up to one of the predefined macros
// N_PRIMITIVES_UP_TO_DEGREE_XX
// below.
//
//
// Example: comment out the line below if you want all of the
// provided primitive polynomials modulo two.
//
// #define PPMT_MAX_DIM 21200

#ifndef quantlib_primitive_polynomials_h
#define quantlib_primitive_polynomials_h

#define N_PRIMITIVES_UP_TO_DEGREE_01         1
#define N_PRIMITIVES_UP_TO_DEGREE_02         2
#define N_PRIMITIVES_UP_TO_DEGREE_03         4
#define N_PRIMITIVES_UP_TO_DEGREE_04         6
#define N_PRIMITIVES_UP_TO_DEGREE_05        12
#define N_PRIMITIVES_UP_TO_DEGREE_06        18
#define N_PRIMITIVES_UP_TO_DEGREE_07        36
#define N_PRIMITIVES_UP_TO_DEGREE_08        52
#define N_PRIMITIVES_UP_TO_DEGREE_09       100
#define N_PRIMITIVES_UP_TO_DEGREE_10       160
#define N_PRIMITIVES_UP_TO_DEGREE_11       336
#define N_PRIMITIVES_UP_TO_DEGREE_12       480
#define N_PRIMITIVES_UP_TO_DEGREE_13      1110
#define N_PRIMITIVES_UP_TO_DEGREE_14      1866
#define N_PRIMITIVES_UP_TO_DEGREE_15      3666
#define N_PRIMITIVES_UP_TO_DEGREE_16      5714
#define N_PRIMITIVES_UP_TO_DEGREE_17     13424
#define N_PRIMITIVES_UP_TO_DEGREE_18     21200

#define N_PRIMITIVES N_PRIMITIVES_UP_TO_DEGREE_18

#ifndef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_18
#endif

#if      PPMT_MAX_DIM > N_PRIMITIVES
# error  PPMT_MAX_DIM cannot be greater than N_PRIMITIVES
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_01
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_01
# define N_MAX_DEGREE 01
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_02
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_02
# define N_MAX_DEGREE 02
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_03
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_03
# define N_MAX_DEGREE 03
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_04
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_04
# define N_MAX_DEGREE 04
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_05
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_05
# define N_MAX_DEGREE 05
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_06
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_06
# define N_MAX_DEGREE 06
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_07
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_07
# define N_MAX_DEGREE 07
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_08
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_08
# define N_MAX_DEGREE 08
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_09
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_09
# define N_MAX_DEGREE 09
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_10
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_10
# define N_MAX_DEGREE 10
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_11
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_11
# define N_MAX_DEGREE 11
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_12
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_12
# define N_MAX_DEGREE 12
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_13
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_13
# define N_MAX_DEGREE 13
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_14
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_14
# define N_MAX_DEGREE 14
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_15
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_15
# define N_MAX_DEGREE 15
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_16
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_16
# define N_MAX_DEGREE 16
#elif    PPMT_MAX_DIM <= N_PRIMITIVES_UP_TO_DEGREE_17
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_17
# define N_MAX_DEGREE 17
#else
# undef  PPMT_MAX_DIM
# define PPMT_MAX_DIM N_PRIMITIVES_UP_TO_DEGREE_18
# define N_MAX_DEGREE 18
#endif


// You can access the following array as in PrimitivePolynomials[i][j]
// with i and j counting from 0 in C convention. PrimitivePolynomials[i][j]
// will get you the j-th (counting from zero) primitive polynomial of degree
// i+1. Each one-dimensional array of primitive polynomials of a given
// degree is terminated with an entry of -1. Accessing beyond this entry
// will result in a memory violation and must be avoided.

#ifdef __cplusplus
extern "C"
#endif
const long *const PrimitivePolynomials[N_MAX_DEGREE];


#endif

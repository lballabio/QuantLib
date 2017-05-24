/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file gaussiannoncentralchisquaredpolynomial.hpp
    \brief non central chi squared polynomials for Gaussian quadratures
*/

#ifndef quantlib_gaussian_non_central_chi_squared_polynomial_hpp
#define quantlib_gaussian_non_central_chi_squared_polynomial_hpp

#include <ql/math/integrals/gaussianorthogonalpolynomial.hpp>

/* Define this to improve the precision of the non central chi squared
   gaussian quadrature by using the boost multiprecision library.
   Needs boost version > 1.52
*/
#ifndef MULTIPRECISION_NON_CENTRAL_CHI_SQUARED_QUADRATURE
//#   define MULTIPRECISION_NON_CENTRAL_CHI_SQUARED_QUADRATURE
#endif

#ifdef MULTIPRECISION_NON_CENTRAL_CHI_SQUARED_QUADRATURE
    #if BOOST_VERSION < 105300
        #error This boost version is too old for boost multi precision support
    #endif

    #include <boost/multiprecision/cpp_dec_float.hpp>
#endif

#include <vector>

namespace QuantLib {
    //! Gauss polynomial for the non central chi squared distribution
    /*! References:
        Gauss quadratures and orthogonal polynomials

        G.H. Gloub and J.H. Welsch: Calculation of Gauss quadrature rule.
        Math. Comput. 23 (1986), 221-230,
        http://web.stanford.edu/class/cme335/spr11/S0025-5718-69-99647-1.pdf

        M. Morandi Cecchi and M. Redivo Zaglia, Computing the coefficients
        of a recurrence formula for numerical integration by moments and
        modified moments.
        http://ac.els-cdn.com/0377042793901522/1-s2.0-0377042793901522-main.pdf?_tid=643d5dca-a05d-11e6-9a56-00000aab0f27&acdnat=1478023545_cf7c87cba4cc9e37a136e68a2564d411
    */

    class GaussNonCentralChiSquaredPolynomial
            : public GaussianOrthogonalPolynomial {
      public:
        GaussNonCentralChiSquaredPolynomial(Real nu, Real lambda);

        Real mu_0() const;
        Real alpha(Size i) const;
        Real beta(Size i) const;
        Real w(Real x) const;

#ifdef MULTIPRECISION_NON_CENTRAL_CHI_SQUARED_QUADRATURE
        typedef boost::multiprecision::number<
            boost::multiprecision::cpp_dec_float<100> > mp_float;
#else
        typedef Real mp_float;
#endif

      private:
        mp_float alpha_(Size i) const;
        mp_float beta_(Size i) const;

        mp_float z(Integer k, Integer i) const;

        const Real nu_, lambda_;

        mutable std::vector<mp_float> b_, c_;
        mutable std::vector<std::vector<mp_float> > z_;
    };
}

#endif

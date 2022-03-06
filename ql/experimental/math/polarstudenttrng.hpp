/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

/*! \file polarstudenttrng.hpp
    \brief Polar Student t random-number generator
*/

#ifndef quantlib_polar_student_t_rng_h
#define quantlib_polar_student_t_rng_h

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    //! Student t random number generator
    /*! Polar transformation based Student T random number generator.
        See "Polar Generation of Random Variates With the t-Distribution",
        Ralph W. Bailey, April 1994, in Mathematics of Computation, Vol 62-206
        page 779.
        The one implemented here is a variant of this algorithm from "Random 
        Number Generation and Monte Carlo Methods", Springer, 2003, page 185. 
        Using a uniform RNG on a [-1,1] support, the extra call to the uniform
        generator (used for the sign of the sample) is avoided.

        Class RNG must implement the following interface:
        \code
            URNG::sample_type RNG::next() const;
        \endcode

        \warning do not use with a low-discrepancy sequence generator.
    */
    template <class URNG>
    class PolarStudentTRng {
      public:
        typedef Sample<Real> sample_type;
        typedef URNG urng_type;

        explicit PolarStudentTRng(Real degFreedom, BigNatural seed = 0) 
            : uniformGenerator_(seed),
              degFreedom_(degFreedom) {
          QL_REQUIRE(degFreedom_ > 0, 
              "Invalid degrees of freedom parameter.");
        }

        explicit PolarStudentTRng(Real degFreedom, const URNG& urng) 
            : uniformGenerator_(urng), 
              degFreedom_(degFreedom) {
          QL_REQUIRE(degFreedom_ > 0, 
              "Invalid degrees of freedom parameter.");
        }

        //! returns a sample from a Student-t distribution
        sample_type next() const;
    private:
        URNG uniformGenerator_;
        mutable Real degFreedom_;
    };

    template <class URNG>
    inline typename PolarStudentTRng<URNG>::sample_type
    PolarStudentTRng<URNG>::next() const {
        Real u, v, rSqr;
        do{
            //samples remapped to [-1,1]:
            v = 2.* uniformGenerator_.next().value - 1.;
            u = 2.* uniformGenerator_.next().value - 1.;
            rSqr = v*v + u*u;
        }while(rSqr >= 1.);
        return {u * std::sqrt(degFreedom_ * (std::pow(rSqr, -2. / degFreedom_) - 1.) / rSqr), 1.};
    }

}

#endif


#ifndef id_e6e88e802520f953e34fdfc70df9353e
#define id_e6e88e802520f953e34fdfc70df9353e
inline bool test_e6e88e802520f953e34fdfc70df9353e(int* i) { return i != 0; }
#endif

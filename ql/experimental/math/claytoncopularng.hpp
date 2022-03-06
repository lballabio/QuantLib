/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Hachemi Benyahia
 Copyright (C) 2010 DeriveXperts SAS

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

/*! \file claytoncopularng.hpp
    \brief Clayton copula random-number generator
*/

#ifndef quantlib_clayton_copula_rng_hpp
#define quantlib_clayton_copula_rng_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/errors.hpp>
#include <vector>

namespace QuantLib {

    //! Clayton copula random-number generator
    template <class RNG>
    class ClaytonCopulaRng {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        typedef RNG urng_type;
        explicit ClaytonCopulaRng(const RNG& uniformGenerator,Real theta);
        sample_type next() const;
      private:
        Real theta_;
        RNG uniformGenerator_;
    };

    template <class RNG>
    ClaytonCopulaRng<RNG>::ClaytonCopulaRng(const RNG& ug, Real th)
    : uniformGenerator_(ug), theta_(th) {
        QL_REQUIRE(th >= -1.0,
                   "theta (" << th << ") must be greater or equal to -1");
        QL_REQUIRE(th != 0.0,
                   "theta (" << th << ") must be different from 0");
    }

    template <class RNG>
    inline typename ClaytonCopulaRng<RNG>::sample_type
    ClaytonCopulaRng<RNG>::next() const {
        typename RNG::sample_type v1 = uniformGenerator_.next();
        typename RNG::sample_type v2 = uniformGenerator_.next();
        Real u1 = v1.value;
        Real u2 = std::pow(std::pow(v1.value,-theta_)*(std::pow(v2.value,-theta_/(theta_+1.0))-1.0)+1.0,-1.0/theta_);
        std::vector<Real> u;
        u.push_back(u1);
        u.push_back(u2);
        return sample_type(u,v1.weight*v2.weight);
    }

}


#endif


#ifndef id_3e5200782da96dea0159f81e3f6a5dc3
#define id_3e5200782da96dea0159f81e3f6a5dc3
inline bool test_3e5200782da96dea0159f81e3f6a5dc3(int* i) { return i != 0; }
#endif

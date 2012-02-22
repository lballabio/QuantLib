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

/*! \file farliegumbelmorgensterncopularng.hpp
    \brief Farlie-Gumbel-Morgenstern copula random-number generator
*/

#ifndef quantlib_farlie_gumbel_morgenstern_copula_rng_hpp
#define quantlib_farlie_gumbel_morgenstern_copula_rng_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/errors.hpp>
#include <vector>

namespace QuantLib {

    //! Farlie-Gumbel-Morgenstern copula random-number generator
    template <class RNG>
    class FarlieGumbelMorgensternCopulaRng {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        typedef RNG urng_type;
        explicit FarlieGumbelMorgensternCopulaRng(const RNG& uniformGenerator,
                                                  Real theta);
        sample_type next() const;
      private:
        Real theta_;
        RNG uniformGenerator_;
    };

    template <class RNG>
    FarlieGumbelMorgensternCopulaRng<RNG>::FarlieGumbelMorgensternCopulaRng(
                                                       const RNG& ug, Real th)
    : uniformGenerator_(ug), theta_(th) {
        QL_REQUIRE(th >= -1.0 && th <= 1.00,
                   "theta (" << th << ") must be in [-1,1]");
    }

    template <class RNG>
    inline typename FarlieGumbelMorgensternCopulaRng<RNG>::sample_type
    FarlieGumbelMorgensternCopulaRng<RNG>::next() const {
        typename RNG::sample_type v1 = uniformGenerator_.next();
        typename RNG::sample_type v2 = uniformGenerator_.next();
        Real u1 = v1.value;
        Real a = theta_*(2.0*u1-1.0);
        Real b = pow(1.0-theta_*(2.0*u1-1.0),2.0)+4.0*theta_*v2.value*(2.0*u1-1.0);
        Real u2 = (2.0*v2.value)/(sqrt(b)-a);
        std::vector<Real> u;
        u.push_back(u1);
        u.push_back(u2);
        return sample_type(u,v1.weight*v2.weight);
    }

}


#endif

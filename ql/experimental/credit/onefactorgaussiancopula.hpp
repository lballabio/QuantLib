/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file onefactorgaussiancopula.hpp
    \brief One-factor Gaussian copula
*/

#ifndef quantlib_one_factor_gaussian_copula_hpp
#define quantlib_one_factor_gaussian_copula_hpp

#include <ql/experimental/credit/onefactorcopula.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! One-factor Gaussian Copula
    /*! The copula model
        \f[ Y_i = a_i\,M+\sqrt{1-a_i^2}\:Z_i \f]
        is specified here by setting the desnity function for all
        variables, \f$ M, Z,\f$ and also \f$ Y \f$ to the standard
        normal distribution
        \f$ \phi(x) = \exp(-x^2/2) / \sqrt{2\pi}. \f$
    */
    class OneFactorGaussianCopula : public OneFactorCopula {
      public:
        explicit OneFactorGaussianCopula(const Handle<Quote>& correlation,
                                         Real maximum = 5, Size integrationSteps = 50)
        : OneFactorCopula (correlation, maximum, integrationSteps) {
            registerWith(correlation_);
        }
        Real density(Real m) const override;
        Real cumulativeZ(Real z) const override;
        /*! overrides the base class implementation based on table data */
        Real cumulativeY(Real y) const override;
        Real testCumulativeY (Real y) const;
        /*! overrides the base class implementation based on table data */
        Real inverseCumulativeY(Real p) const override;

      private:
        // nothing to be done when correlation changes
        void performCalculations() const override {}

        NormalDistribution density_;
        CumulativeNormalDistribution cumulative_;
        InverseCumulativeNormal inverseCumulative_;
    };

    inline Real OneFactorGaussianCopula::density (Real m) const {
        return density_(m);
    }

    inline Real OneFactorGaussianCopula::cumulativeZ (Real z) const {
        return cumulative_(z);
    }

    inline Real OneFactorGaussianCopula::cumulativeY (Real y) const {
        return cumulative_(y);
    }

    inline Real OneFactorGaussianCopula::inverseCumulativeY (Real p) const {
        return inverseCumulative_(p);
    }

}

#endif

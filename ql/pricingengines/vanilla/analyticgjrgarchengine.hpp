/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Yee Man Chan

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

/*! \file analyticgjrgarchengine.hpp
    \brief analytic GJR-GARCH-model engine
*/

#ifndef quantlib_analytic_gjrgarch_engine_hpp
#define quantlib_analytic_gjrgarch_engine_hpp

#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/models/equity/gjrgarchmodel.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <complex>

namespace QuantLib {

    //! GJR-GARCH(1,1) engine
    /*! References:

    Jin-Chuan Duan, Genevieve Gauthier, Jean-Guy Simonato, 
    Caroline Sasseville, 2006. Approximating the GJR-GARCH
    and EGARCH option pricing models analytically
    Journal of Computational Finance, Volume 9, Number 3,
    Spring 2006

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in the Duan et al's
              2006 paper.
    */

    class AnalyticGJRGARCHEngine
        : public GenericModelEngine<GJRGARCHModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        AnalyticGJRGARCHEngine(const boost::shared_ptr<GJRGARCHModel>& model);
        void calculate() const;
        // call back for extended stochastic volatility
        // plus jump diffusion engines like bates model
      private:
// store parameters
    mutable bool init_;
    mutable Real h1_;
    mutable Real b0_;
    mutable Real b1_;
    mutable Real b2_;
    mutable Real b3_;
    mutable Real la_;
    mutable Real r_;
    mutable Size T_;
// intermediate constants determined by b1,b2,b3,la
    mutable Real m1_;
    mutable Real m2_;
    mutable Real m3_;
    mutable Real v1_;
    mutable Real v2_;
    mutable Real v3_;
    mutable Real z1_;
    mutable Real z2_;
    mutable Real x1_;
// statistical data for the GJR-GARCH process determined by
// h1,b0,b1,b2,b3,r,T
    mutable Real ex_; // mean
    mutable Real sigma_; // variance
    mutable Real k3_; // skewness
    mutable Real k4_; // kurtosis
    };

}

#endif

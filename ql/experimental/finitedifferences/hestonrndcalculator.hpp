/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file hestonrndcalculator.hpp
    \brief risk neutral terminal density calculator for the
           Heston stochastic volatility model
*/

#ifndef quantlib_heston_risk_neutral_density_calculator_hpp
#define quantlib_heston_risk_neutral_density_calculator_hpp

#include <ql/experimental/finitedifferences/riskneutraldensitycalculator.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {
    class HestonProcess;

    //! Risk neutral terminal probability density for the Heston model

    /*! References:

        The formulas are taken from A. Dragulescu, V. Yakovenko, 2002.
        Probability distribution of returns in the Heston model
        with stochastic volatility.
        http://arxiv.org/pdf/cond-mat/0203046.pdf
     */

    class HestonRNDCalculator : public RiskNeutralDensityCalculator {
    public:
        HestonRNDCalculator(
            const boost::shared_ptr<HestonProcess>& hestonProcess,
            Real integrationEps= 1e-6,
            Size maxIntegrationIterations = 10000ul);

        // x=ln(S)
        Real pdf(Real x, Time t) const;
        Real cdf(Real x, Time t) const;
        Real invcdf(Real q, Time t) const;

    private:
        Real x_t(Real x, Time t) const;

        const boost::shared_ptr<HestonProcess> hestonProcess_;
        const Real x0_;
        const Real integrationEps_;
        const Size maxIntegrationIterations_;
    };
}

#endif

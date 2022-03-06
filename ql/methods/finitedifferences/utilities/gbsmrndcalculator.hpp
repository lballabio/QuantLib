/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

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

/*! \file gbsmrndcalculator.hpp
    \brief risk neutral terminal density calculator for the
           Black-Scholes-Merton model with strike dependent volatility
*/

#ifndef quantlib_gbsm_risk_neutral_density_calculator_hpp
#define quantlib_gbsm_risk_neutral_density_calculator_hpp

#include <ql/methods/finitedifferences/utilities/riskneutraldensitycalculator.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {
    class GeneralizedBlackScholesProcess;

    class GBSMRNDCalculator : public RiskNeutralDensityCalculator {
    public:
      explicit GBSMRNDCalculator(ext::shared_ptr<GeneralizedBlackScholesProcess> process);

      Real pdf(Real s, Time t) const override;
      Real cdf(Real s, Time t) const override;
      Real invcdf(Real q, Time t) const override;

    private:
      const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };
}

#endif


#ifndef id_f2c5e26083d70167b9004f4637bb98b7
#define id_f2c5e26083d70167b9004f4637bb98b7
inline bool test_f2c5e26083d70167b9004f4637bb98b7(const int* i) {
    return i != nullptr;
}
#endif

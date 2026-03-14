/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Göttker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file squarerootprocessrndcalculator.hpp
    \brief risk neutral terminal density calculator for the square root process
*/

#ifndef quantlib_square_root_process_risk_neutral_density_calculator_hpp
#define quantlib_square_root_process_risk_neutral_density_calculator_hpp

#include <ql/methods/finitedifferences/utilities/riskneutraldensitycalculator.hpp>

namespace QuantLib {
    class SquareRootProcessRNDCalculator : public RiskNeutralDensityCalculator {
      public:
        SquareRootProcessRNDCalculator(
            Real v0, Real kappa, Real theta, Real sigma);

        Real pdf(Real v, Time t) const override;
        Real cdf(Real v, Time t) const override;
        Real invcdf(Real q, Time t) const override;

        Real stationary_pdf(Real v) const;
        Real stationary_cdf(Real v) const;
        Real stationary_invcdf(Real q) const;

      private:
        const Real v0_, kappa_, theta_, d_, df_;
    };
}

#endif

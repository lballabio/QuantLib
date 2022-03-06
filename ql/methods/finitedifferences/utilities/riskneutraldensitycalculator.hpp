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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file riskneutraldensitycalculator.hpp
    \brief interface for a single asset risk neutral terminal density calculation
*/

#ifndef quantlib_risk_neutral_density_calculator_hpp
#define quantlib_risk_neutral_density_calculator_hpp

#include <ql/types.hpp>

namespace QuantLib {
    class RiskNeutralDensityCalculator {
      public:
        virtual Real pdf(Real x, Time t) const = 0;
        virtual Real cdf(Real x, Time t) const = 0;
        virtual Real invcdf(Real p, Time t) const = 0;

        virtual ~RiskNeutralDensityCalculator() = default;

      protected:
        class InvCDFHelper {
          public:
            InvCDFHelper(const RiskNeutralDensityCalculator* calculator,
                         Real guess, Real accuracy, Size maxEvaluations,
                         Real stepSize=0.01);

            Real inverseCDF(Real p, Time t) const;
          private:
            const RiskNeutralDensityCalculator* const calculator_;
            const Real guess_;
            const Real accuracy_;
            const Size maxEvaluations_;
            const Real stepSize_;
        };
    };
}

#endif


#ifndef id_6ce7ab94f41c815f47c7ede6f0dac336
#define id_6ce7ab94f41c815f47c7ede6f0dac336
inline bool test_6ce7ab94f41c815f47c7ede6f0dac336(int* i) { return i != 0; }
#endif

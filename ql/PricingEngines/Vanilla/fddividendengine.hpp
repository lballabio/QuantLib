
/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fddividendengine.hpp
    \brief base engine for option with dividends
*/

#ifndef quantlib_fd_dividend_engine_hpp
#define quantlib_fd_dividend_engine_hpp

#include <ql/PricingEngines/Vanilla/fdmultiperiodengine.hpp>

namespace QuantLib {

    //! Finite-differences pricing engine for dividend European options
    /*! \ingroup vanillaengines

        \test
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
        - the invariance of the results upon addition of null
          dividends is tested.
    */
    class FDDividendEngine : public FDMultiPeriodEngine {
      public:
        FDDividendEngine(Size timeSteps = 100,
                         Size gridPoints = 100,
                         bool timeDependent = false)
        : FDMultiPeriodEngine(timeSteps, gridPoints,
                              timeDependent) {}
      private:
        void executeIntermediateStep(Size step) const;
        void movePricesBeforeExDiv(Array& prices,
                                   const Array& newGrid,
                                   const Array& oldGrid) const;
        Real addElements(const std::vector<Real>& A) const{
            return std::accumulate(A.begin(), A.end(), 0.0);
        }
        Real getDividend(int i) const {
            return arguments_.dividends[i];
        }
    };

}


#endif

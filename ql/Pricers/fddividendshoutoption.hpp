
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file fddividendshoutoption.hpp
    \brief base class for shout option with dividends
*/

#ifndef quantlib_dividend_shout_option_pricer_h
#define quantlib_dividend_shout_option_pricer_h

#include <ql/Pricers/fddividendoption.hpp>
#include <ql/FiniteDifferences/shoutcondition.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED

    //! Shout option with dividends
    /*! \deprecated use DividendVanillaOption with FDDividendShoutEngine
                    instead
    */
    class FdDividendShoutOption : public FdDividendOption {
      public:
        // constructor
        FdDividendShoutOption(
                 Option::Type type, Real underlying,
                 Real strike, Spread dividendYield, Rate riskFreeRate,
                 Time residualTime, Volatility volatility,
                 const std::vector<Real>& dividends = std::vector<Real>(),
                 const std::vector<Time>& exdivdates = std::vector<Time>(),
                 Size timeSteps = 100, Size gridPoints = 100);

        boost::shared_ptr<SingleAssetOption> clone() const;
        Real dividendRho() const {
            QL_FAIL("not implemented");
        }
      protected:
        void initializeStepCondition() const;
    };

    #endif

}


#endif

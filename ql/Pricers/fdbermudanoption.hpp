
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

/*! \file fdbermudanoption.hpp
    \brief finite-difference evaluation of Bermudan option
*/

#ifndef quantlib_bermudan_option_pricer_h
#define quantlib_bermudan_option_pricer_h

#include <ql/Pricers/fdmultiperiodoption.hpp>

namespace QuantLib {

    //! Bermudan option
    class FdBermudanOption : public FdMultiPeriodOption {
      public:
        // constructor
        FdBermudanOption(Option::Type type, Real underlying,
                         Real strike, Spread dividendYield, 
                         Rate riskFreeRate, Time residualTime, 
                         Volatility volatility,
                         const std::vector<Time>& dates = std::vector<Time>(),
                         Size timeSteps = 100, Size gridPoints = 100);
        boost::shared_ptr<SingleAssetOption> clone() const;
      protected:
        Real extraTermInBermudan ;
        void initializeStepCondition() const;
        void executeIntermediateStep(Size ) const;
    };

}


#endif

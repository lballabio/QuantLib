
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file performanceoption.hpp
    \brief Performance option
*/

#ifndef quantlib_pricers_performance_option_h
#define quantlib_pricers_performance_option_h

#include <ql/Pricers/europeanoption.hpp>

namespace QuantLib {

    //! Performance option
    /*! A performance option is a variant of a cliquet option: the
        payoff of each forward-starting (a.k.a. deferred strike)
        options is \$ max(S/X- 1) \$.
    */
    class PerformanceOption {
      public:
        PerformanceOption(Option::Type type,
                          double underlying,
                          double moneyness,
                          const std::vector<Spread>& dividendYield,
                          const std::vector<Rate>& riskFreeRate,
                          const std::vector<Time>& times,
                          const std::vector<double>& volatility);
        double value() const;
        double delta() const;
        double gamma() const;
        double theta() const;
        double vega() const;
        double rho() const;
        double dividendRho() const;
      private:
        double moneyness_;
        std::vector<Rate> riskFreeRate_;
        std::vector<Time> times_;
        Size numOptions_;
        std::vector<boost::shared_ptr<EuropeanOption> > optionlet_;
        std::vector<DiscountFactor> discounts_;
    };

}


#endif





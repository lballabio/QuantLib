
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

/*! \file Pricers/cliquetoption.hpp
    \brief Cliquet option
*/

#ifndef quantlib_pricers_cliquet_option_h
#define quantlib_pricers_cliquet_option_h

#include <ql/Pricers/europeanoption.hpp>

namespace QuantLib {

    //! cliquet (Ratchet) option
    /*! A cliquet option, also known as Ratchet option, is a series of
        forward-starting (a.k.a. deferred strike) options where the
        strike for each forward start option is set equal to a fixed
        percentage of the spot price at the beginning of each period.

        In the particular case in which only two dates are given the
        cliquet option is the same as a forward-starting option
        starting at the first date and expiring at the second date.
    */
    class CliquetOptionPricer {
      public:
        CliquetOptionPricer(Option::Type type,
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
        std::vector<Time> times_;
        std::vector<Spread> dividendYield_;
        Size numOptions_;
        std::vector<Handle<EuropeanOption> > optionlet_;
        std::vector<double> weight_;
        std::vector<DiscountFactor> forwardDiscounts_;
    };

}


#endif





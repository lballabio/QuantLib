

/*
 Copyright (C) 2002 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file cliquetoption.hpp
    \brief Textbook example of european-style multi-period option.

    \fullpath
    ql/Pricers/%cliquetoption.hpp
*/

// $Id$

#ifndef quantlib_pricers_cliquet_option_h
#define quantlib_pricers_cliquet_option_h

#include <ql/Pricers/europeanoption.hpp>
#include <vector>

namespace QuantLib {

    namespace Pricers {

        //! cliquet (Ratchet) option
        /*! A cliquet option, also known as Ratchet option, is a series
            of forward-starting options where the strike for the next
            exercize date is set to the spot price at the beginning of each
            period.
            In the particular case in which only two dates are given the
            price of the option is the same as that of a forward-starting
            option starting at the first date and expiring at the second
            date.
        */
        class CliquetOption : public SingleAssetOption {
          public:
            CliquetOption(Option::Type type,
                          double underlying,
                          double moneyness,
                          Spread dividendYield,
                          Rate riskFreeRate,
                          const std::vector<Time>& times,
                          double volatility);
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            double dividendRho() const;
            Handle<SingleAssetOption> clone() const;
          private:
            double moneyness_;
            Rate riskFreeRate_;
            std::vector<Time> times_;
            int numPeriods_;
            std::vector<Handle<EuropeanOption> > optionlet_;
            std::vector<double> weight_;
        };

    }

}


#endif





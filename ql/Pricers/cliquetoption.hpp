
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
                          Spread dividendYield,
                          Rate riskFreeRate,
                          const std::vector<Time> &dates,
                          double volatility);
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            Handle<SingleAssetOption> clone() const;
          private:
            int numPeriods_;
            std::vector<Handle<EuropeanOption> > optionlet_;
            std::vector<double> weight_;
        };

    }

}


#endif





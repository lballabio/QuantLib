
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file cliquetoption.hpp
    \brief Textbook example of european-style multi-period option.

    \fullpath
    ql/Pricers/%cliquetoption.hpp
*/

// $Id$

#ifndef quantlib_pricers_cliquet_option_h
#define quantlib_pricers_cliquet_option_h

#include "ql/Pricers/europeanoption.hpp"
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
                          Rate dividendYield,
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





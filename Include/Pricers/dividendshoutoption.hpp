/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at 
 *      http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file dividendshoutoption.hpp
    \brief base class for shout option with dividends
    
    $Source$
    $Log$
    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/03/20 15:15:55  marmar
    DividendOption and DividendShoutOption are examples of
     MultiPeriodOption's
    
*/

#ifndef shaft_dividend_shout_option_pricer_h
#define shaft_dividend_shout_option_pricer_h

#include "qldefines.hpp"
#include "dividendoption.hpp"
#include "shoutcondition.hpp"

namespace QuantLib {

    namespace Pricers {

        class DividendShoutOption : public DividendOption {
          public:
            // constructor
            DividendShoutOption(Type type, double underlying, 
                double strike, Rate dividendYield, Rate riskFreeRate, 
                Time residualTime, double volatility, 
                const std::vector<double>& dividends = std::vector<double>(), 
                const std::vector<Time>& exdivdates = std::vector<Time>(), 
                int timeSteps = 100, int gridPoints = 100);
                
            Handle<BSMOption> clone() const { 
                return Handle<BSMOption>(new DividendShoutOption(*this));
            }
          private:
            void initializeStepCondition() const;
        };

    }

}

#endif

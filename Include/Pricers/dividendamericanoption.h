     
/*
 * Copyright (C) 2000
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file dividendamericanoption.h
    \brief american option with discrete dividends
    
    $Source$
    $Name$
    $Log$
    Revision 1.9  2001/01/10 16:35:35  nando
    timeStepPerDiv was double now it is int

    Revision 1.8  2001/01/08 11:44:17  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.7  2001/01/08 10:28:16  lballabio
    Moved Array to Math namespace

    Revision 1.6  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks
    
*/

#ifndef shaft_dividend_american_option_pricer_h
#define shaft_dividend_american_option_pricer_h

#include "qldefines.h"
#include "bsmnumericaloption.h"
#include "americancondition.h"
#include <vector>

/*
#include "date.h"
#include "dividendeuropeanoption.h"
#include "finitedifferencemodel.h"
#include "handle.h"

#include "finitedifferences.h"
#include "newcubicspline.h"
#include "cranknicolson.h"
#include "bsmnumericaloption.h"
#include "americancondition.h"
*/
namespace QuantLib {
    namespace Pricers {

        class DividendAmericanOption : public BSMNumericalOption {
          public:
            // constructor
            DividendAmericanOption(Type type, double underlying, 
                double strike, Rate underlyingGrowthRate, Rate riskFreeRate, 
                Time residualTime, double volatility, 
                const std::vector<double>& dividends, 
                const std::vector<Time>& exdivdates, int timeSteps, 
                int gridPoints);
            // accessors
            double value() const;
            private:
            mutable bool theOptionIsAmerican;
            int timeStepPerDiv;
            std::vector<double> theDividends;
            std::vector<Time> theExDivDates;
            unsigned int  theNumberOfDivs;
            void movePricesBeforeExDiv(double Div, const Array& newGrid, 
                Array& prices, const Array& oldGrid) const;
            Handle<BSMOption> clone() const { 
                return Handle<BSMOption>(new DividendAmericanOption(*this));
            }
            double addElements(const std::vector<double>& A) const{
                double sum = 0.0;
                return sum = std::accumulate(A.begin(),A.end(),sum);   
            }
        };

    }
}


#endif

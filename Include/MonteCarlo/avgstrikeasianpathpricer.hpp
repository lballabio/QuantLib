
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file avgstrikeasianpathpricer.hpp

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/02/13 15:05:23  lballabio
    Trimmed a couple of long file names for Mac compatibility

    Revision 1.1  2001/02/05 16:51:10  marmar
    AverageAsianPathPricer substituted by AveragePriceAsianPathPricer
    and AverageStrikeAsianPathPricer

*/

#ifndef quantlib_montecarlo_average_strike_asian_path_pricer_h
#define quantlib_montecarlo_average_strike_asian_path_pricer_h

#include "qldefines.hpp"
#include "europeanpathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        class AverageStrikeAsianPathPricer : public EuropeanPathPricer {
        public:
            AverageStrikeAsianPathPricer(): EuropeanPathPricer() {}
            AverageStrikeAsianPathPricer(Option::Type type,
                double underlying, double strike, double discount);
            virtual double value(const Path &path) const;
        };

    }

}


#endif


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

/*! \file himalayapathpricer.hpp
    $Source$
    $Log$
    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/02/07 10:13:00  marmar
    Himalaya-type path pricer


*/

#ifndef quantlib_himalaya_path_pricer_h
#define quantlib_himalaya_path_pricer_h

#include <vector>

#include "qldefines.hpp"
#include "date.hpp"
#include "multipathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {
    /*! HimalayaPathPricer evaluates the european-type himalaya option
        on a multi-path.
        The payoff of an himalaya option is computed in the following way:
            Given a basket of N assets, and N time periods, at end of
            each period the option who performed the best is added to the
            average and then discarded from the basket. At the end of the
            N periods the option pays the max between the strike and the
            average of the best performers.
    */

        class HimalayaPathPricer : public MultiPathPricer {
        public:
            HimalayaPathPricer():MultiPathPricer(){}
            HimalayaPathPricer(const Array &underlying,
                double strike, double discount);
            double value(const MultiPath &path) const;
        protected:
            double discount_;
            Array underlying_;
            double strike_;
        };

    }

}

#endif

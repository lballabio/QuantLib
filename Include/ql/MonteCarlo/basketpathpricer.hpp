
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

/*! \file basketpathpricer.hpp
    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/05/24 11:34:07  nando
    smoothing #include xx.hpp

    Revision 1.2  2001/05/23 19:30:27  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

*/

#ifndef quantlib_basket_path_pricer_h
#define quantlib_basket_path_pricer_h

#include "ql/MonteCarlo/multipathpricer.hpp"
#include "ql/options.hpp"

namespace QuantLib {

    namespace MonteCarlo {
        /*! BasketPathPricer evaluates the european-type basket option
            on a multi-path.
            The value of the option at expriration is given by the value
            of the underlying which has best performed
        */

        class BasketPathPricer : public MultiPathPricer {
        public:
            BasketPathPricer():MultiPathPricer(){}
            BasketPathPricer(const Array &underlying, double discount);
            double value(const MultiPath &path) const;
        protected:
            double discount_;
            Array underlying_;
        };

    }

}

#endif

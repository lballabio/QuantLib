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
 *          http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file basketpathpricer.h    
    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/02/02 10:52:54  marmar
    Example of a path pricer depending on multiple factors

*/

#ifndef quantlib_basket_path_pricer_h
#define quantlib_basket_path_pricer_h

#include "qldefines.h"
#include "multipathpricer.h"
#include "options.h"

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
            mutable double discount_;
            mutable Array underlying_;
        };

    }

}

#endif

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

/*! \file pagodapathpricer.hpp   
    $Source$
    $Log$
    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/03/22 12:04:40  marmar
    Introducing pagoda options


*/

#ifndef quantlib_pagoda_path_pricer_h
#define quantlib_pagoda_path_pricer_h

#include "qldefines.hpp"
#include "multipathpricer.hpp"
#include "options.hpp"

namespace QuantLib {

    namespace MonteCarlo {
        /*! \brief A pagoda option is a roofed asian option.
        Given a certain portfolio, of assets at the end of the period        
        it is returned the minimum of a given roof and a certain fraction of 
        the positive portfolio performance. 
        If the performance of the portfolio is below then option is null.
        */

        class PagodaPathPricer : public MultiPathPricer {
        public:
            PagodaPathPricer():MultiPathPricer(){}
            PagodaPathPricer(const Array &underlying, double roof, 
                             double discount);
            double value(const MultiPath &path) const;
        protected:
            double roof_, discount_;
            Array underlying_;
        };

    }

}

#endif

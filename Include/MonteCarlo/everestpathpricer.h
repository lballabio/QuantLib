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

/*! \file everestpathpricer.h    
    $Source$
    $Log$
    Revision 1.1  2001/03/06 16:58:41  marmar
    First, simplified version, of everest option

*/

#ifndef quantlib_everest_path_pricer_h
#define quantlib_everest_path_pricer_h

#include <vector>

#include "qldefines.h"
#include "date.h"
#include "multipathpricer.h"

namespace QuantLib {

    namespace MonteCarlo {
    /*! EverestPathPricer evaluates the european-type everest option     
        on a multi-path. 
        The payoff of an everest option is simply given by the
        final-price initial-price ratio of the worst performer
    */

        class EverestPathPricer : public MultiPathPricer {
        public:
            EverestPathPricer():MultiPathPricer(){}
            EverestPathPricer(const Array &underlying, 
                double discount);
            double value(const MultiPath &path) const;
        protected:
            double discount_;
            Array underlying_;
        };

    }

}

#endif

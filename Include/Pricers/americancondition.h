
/*
 * Copyright (C) 2000, 2001
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
 *   http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file americancondition.h
    \brief american option exercise condition

    $Source$
    $Name$
    $Log$
    Revision 1.8  2001/03/01 13:56:44  marmar
    Methods are now inlined explicitly

    Revision 1.7  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.6  2001/01/08 11:44:17  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.5  2001/01/08 10:28:16  lballabio
    Moved Array to Math namespace

    Revision 1.4  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef BSM_american_condition_h
#define BSM_american_condition_h

#include "qldefines.h"
#include "standardstepcondition.h"
#include <algorithm>
#include <functional>

namespace QuantLib {

    namespace Pricers {

        class BSMAmericanCondition : 
                public FiniteDifferences::StandardStepCondition {
          public:
            BSMAmericanCondition(const Array& initialPrices);
            void applyTo(Array& a, Time t) const;
          private:
            Array initialPrices_;
        };

        inline BSMAmericanCondition::BSMAmericanCondition(
                    const Array& initialPrices) 
            : initialPrices_(initialPrices) {}
            
        inline void BSMAmericanCondition::applyTo(Array& a, Time t) const {
                for (int i = 0; i < a.size(); i++)
                    a[i] = QL_MAX(a[i], initialPrices_[i]);
            }

    }

}

#endif

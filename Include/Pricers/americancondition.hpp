
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

/*! \file americancondition.hpp
    \brief american option exercise condition

    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.9  2001/03/02 08:36:44  enri
    Shout options added:
    	* BSMAmericanOption is now AmericanOption, same interface
    	* ShoutOption added
    	* both ShoutOption and AmericanOption inherit from
    	  StepConditionOption
    offline.doxy.linux added.

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

#include "qldefines.hpp"
#include "FiniteDifferences/standardstepcondition.hpp"
#include <algorithm>
#include <functional>

namespace QuantLib {

    namespace Pricers {

        class AmericanCondition :
            public FiniteDifferences::StandardStepCondition {
        public:
            AmericanCondition(const Array& initialPrices);
            void applyTo(Array& a, Time t) const;
        private:
            Array initialPrices_;
        };

        inline AmericanCondition::AmericanCondition(
            const Array& initialPrices)
            : initialPrices_(initialPrices) {}

        inline void AmericanCondition::applyTo(Array& a, Time t) const {
            for (int i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i], initialPrices_[i]);
        }
    }
}

#endif


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
    $Log$
    Revision 1.3  2001/05/24 13:57:51  nando
    smoothing #include xx.hpp and cutting old Log messages

*/

#ifndef BSM_american_condition_h
#define BSM_american_condition_h

#include "ql/FiniteDifferences/standardstepcondition.hpp"

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

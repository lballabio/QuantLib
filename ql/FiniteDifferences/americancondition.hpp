
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file americancondition.hpp
    \brief american option exercise condition

    \fullpath
    ql/FiniteDifferences/%americancondition.hpp
*/

// $Id$

#ifndef quantlib_fd_american_condition_h
#define quantlib_fd_american_condition_h

#include <ql/FiniteDifferences/fdtypedefs.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        class AmericanCondition
        : public FiniteDifferences::StandardStepCondition {
          public:
            AmericanCondition(const Array& initialPrices);
            void applyTo(Array& a,
                         Time t) const;
          private:
            Array initialPrices_;
        };


        // inline definitions

        inline AmericanCondition::AmericanCondition(
            const Array& initialPrices)
            : initialPrices_(initialPrices) {}

        inline void AmericanCondition::applyTo(Array& a, Time t) const {
            for (Size i = 0; i < a.size(); i++)
                a[i] = QL_MAX(a[i], initialPrices_[i]);
        }

    }

}


#endif

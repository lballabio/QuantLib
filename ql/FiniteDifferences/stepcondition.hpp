
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file stepcondition.hpp
    \brief conditions to be applied at every time step
*/

// $Id$

#ifndef quantlib_step_condition_h
#define quantlib_step_condition_h

#include <ql/date.hpp>
#include <ql/numericalmethod.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        //! condition to be applied at every time step
        template <class arrayType>
        class StepCondition {
          public:
            virtual ~StepCondition() {}
            virtual void applyTo(arrayType& a,
                                 Time t) const = 0;
            virtual void applyTo(Handle<DiscretizedAsset> asset) const = 0;
        };

    }

}


#endif

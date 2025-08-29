/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmamericanstepcondition.hpp
    \brief american step condition for multi dimensional problems
*/

#ifndef quantlib_fdm_american_step_condition_hpp
#define quantlib_fdm_american_step_condition_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>

namespace QuantLib {

    class FdmAmericanStepCondition : public StepCondition<Array> {
      public:
        FdmAmericanStepCondition(ext::shared_ptr<FdmMesher> mesher,
                                 ext::shared_ptr<FdmInnerValueCalculator> calculator);

        void applyTo(Array& a, Time) const override;

      private:
        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<FdmInnerValueCalculator> calculator_;
    };
}
#endif

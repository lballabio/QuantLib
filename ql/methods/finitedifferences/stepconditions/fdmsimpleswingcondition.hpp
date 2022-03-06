/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmsimpleswingcondition.hpp
    \brief simple swing step condition
*/

#ifndef quantlib_fdm_simple_swing_condition_hpp
#define quantlib_fdm_simple_swing_condition_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>

namespace QuantLib {

    class FdmSimpleSwingCondition : public StepCondition<Array> {
      public:
        FdmSimpleSwingCondition(std::vector<Time> exerciseTimes,
                                ext::shared_ptr<FdmMesher> mesher,
                                ext::shared_ptr<FdmInnerValueCalculator> calculator,
                                Size swingDirection,
                                Size minExercises = 0);

        void applyTo(Array& a, Time t) const override;

      private:
        const std::vector<Time> exerciseTimes_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<FdmInnerValueCalculator> calculator_;
        const Size minExercises_;
        const Size swingDirection_;
    };
}
#endif


#ifndef id_8dd0df923fea5af6bb55c9100d19b856
#define id_8dd0df923fea5af6bb55c9100d19b856
inline bool test_8dd0df923fea5af6bb55c9100d19b856(const int* i) {
    return i != nullptr;
}
#endif

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

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

/*! \file fdconditions.hpp
    \brief Finite-difference templates to generate engines
*/

#ifndef quantlib_fd_conditions_hpp
#define quantlib_fd_conditions_hpp

#include <ql/methods/finitedifferences/fdtypedefs.hpp>
#include <ql/methods/finitedifferences/americancondition.hpp>
#include <ql/methods/finitedifferences/shoutcondition.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/interestrate.hpp>

namespace QuantLib {

    /*! \deprecated Use the new finite-differences framework instead.
                    Deprecated in version 1.22.
    */
    template <typename baseEngine>
    class QL_DEPRECATED FDAmericanCondition : public baseEngine {
      public:
        FDAmericanCondition(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps = 100, Size gridPoints = 100,
             bool timeDependent = false)
        : baseEngine(process, timeSteps, gridPoints, timeDependent) {}
      protected:
        void initializeStepCondition() const {

            QL_DEPRECATED_DISABLE_WARNING

            baseEngine::stepCondition_ =
                ext::shared_ptr<StandardStepCondition>(
                  new AmericanCondition(baseEngine::intrinsicValues_.values()));

            QL_DEPRECATED_ENABLE_WARNING

        }
    };

    template <typename baseEngine>
    class FDShoutCondition : public baseEngine {
      public:
        FDShoutCondition(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps = 100, Size gridPoints = 100,
             bool timeDependent = false)
        : baseEngine(process, timeSteps, gridPoints, timeDependent) {}
      protected:
        void initializeStepCondition() const override {
            Time residualTime = baseEngine::getResidualTime();
            Rate riskFreeRate = baseEngine::process_->riskFreeRate()
                ->zeroRate(residualTime, Continuous);

            baseEngine::stepCondition_ =
                ext::shared_ptr<StandardStepCondition>(
                     new ShoutCondition(baseEngine::intrinsicValues_.values(),
                                        residualTime,
                                        riskFreeRate));
        }
    };

}


#endif


#ifndef id_a07a65a1a3adb14ef40be44e9703caa4
#define id_a07a65a1a3adb14ef40be44e9703caa4
inline bool test_a07a65a1a3adb14ef40be44e9703caa4(const int* i) {
    return i != nullptr;
}
#endif

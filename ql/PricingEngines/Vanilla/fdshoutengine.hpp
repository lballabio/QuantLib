
/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdshoutengine.hpp
    \brief Finite-differences shout engine
*/

#ifndef quantlib_fd_shout_engine_hpp
#define quantlib_fd_shout_engine_hpp

#include <ql/PricingEngines/Vanilla/fdstepconditionengine.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/FiniteDifferences/shoutcondition.hpp>

namespace QuantLib {

    //! Finite-differences pricing engine for shout vanilla options
    /*! \ingroup vanillaengines */
    class FDShoutEngine : public FDStepConditionEngine {
      public:
        FDShoutEngine(Size timeSteps=100, Size gridPoints=100,
                      bool timeDependent = false)
        : FDStepConditionEngine(timeSteps, gridPoints, timeDependent) {}
      private:
        void initializeStepCondition() const {
            const boost::shared_ptr<BlackScholesProcess>& process =
                arguments_.blackScholesProcess;
            Time residualTime = getResidualTime();
            Rate riskFreeRate = process->riskFreeRate()->zeroRate(residualTime,
                                                                  Continuous);

            stepCondition_ = boost::shared_ptr<StandardStepCondition>(
                                          new ShoutCondition(intrinsicValues_,
                                                             residualTime,
                                                             riskFreeRate));
        }
    };

}


#endif

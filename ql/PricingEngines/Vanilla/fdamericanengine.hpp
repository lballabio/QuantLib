/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdamericanengine.hpp
    \brief Finite-differences American option engine
*/

#ifndef quantlib_fd_american_engine_hpp
#define quantlib_fd_american_engine_hpp

#include <ql/PricingEngines/Vanilla/fdstepconditionengine.hpp>
#include <ql/PricingEngines/Vanilla/fdconditions.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>



namespace QuantLib {

    //! Finite-differences pricing engine for American one asset options
    /*! \ingroup vanillaengines

        \test
        - the correctness of the returned value is tested by
          reproducing results available in literature.
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
    */
    class FDAmericanEngine : public OneAssetOption::engine,
        public FDAmericanCondition<FDStepConditionEngine> {
      public:
        FDAmericanEngine(Size timeSteps=100, Size gridPoints=100,
                         bool timeDependent = false)
        : FDAmericanCondition<FDStepConditionEngine>(timeSteps, gridPoints, timeDependent) {}
      private:
        void calculate() const {
            setupArguments(&arguments_);
            FDStepConditionEngine::calculate(&results_);
        }
    };

}


#endif

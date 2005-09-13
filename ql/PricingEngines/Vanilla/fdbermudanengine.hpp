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

/*! \file fdbermudanengine.hpp
    \brief finite-difference Bermudan engine
*/

#ifndef quantlib_fd_bermudan_engine_hpp
#define quantlib_fd_bermudan_engine_hpp

#include <ql/PricingEngines/Vanilla/fdmultiperiodengine.hpp>

namespace QuantLib {

    //! Finite-differences Bermudan engine
    /*! \ingroup vanillaengines */
    class FDBermudanEngine : public DividendVanillaOption::engine,
        public FDMultiPeriodEngine {
      public:
        // constructor
        FDBermudanEngine(Size timeSteps = 100,
                         Size gridPoints = 100,
                         bool timeDependent = false)
        : FDMultiPeriodEngine(timeSteps, gridPoints,
                              timeDependent) {}
        void calculate() {
            setupArguments(&arguments_, &arguments_);
            FDMultiPeriodEngine::calculate(&results_);
        }
      protected:
        Real extraTermInBermudan ;
        void initializeStepCondition() const {
            stepCondition_ = boost::shared_ptr<StandardStepCondition>(
                                                  new NullCondition<Array>());
        };
        void executeIntermediateStep(Size ) const {
            Size size = intrinsicValues_.size();
            for (Size j=0; j<size; j++)
                prices_[j] = std::max(prices_[j], intrinsicValues_[j]);
        }
    };

}


#endif

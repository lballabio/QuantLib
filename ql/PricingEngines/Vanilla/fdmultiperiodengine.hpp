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

/*! \file fdmultiperiodengine.hpp
    \brief base engine for options with events happening at specific times
*/

#ifndef quantlib_fd_multi_period_engine_hpp
#define quantlib_fd_multi_period_engine_hpp

#include <ql/PricingEngines/Vanilla/fdvanillaengine.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/event.hpp>

namespace QuantLib {

    class FDMultiPeriodEngine : public FDVanillaEngine {
      protected:
        FDMultiPeriodEngine(Size gridPoints=100, Size timeSteps=100,
                            bool timeDependent = false);
        mutable std::vector<boost::shared_ptr<Event> > events_;
        mutable std::vector<Time> stoppingTimes_;
        Size timeStepPerPeriod_;
        mutable SampledCurve prices_;
        void setupArguments(
               const OneAssetOption::arguments* args,
               const std::vector<boost::shared_ptr<Event> >& schedule) const {
            FDVanillaEngine::setupArguments(args);
            events_ = schedule;
            stoppingTimes_.clear();
            for (Size i=0; i<schedule.size(); i++)
                stoppingTimes_.push_back(process_->time(events_[i]->date()));
        };
        void setupArguments(const OneAssetOption::arguments* args) const {
            FDVanillaEngine::setupArguments(args);
            events_.clear();
            stoppingTimes_ = args->stoppingTimes;
        };

        void calculate(OneAssetOption::results* result) const;
        mutable boost::shared_ptr<StandardStepCondition > stepCondition_;
        mutable boost::shared_ptr<StandardFiniteDifferenceModel> model_;
        virtual void executeIntermediateStep(Size step) const = 0;
        virtual void initializeStepCondition() const;
        virtual void initializeModel() const;
        Time getDividendTime(Size i) const {
            return stoppingTimes_[i];
        }
    };

}


#endif

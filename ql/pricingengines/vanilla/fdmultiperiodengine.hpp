/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file fdmultiperiodengine.hpp
    \brief base engine for options with events happening at specific times
*/

#ifndef quantlib_fd_multi_period_engine_hpp
#define quantlib_fd_multi_period_engine_hpp

#include <ql/pricingengines/vanilla/fdvanillaengine.hpp>
#include <ql/methods/finitedifferences/fdtypedefs.hpp>
#include <ql/instruments/oneassetoption.hpp>
#include <ql/event.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    class FDMultiPeriodEngine : public FDVanillaEngine {
      protected:
        FDMultiPeriodEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size gridPoints = 100, Size timeSteps = 100,
             bool timeDependent = false);
        mutable std::vector<boost::shared_ptr<Event> > events_;
        mutable std::vector<Time> stoppingTimes_;
        Size timeStepPerPeriod_;
        mutable SampledCurve prices_;

        virtual void setupArguments(
               const PricingEngine::arguments* args,
               const std::vector<boost::shared_ptr<Event> >& schedule) const {
            FDVanillaEngine::setupArguments(args);
            events_ = schedule;
            stoppingTimes_.clear();
            Size n = schedule.size();
            stoppingTimes_.reserve(n);
            for (Size i=0; i<n; ++i)
                stoppingTimes_.push_back(process_->time(events_[i]->date()));
        }

        virtual void setupArguments(const PricingEngine::arguments* a) const {
            FDVanillaEngine::setupArguments(a);
            const OneAssetOption::arguments *args =
                dynamic_cast<const OneAssetOption::arguments*>(a);
            QL_REQUIRE(args, "incorrect argument type");
            events_.clear();

            Size n = args->exercise->dates().size();
            stoppingTimes_.resize(n);
            for (Size i=0; i<n; ++i)
                stoppingTimes_[i] =
                      process_->time(args->exercise->date(i));
        }

        virtual void calculate(PricingEngine::results*) const;
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

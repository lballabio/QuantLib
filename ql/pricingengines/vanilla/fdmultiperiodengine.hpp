/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2007, 2009 StatPro Italia srl

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
#include <ql/instruments/oneassetoption.hpp>
#include <ql/methods/finitedifferences/fdtypedefs.hpp>
#include <ql/event.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    template <template <class> class Scheme = CrankNicolson>
    class FDMultiPeriodEngine : public FDVanillaEngine {
      protected:
        typedef FiniteDifferenceModel<Scheme<TridiagonalOperator> > model_type;

        FDMultiPeriodEngine(
             const std::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps = 100, Size gridPoints = 100,
             bool timeDependent = false);
        mutable std::vector<std::shared_ptr<Event> > events_;
        mutable std::vector<Time> stoppingTimes_;
        Size timeStepPerPeriod_;
        mutable SampledCurve prices_;

        virtual void setupArguments(
               const PricingEngine::arguments* args,
               const std::vector<std::shared_ptr<Event> >& schedule) const {
            FDVanillaEngine::setupArguments(args);
            events_ = schedule;
            stoppingTimes_.clear();
            Size n = schedule.size();
            stoppingTimes_.reserve(n);
            for (Size i=0; i<n; ++i)
                stoppingTimes_.push_back(process_->time(events_[i]->date()));
        }

        void setupArguments(const PricingEngine::arguments* a) const override {
            FDVanillaEngine::setupArguments(a);
            const auto* args = dynamic_cast<const OneAssetOption::arguments*>(a);
            QL_REQUIRE(args, "incorrect argument type");
            events_.clear();

            Size n = args->exercise->dates().size();
            stoppingTimes_.resize(n);
            for (Size i=0; i<n; ++i)
                stoppingTimes_[i] =
                      process_->time(args->exercise->date(i));
        }

        virtual void calculate(PricingEngine::results*) const;
        mutable std::shared_ptr<StandardStepCondition > stepCondition_;
        mutable std::shared_ptr<model_type> model_;
        virtual void executeIntermediateStep(Size step) const = 0;
        virtual void initializeStepCondition() const;
        virtual void initializeModel() const;
        Time getDividendTime(Size i) const {
            return stoppingTimes_[i];
        }
    };


    // template definitions

    template <template <class> class Scheme>
    FDMultiPeriodEngine<Scheme>::FDMultiPeriodEngine(
             const std::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps, Size gridPoints, bool timeDependent)
    : FDVanillaEngine(process, timeSteps, gridPoints, timeDependent),
      timeStepPerPeriod_(timeSteps) {}

    template <template <class> class Scheme>
    void FDMultiPeriodEngine<Scheme>::calculate(
                                            PricingEngine::results* r) const {
        auto* results = dynamic_cast<OneAssetOption::results*>(r);
        QL_REQUIRE(results, "incorrect argument type");
        Time beginDate, endDate;
        Size dateNumber = stoppingTimes_.size();
        bool lastDateIsResTime = false;
        Integer firstIndex = -1;
        Integer lastIndex = static_cast<Integer>(dateNumber) - 1;
        bool firstDateIsZero = false;
        Time firstNonZeroDate = getResidualTime();

        Real dateTolerance = 1e-6;

        if (dateNumber > 0) {
            QL_REQUIRE(getDividendTime(0) >= 0,
                       "first date (" << getDividendTime(0)
                       << ") cannot be negative");
            if(getDividendTime(0) < getResidualTime() * dateTolerance ){
                firstDateIsZero = true;
                firstIndex = 0;
                if(dateNumber >= 2)
                    firstNonZeroDate = getDividendTime(1);
            }

            if (std::fabs(getDividendTime(lastIndex) - getResidualTime())
                < dateTolerance) {
                lastDateIsResTime = true;
                lastIndex = Integer(dateNumber) - 2;
            }

            if (!firstDateIsZero)
                firstNonZeroDate = getDividendTime(0);

            if (dateNumber >= 2) {
                for (Size j = 1; j < dateNumber; j++)
                    QL_REQUIRE(getDividendTime(j-1) < getDividendTime(j),
                               "dates must be in increasing order: "
                               << getDividendTime(j-1)
                               << " is not strictly smaller than "
                               << getDividendTime(j));
            }
        }

        Time dt = getResidualTime()/(timeStepPerPeriod_*(dateNumber+1));

        // Ensure that dt is always smaller than the first non-zero date
        if (firstNonZeroDate <= dt)
            dt = firstNonZeroDate/2.0;

        setGridLimits();
        initializeInitialCondition();
        initializeOperator();
        initializeBoundaryConditions();
        initializeModel();
        initializeStepCondition();

        prices_ = intrinsicValues_;
        if(lastDateIsResTime)
            executeIntermediateStep(dateNumber - 1);

        Integer j = lastIndex;
        do {
            if (j == Integer(dateNumber) - 1)
                beginDate = getResidualTime();
            else
                beginDate = getDividendTime(j+1);

            if (j >= 0)
                endDate = getDividendTime(j);
            else
                endDate = dt;

            model_->rollback(prices_.values(),
                             beginDate, endDate,
                             timeStepPerPeriod_, *stepCondition_);
            if (j >= 0)
                executeIntermediateStep(j);
        } while (--j >= firstIndex);

        model_->rollback(prices_.values(), dt, 0, 1, *stepCondition_);

        if(firstDateIsZero)
            executeIntermediateStep(0);

        results->value = prices_.valueAtCenter();
        results->delta = prices_.firstDerivativeAtCenter();
        results->gamma = prices_.secondDerivativeAtCenter();
        results->additionalResults["priceCurve"] = prices_;
    }

    template <template <class> class Scheme>
    void FDMultiPeriodEngine<Scheme>::initializeStepCondition() const{
        stepCondition_ = std::shared_ptr<StandardStepCondition>(
                                                  new NullCondition<Array>());
    }

    template <template <class> class Scheme>
    void FDMultiPeriodEngine<Scheme>::initializeModel() const{
        model_ = std::shared_ptr<model_type>(
                              new model_type(finiteDifferenceOperator_,BCs_));
    }

}


#endif

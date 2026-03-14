/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Grzegorz Andruszkiewicz

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

#include <ql/experimental/catbonds/catrisk.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <random>
#include <utility>

namespace QuantLib {


    EventSetSimulation::EventSetSimulation(
        ext::shared_ptr<std::vector<std::pair<Date, Real> > > events,
        Date eventsStart,
        Date eventsEnd,
        Date start,
        Date end)
    : CatSimulation(start, end), events_(std::move(events)), eventsStart_(eventsStart),
      eventsEnd_(eventsEnd) {
        years_ = end_.year()-start_.year();
        if(eventsStart_.month()<start_.month() 
                            || (eventsStart_.month()==start_.month() 
                                && eventsStart_.dayOfMonth()<=start_.dayOfMonth())) {
            periodStart_ = Date(start_.dayOfMonth(), start_.month(), eventsStart_.year());
        } else {
            periodStart_ = Date(start_.dayOfMonth(), start_.month(), eventsStart_.year()+1);
        }
        periodEnd_ = Date(end_.dayOfMonth(), end_.month(), periodStart_.year()+years_);
        while(i_<events_->size() && (*events_)[i_].first<periodStart_) ++i_; //i points to the first element after the start of the relevant period.
    }

    bool EventSetSimulation::nextPath(std::vector< std::pair< Date, Real > >& path) {
        path.resize(0);
        if(periodEnd_>eventsEnd_) //Ran out of event data 
            return false;

        while(i_<events_->size() && (*events_)[i_].first<periodStart_) {
            ++i_; //skip the elements between the previous period and this period
        }
        while(i_<events_->size()  && (*events_)[i_].first<=periodEnd_){
            std::pair<Date, Real> e(events_->at(i_).first+(start_.year() - periodStart_.year())*Years, events_->at(i_).second);
            path.push_back(e);
            ++i_; //i points to the first element after the start of the relevant period.
        }
        if(start_+years_*Years<end_) {
            periodStart_+=(years_+1)*Years;
            periodEnd_+=(years_+1)*Years;
        } else {
            periodStart_+=years_*Years;
            periodEnd_+=years_*Years;
        }
        return true;
    }

    EventSet::EventSet(ext::shared_ptr<std::vector<std::pair<Date, Real> > > events,
                       Date eventsStart,
                       Date eventsEnd)
    : events_(std::move(events)), eventsStart_(eventsStart), eventsEnd_(eventsEnd) {}

    ext::shared_ptr<CatSimulation> EventSet::newSimulation(const Date& start, const Date& end) const{
        return ext::make_shared<EventSetSimulation>(events_, eventsStart_, eventsEnd_, start, end);
    }

    BetaRiskSimulation::BetaRiskSimulation(Date start, Date end, Real maxLoss, Real lambda, Real alpha, Real beta) 
              : CatSimulation(start, end), 
                maxLoss_(maxLoss), 
                exponential_(lambda),
                gammaAlpha_(alpha),
                gammaBeta_(beta)
    {
        DayCounter dayCounter = ActualActual(ActualActual::ISDA);
        dayCount_ = dayCounter.dayCount(start, end);
        yearFraction_ = dayCounter.yearFraction(start, end);
    }

    Real BetaRiskSimulation::generateBeta()
    {
        Real X = gammaAlpha_(rng_);
        Real Y = gammaBeta_(rng_);
        return X*maxLoss_/(X+Y);
    }

    bool BetaRiskSimulation::nextPath(std::vector<std::pair<Date, Real> > &path)
    {        
        path.resize(0);
        Real eventFraction = exponential_(rng_);
        while(eventFraction<=yearFraction_)
        {
            auto days =
                static_cast<Integer>(std::lround(eventFraction * dayCount_ / yearFraction_));
            Date eventDate = start_ + days*Days;
            if(eventDate<=end_)
            {
                path.emplace_back(eventDate, generateBeta());
            }
            else break;
            eventFraction = exponential_(rng_);
        }
        return true;
    }

    BetaRisk::BetaRisk(Real maxLoss, 
                 Real years, 
                 Real mean, 
                 Real stdDev) 
    : maxLoss_(maxLoss), lambda_(1.0/years) {
        QL_REQUIRE(mean<maxLoss, "Mean "<<mean<<"of the loss distribution must be less than the maximum loss "<<maxLoss);
        Real normalizedMean = mean/maxLoss;
        Real normalizedVar = stdDev*stdDev/(maxLoss*maxLoss);
        QL_REQUIRE(normalizedVar<normalizedMean*(1.0-normalizedMean), "Standard deviation of "<<stdDev<<" is impossible to achieve in gamma distribution with mean "<<mean);
        Real nu = normalizedMean*(1.0-normalizedMean)/normalizedVar - 1.0;
        alpha_=normalizedMean*nu;
        beta_=(1.0-normalizedMean)*nu;
    }

    ext::shared_ptr<CatSimulation> BetaRisk::newSimulation(const Date& start, const Date& end) const {
        return ext::make_shared<BetaRiskSimulation>(start, end, maxLoss_, lambda_, alpha_, beta_);
    }
}

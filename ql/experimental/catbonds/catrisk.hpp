/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Grzegorz Andruszkiewicz

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

/*! \file catrisk.hpp
    \brief classes that encapsulate catastrophe risk
*/

#ifndef quantlib_catrisk_hpp
#define quantlib_catrisk_hpp

#include <ql/time/date.hpp>
#include <ql/errors.hpp>
#include <ql/shared_ptr.hpp>
#include <random>
#include <vector>

namespace QuantLib {

    class CatSimulation {
      public:
        CatSimulation(Date start, 
                      Date end) 
        : start_(start), end_(end) 
        {}

        virtual ~CatSimulation() = default;
        virtual bool nextPath(std::vector<std::pair<Date, Real> > &path) = 0;
      protected:
        Date start_;
        Date end_;
    };

    class CatRisk {
      public:
        virtual ~CatRisk() = default;
        virtual ext::shared_ptr<CatSimulation> newSimulation(const Date& start, const Date& end) const = 0;
    };

    class EventSetSimulation : public CatSimulation {
      public:
        EventSetSimulation(ext::shared_ptr<std::vector<std::pair<Date, Real> > > events,
                           Date eventsStart,
                           Date eventsEnd,
                           Date start,
                           Date end);
        bool nextPath(std::vector<std::pair<Date, Real> >& path) override;

      private:
        ext::shared_ptr<std::vector<std::pair<Date, Real> > > events_;
        Date eventsStart_;
        Date eventsEnd_;

        Year years_;
        Date periodStart_;
        Date periodEnd_;
        unsigned int i_;
    };

    class EventSet : public CatRisk {        
      public:
        EventSet(ext::shared_ptr<std::vector<std::pair<Date, Real> > > events,
                 Date eventsStart,
                 Date eventsEnd);

        ext::shared_ptr<CatSimulation> newSimulation(const Date& start,
                                                     const Date& end) const override;

      private:
        ext::shared_ptr<std::vector<std::pair<Date, Real> > > events_; 
        Date eventsStart_;
        Date eventsEnd_;
    };

    class BetaRiskSimulation : public CatSimulation {
      public:
        BetaRiskSimulation(Date start, 
                           Date end, 
                           Real maxLoss, 
                           Real lambda, 
                           Real alpha, 
                           Real beta) ;

        bool nextPath(std::vector<std::pair<Date, Real> >& path) override;
        Real generateBeta();
    
      private:
        Real maxLoss_;
    
        Integer dayCount_;
        Real yearFraction_;
    
        std::mt19937 rng_;
        std::exponential_distribution<Real> exponential_;
        std::gamma_distribution<Real> gammaAlpha_;
        std::gamma_distribution<Real> gammaBeta_;
    };

    class BetaRisk : public CatRisk {
      public:
        BetaRisk(Real maxLoss, 
                 Real years, 
                 Real mean, 
                 Real stdDev);

        ext::shared_ptr<CatSimulation> newSimulation(const Date& start,
                                                     const Date& end) const override;

      private:
        Real maxLoss_;
        Real lambda_;
        Real alpha_;
        Real beta_;
    };

}

#endif

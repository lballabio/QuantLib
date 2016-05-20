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
#include <boost/shared_ptr.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/random.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif
#include <vector>

namespace QuantLib {

    class CatSimulation {
      public:
        CatSimulation(Date start, 
                      Date end) 
        : start_(start), end_(end) 
        {}

        virtual ~CatSimulation(){}
        virtual bool nextPath(std::vector<std::pair<Date, Real> > &path) = 0;
      protected:
        Date start_;
        Date end_;
    };

    class CatRisk {
      public:
        virtual ~CatRisk() {}
        virtual boost::shared_ptr<CatSimulation> newSimulation(const Date& start, const Date& end) const = 0;
    };

    class EventSetSimulation : public CatSimulation {
      public:
        EventSetSimulation(boost::shared_ptr<std::vector<std::pair<Date, Real> > > events, Date eventsStart, Date eventsEnd, Date start, Date end);
        virtual bool nextPath(std::vector<std::pair<Date, Real> > &path);
      
      private:
        boost::shared_ptr<std::vector<std::pair<Date, Real> > > events_;
        Date eventsStart_;
        Date eventsEnd_;

        Year years_;
        Date periodStart_;
        Date periodEnd_;
        unsigned int i_;
    };

    class EventSet : public CatRisk {        
      public:
        EventSet(boost::shared_ptr<std::vector<std::pair<Date, Real> > > events, 
                 Date eventsStart, 
                 Date eventsEnd);

        boost::shared_ptr<CatSimulation> newSimulation(const Date& start, const Date& end) const;
      private:
        boost::shared_ptr<std::vector<std::pair<Date, Real> > > events_; 
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

        virtual bool nextPath(std::vector<std::pair<Date, Real> > &path);
        Real generateBeta();
    
      private:
        Real maxLoss_;
    
        Integer dayCount_;
        Real yearFraction_;
    
        boost::mt19937 rng_;
        boost::variate_generator<boost::mt19937&, boost::exponential_distribution<> > exponential_;
        boost::variate_generator<boost::mt19937&, boost::gamma_distribution<> > gammaAlpha_;
        boost::variate_generator<boost::mt19937&, boost::gamma_distribution<> > gammaBeta_;
    };

    class BetaRisk : public CatRisk {
      public:
        BetaRisk(Real maxLoss, 
                 Real years, 
                 Real mean, 
                 Real stdDev);

        virtual boost::shared_ptr<CatSimulation> newSimulation(const Date& start, const Date& end) const;

      private:
        Real maxLoss_;
        Real lambda_;
        Real alpha_;
        Real beta_;
    };

}

#endif

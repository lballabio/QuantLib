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

/*! \file riskynotional.hpp
    \brief classes to track the notional of a cat bond
*/

#ifndef quantlib_risky_notional_hpp
#define quantlib_risky_notional_hpp

#include <ql/time/date.hpp>
#include <ql/errors.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <algorithm>

namespace QuantLib {

    class EventPaymentOffset {
      public:
        virtual ~EventPaymentOffset() {}
        virtual Date paymentDate(const Date& eventDate) = 0;
    };

    class NoOffset : public EventPaymentOffset {
      public:
        virtual Date paymentDate(const Date& eventDate) { return eventDate; }
    };

    class NotionalPath {
      public:
        NotionalPath();

        Rate notionalRate(const Date& date) const; //The fraction of the original notional left on a given date

        void reset();

        void addReduction(const Date &date, Rate newRate);

        Real loss();

      private:
        std::vector<std::pair<Date, Real> > notionalRate_;
    };

    class NotionalRisk {
    public:
        NotionalRisk(boost::shared_ptr<EventPaymentOffset> paymentOffset)
        : paymentOffset_(paymentOffset) {}
        virtual ~NotionalRisk() {}

        virtual void updatePath(const std::vector<std::pair<Date, Real> >  &events, NotionalPath &path) const = 0;

      protected:
        boost::shared_ptr<EventPaymentOffset> paymentOffset_;       
    };

    class DigitalNotionalRisk : public NotionalRisk {
      public:
        DigitalNotionalRisk(boost::shared_ptr<EventPaymentOffset> paymentOffset,
                           Real threshold)
        : NotionalRisk(paymentOffset) , threshold_(threshold)
        {}

        virtual void updatePath(const std::vector<std::pair<Date, Real> >  &events, 
                                NotionalPath &path) const;
      protected:
        Real threshold_;
    };


    class ProportionalNotionalRisk : public NotionalRisk
    {
    public:
        ProportionalNotionalRisk(boost::shared_ptr<EventPaymentOffset> paymentOffset,
                           Real attachement, Real exhaustion)
                           : NotionalRisk(paymentOffset) , attachement_(attachement), exhaustion_(exhaustion)
        {
            QL_REQUIRE(attachement<exhaustion, "exhaustion level needs to be greater than attachement");
        }

        virtual void updatePath(const std::vector<std::pair<Date, Real> >  &events, NotionalPath &path) const
        {
            path.reset();
            Real losses = 0;
            Real previousNotional = 1;
            for(size_t i=0; i<events.size(); ++i)
            {
                losses+=events[i].second;
                if(losses>attachement_ && previousNotional>0)
                {
                    previousNotional = std::max(0.0, (exhaustion_-losses)/(exhaustion_-attachement_));
                    path.addReduction(paymentOffset_->paymentDate(events[i].first), previousNotional);
                }
            }
        }
    protected:
        Real attachement_;
        Real exhaustion_;
    };

}

#endif

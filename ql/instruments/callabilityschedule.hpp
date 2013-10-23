/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2005, 2006 Theo Boafo

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

/*! \file callabilityschedule.hpp
    \brief Schedule of put/call dates
*/

#ifndef quantlib_callability_schedule_hpp
#define quantlib_callability_schedule_hpp

#include <ql/event.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/utilities/null.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <vector>

namespace QuantLib {

    //! %instrument callability
    class Callability : public Event {
      public:
        //! amount to be paid upon callability
        class Price {
          public:
            enum Type { Dirty, Clean };
            Price() : amount_(Null<Real>()) {}
            Price(Real amount, Type type) : amount_(amount), type_(type) {}
            Real amount() const {
                QL_REQUIRE(amount_ != Null<Real>(), "no amount given");
                return amount_;
            }
            Type type() const { return type_; }
          private:
            Real amount_;
            Type type_;
        };
        //! type of the callability
        enum Type { Call, Put };

        Callability(const Price& price, Type type, const Date& date)
        : price_(price), type_(type), date_(date) {}
        const Price& price() const {
            QL_REQUIRE(price_, "no price given");
            return *price_;
        }
        Type type() const { return type_; }
        //! \name Event interface
        //@{
        Date date() const { return date_; }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::optional<Price> price_;
        Type type_;
        Date date_;
    };

    inline void Callability::accept(AcyclicVisitor& v){
        Visitor<Callability>* v1 = dynamic_cast<Visitor<Callability>*>(&v);
        if(v1 != 0)
            v1->visit(*this);
        else
            Event::accept(v);
    }

    typedef std::vector<boost::shared_ptr<Callability> > CallabilitySchedule;

}

#endif

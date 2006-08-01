/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Nicolas Di Césaré

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

/*! \file indexedcoupon.hpp
    \brief indexed coupon
*/

#ifndef quantlib_indexed_coupon_hpp
#define quantlib_indexed_coupon_hpp

#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/Indexes/xibor.hpp>

namespace QuantLib {

    //! Base indexed coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class IndexedCoupon : public FloatingRateCoupon,
                          public Observer {
      public:
        IndexedCoupon(const Date& paymentDate,
                      const Real nominal,
                      const Date& startDate,
                      const Date& endDate,
                      const Integer fixingDays,
                      const boost::shared_ptr<Index>& index,
                      const Real gearing = 1.0,
                      const Spread spread = 0.0,
                      const Date& refPeriodStart = Date(),
                      const Date& refPeriodEnd = Date(),
                      const DayCounter& dayCounter = DayCounter());
        virtual ~IndexedCoupon() {}
        //! \name CashFlow interface
        //@{
        Real amount() const { return rate() * accrualPeriod() * nominal(); }
        //@}
        //! \name Coupon interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        Rate indexFixing() const { return index_->fixing(fixingDate()); }
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<Index>& index() const { return index_; }
        //@}
        //! \name Observer interface
        //@{
        void update() { notifyObservers(); }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::shared_ptr<Index> index_;
        DayCounter dayCounter_;
    };

    // inline definitions

    inline IndexedCoupon::IndexedCoupon(const Date& paymentDate,
                                        Real nominal,
                                        const Date& startDate,
                                        const Date& endDate,
                                        Integer fixingDays,
                                        const boost::shared_ptr<Index>& index,
                                        const Real gearing,
                                        Spread spread,
                                        const Date& refPeriodStart,
                                        const Date& refPeriodEnd,
                                        const DayCounter& dayCounter)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, gearing, spread,
                         refPeriodStart, refPeriodEnd),
      index_(index), dayCounter_(dayCounter)
    {
        if (dayCounter_.empty()) {
            boost::shared_ptr<Xibor> xibor =
                boost::dynamic_pointer_cast<Xibor>(index);
            if (xibor)
                dayCounter_ = xibor->dayCounter();
            else
                QL_FAIL("day counter not specified and not retrievable "
                        "from index");
        }
        registerWith(index_);
    }

    inline void IndexedCoupon::accept(AcyclicVisitor& v) {
        Visitor<IndexedCoupon>* v1 =
            dynamic_cast<Visitor<IndexedCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

}

#endif

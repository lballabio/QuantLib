
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file parcoupon.hpp
    \brief Coupon at par on a term structure
*/

#ifndef quantlib_par_coupon_hpp
#define quantlib_par_coupon_hpp

#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/Indexes/xibor.hpp>

namespace QuantLib {

    //! %coupon at par on a term structure
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class ParCoupon : public FloatingRateCoupon,
                      public Observer {
      public:
        ParCoupon(Real nominal,
                  const Date& paymentDate,
                  const boost::shared_ptr<Xibor>& index,
                  const Date& startDate, const Date& endDate,
                  Integer fixingDays,
                  Spread spread = 0.0,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter());
        //! \name CashFlow interface
        //@{
        Real amount() const;
        //@}
        //! \name Coupon interface
        //@{
        DayCounter dayCounter() const;
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        Rate indexFixing() const;
        Date fixingDate() const;
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<Xibor>& index() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::shared_ptr<Xibor> index_;
        DayCounter dayCounter_;
    };


    // inline definitions

    inline DayCounter ParCoupon::dayCounter() const {
        return dayCounter_.isNull() ? index_->dayCounter() : dayCounter_;
    }

    inline Date ParCoupon::fixingDate() const {
        return index_->calendar().advance(accrualStartDate_,
                                          -fixingDays_, Days,
                                          Preceding);
    }

    inline const boost::shared_ptr<Xibor>& ParCoupon::index() const {
        return index_;
    }

    inline void ParCoupon::update() {
        notifyObservers();
    }

    inline void ParCoupon::accept(AcyclicVisitor& v) {
        Visitor<ParCoupon>* v1 = dynamic_cast<Visitor<ParCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

}


#endif

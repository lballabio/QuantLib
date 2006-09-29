/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 StatPro Italia srl
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

/*! \file floatingratecoupon.hpp
    \brief Coupon paying a variable index-based rate
*/

#ifndef quantlib_floating_rate_coupon_hpp
#define quantlib_floating_rate_coupon_hpp

#include <ql/CashFlows/coupon.hpp>
#include <ql/Utilities/null.hpp>
#include <ql/Indexes/interestrateindex.hpp>

namespace QuantLib {

    //! %Coupon paying a variable index-based rate
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
        \todo add gearing unit test
    */
    class FloatingRateCoupon : public Coupon,
                               public Observer {
      public:
        FloatingRateCoupon(const Date& paymentDate,
                           const Real nominal,
                           const Date& startDate,
                           const Date& endDate,
                           const Integer fixingDays,
                           const boost::shared_ptr<InterestRateIndex>& index,
                           const Real gearing = 1.0,
                           const Spread spread = 0.0,
                           const Date& refPeriodStart = Date(),
                           const Date& refPeriodEnd = Date(),
                           const DayCounter& dayCounter = DayCounter());
        virtual ~FloatingRateCoupon() {}
        //! \name Coupon interface
        //@{
        virtual Rate rate() const;
        Real amount() const;
        Real accruedAmount(const Date&) const;
        DayCounter dayCounter() const;
        //@}
        //! \name Inspectors
        //@{
        //! floating index
        const boost::shared_ptr<InterestRateIndex>& index() const;
        //! fixing days
        Integer fixingDays() const;
        //! fixing date
        virtual Date fixingDate() const;
        //! index gearing, i.e. multiplicative coefficient for the index
        Real gearing() const;
        //! fixing of the underlying index
        Rate indexFixing() const;
        //! convexity adjustment
        Rate convexityAdjustment() const;
        //! convexity-adjusted fixing
        Rate adjustedFixing() const;
        //! spread paid over the fixing of the underlying index
        Spread spread() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        //! convexity adjustment for the given index fixing
        virtual Rate convexityAdjustmentImpl(Rate fixing) const;
        boost::shared_ptr<InterestRateIndex> index_;
        DayCounter dayCounter_;
        Integer fixingDays_;
        Real gearing_;
        Spread spread_;
    };


    // inline definitions

    inline FloatingRateCoupon::FloatingRateCoupon(
                         const Date& paymentDate, const Real nominal,
                         const Date& startDate, const Date& endDate,
                         const Integer fixingDays,
                         const boost::shared_ptr<InterestRateIndex>& index,
                         const Real gearing, const Spread spread,
                         const Date& refPeriodStart, const Date& refPeriodEnd,
                         const DayCounter& dayCounter)
    : Coupon(nominal, paymentDate,
             startDate, endDate, refPeriodStart, refPeriodEnd),
      index_(index), dayCounter_(dayCounter), fixingDays_(fixingDays),
      gearing_(gearing), spread_(spread)
    {
        if (dayCounter_.empty())
            dayCounter_ = index_->dayCounter();
        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());
    }

    inline Rate FloatingRateCoupon::rate() const {
        return gearing() * adjustedFixing() + spread();
    }

    inline Real FloatingRateCoupon::amount() const {
        return rate() * accrualPeriod() * nominal();
    }

    inline Real FloatingRateCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal() * rate() *
                dayCounter().yearFraction(accrualStartDate_,
                                          std::min(d,accrualEndDate_),
                                          refPeriodStart_,
                                          refPeriodEnd_);
        }
    }

    inline DayCounter FloatingRateCoupon::dayCounter() const {
        return dayCounter_;
   }

    inline const boost::shared_ptr<InterestRateIndex>&
    FloatingRateCoupon::index() const {
        return index_;
    }

    inline Integer FloatingRateCoupon::fixingDays() const {
        return fixingDays_;
    }

    inline Date FloatingRateCoupon::fixingDate() const {
        return index_->calendar().advance(accrualStartDate_,
                                          -fixingDays(), Days,
                                          Preceding);
    }



    inline Real FloatingRateCoupon::gearing() const {
        return gearing_;
    }

    inline Rate FloatingRateCoupon::indexFixing() const {
        return index_->fixing(fixingDate());
    }

    inline Rate FloatingRateCoupon::convexityAdjustment() const {
        return convexityAdjustmentImpl(indexFixing());
    }

    inline Rate FloatingRateCoupon::adjustedFixing() const {
        Rate f = indexFixing();
        return f + convexityAdjustmentImpl(f);
    }

    inline Spread FloatingRateCoupon::spread() const {
        return spread_;
    }

    inline void FloatingRateCoupon::update() {
        notifyObservers();
    }

    inline Rate
    FloatingRateCoupon::convexityAdjustmentImpl(Rate) const {
        return 0.0;
    }

    inline void FloatingRateCoupon::accept(AcyclicVisitor& v) {
        Visitor<FloatingRateCoupon>* v1 =
            dynamic_cast<Visitor<FloatingRateCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Coupon::accept(v);
    }

}

#endif

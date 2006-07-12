/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file cmscoupon.hpp
    \brief CMS coupon
*/

#ifndef quantlib_cms_coupon_hpp
#define quantlib_cms_coupon_hpp

#include <ql/Indexes/swaprate.hpp>
#include <ql/CashFlows/all.hpp>
#include <ql/swaptionvolstructure.hpp>

namespace QuantLib {

    //! CMS coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class CMSCoupon : public FloatingRateCoupon,
                      public Observer {
      public:
        CMSCoupon(Real nominal,
                  const Date& paymentDate,
                  const boost::shared_ptr<SwapRate>& index,
                  const Date& startDate, const Date& endDate,
                  Integer fixingDays,
                  const DayCounter& dayCounter,
                  Rate baseRate = 0.0,
                  Real multiplier = 1.0,
                  Rate cap = Null<Rate>(),
                  Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date());
        //! \name CashFlow interface
        //@{
        Real amount() const;
        //@}
        //! \name Coupon interface
        //@{
        Rate rate() const;
        DayCounter dayCounter() const { return dayCounter_; }
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        Rate fixing() const;
        Date fixingDate() const;
        Rate indexFixing() const;
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<SwapRate>& index() const { return index_; }
        Rate baseRate() const { return baseRate_; }
        Rate cap() const { return cap_; }
        Rate floor() const { return floor_; }
        Real multiplier() const { return multiplier_; }
        //@}
        //! \name Modifiers
        //@{
        void setSwaptionVolatility(
                const Handle<SwaptionVolatilityStructure>&);
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
        boost::shared_ptr<SwapRate> index_;
        DayCounter dayCounter_;
        Rate baseRate_, cap_, floor_;
        Real multiplier_;
        Handle<SwaptionVolatilityStructure> swaptionVol_;
    };


    std::vector<boost::shared_ptr<CashFlow> >
    CMSCouponVector(const Schedule& schedule,
                    BusinessDayConvention paymentAdjustment,
                    const std::vector<Real>& nominals,
                    const boost::shared_ptr<SwapRate>& index,
                    Integer fixingDays,
                    const DayCounter& dayCounter,
                    const std::vector<Rate>& baseRate,
                    const std::vector<Real>& fractions,
                    const std::vector<Rate>& caps,
                    const std::vector<Rate>& floors);

}


#endif

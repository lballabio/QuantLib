/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
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

#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/Indexes/swapindex.hpp>
#include <ql/swaptionvolstructure.hpp>

namespace QuantLib {

    class CMSCoupon;

    //! pricer for vanilla CMS coupons
    class VanillaCMSCouponPricer {
      public:
		VanillaCMSCouponPricer(const Handle<SwaptionVolatilityStructure>& swaptionVol)
		: swaptionVol_(swaptionVol) {};
        virtual ~VanillaCMSCouponPricer() {};
        virtual Real price() const = 0;
        virtual Rate rate() const = 0;
        virtual void initialize(const CMSCoupon& coupon) = 0;
		Handle<SwaptionVolatilityStructure> swaptionVolatility() const{
			return swaptionVol_;
	    };
		void setSwaptionVolatility(const Handle<SwaptionVolatilityStructure>& swaptionVol){
			swaptionVol_ = swaptionVol;
		};
      private:
		Handle<SwaptionVolatilityStructure> swaptionVol_;
    };

    //! CMS coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class CMSCoupon : public FloatingRateCoupon {
      public:
        CMSCoupon(const Date& paymentDate,
                  const Real nominal,
                  const Date& startDate, 
                  const Date& endDate,
                  const Integer fixingDays,
                  const boost::shared_ptr<SwapIndex>& index,
                  const Real gearing = 1.0,
                  const Spread spread= 0.0,
                  const Rate cap = Null<Rate>(),
                  const Rate floor = Null<Rate>(),
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false);

        //! \name Coupon interface
        //@{
        Real price(const Handle<YieldTermStructure>& discountingCurve) const;
        Rate rate() const;
        // legacy code (analytical integration) to be removed later
        Rate rate1() const;
        //@}
        //! \name Inspectors
        //@{
        const boost::shared_ptr<SwapIndex>& swapIndex() const {
            return swapIndex_;
        }
        Rate cap() const { return cap_; }
        Rate floor() const { return floor_; }

        void setPricer(const boost::shared_ptr<VanillaCMSCouponPricer>& pricer){
			pricer_ = pricer;
		};
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:

        Rate adjustedFixing() const{
            CMSCoupon couponWithoutOptionality(date(),
                                              nominal(),
                                              accrualStartDate(), 
                                              accrualEndDate(),
                                              fixingDays(),
                                              swapIndex(),
                                              gearing(),
                                              spread(),
                                              100.,
                                              0.,
                                              referencePeriodStart(), 
                                              referencePeriodEnd(),
                                              dayCounter(),
                                              isInArrears_);
            couponWithoutOptionality.setPricer(pricer_);
            return (gearing() == 0.0 ? 0.0 :
                couponWithoutOptionality.rate()-spread())/gearing();
        }
        Rate convexityAdjustmentImpl(Rate f) const {
            return (gearing() == 0.0 ? 0.0 : adjustedFixing()-f);
        }
        boost::shared_ptr<SwapIndex> swapIndex_;
        Rate cap_, floor_;
        boost::shared_ptr<VanillaCMSCouponPricer> pricer_;
    };

}

#endif

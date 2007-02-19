/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco

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

/*! \file couponpricer.hpp
    \brief Coupon Pricers
*/

#ifndef quantlib_coupon_pricer_hpp
#define quantlib_coupon_pricer_hpp

#include <ql/CashFlows/core.hpp>
#include <ql/option.hpp>
#include <ql/capvolstructures.hpp>
#include <ql/swaptionvolstructure.hpp>

namespace QuantLib {


    class FloatingRateCoupon;
    //
    //! generic pricer for FloatingRate coupons
    //
    class FloatingRateCouponPricer: public virtual Observer,
                                    public virtual Observable{
      public:
        virtual ~FloatingRateCouponPricer() {}
        /* */
        virtual Real swapletPrice() const = 0;
        virtual Rate swapletRate() const = 0;
        virtual Real capletPrice(Rate effectiveCap) const = 0;
        virtual Rate capletRate(Rate effectiveCap) const = 0;
        virtual Real floorletPrice(Rate effectiveFloor) const = 0;
        virtual Rate floorletRate(Rate effectiveFloor) const = 0;
        virtual void initialize(const FloatingRateCoupon& coupon)= 0;  
        //! \name Observer interface
        //@{
        void update(){notifyObservers();}
        //@}
    };

    
    class IborCoupon;
    //! pricer for cappedFlooredIbor coupons
    class IborCouponPricer: public FloatingRateCouponPricer{
      public:
        IborCouponPricer(const Handle<CapletVolatilityStructure>& capletVol)
         : capletVol_(capletVol) { registerWith(capletVol_);};
        virtual ~IborCouponPricer() {}
		
        Handle<CapletVolatilityStructure> capletVolatility() const{
	        return capletVol_;
	    }
		void setCapletVolatility(const Handle<CapletVolatilityStructure>& capletVol){
            unregisterWith(capletVol_);
            capletVol_ = capletVol;
            QL_REQUIRE(!capletVol_.empty(), "no adequate capletVol given");
            registerWith(capletVol_);
            update();
		}
    private:
        Handle<CapletVolatilityStructure> capletVol_;
    };

    //! By Black formula
    class BlackIborCouponPricer: public IborCouponPricer{
      public:
		BlackIborCouponPricer(const Handle<CapletVolatilityStructure>& capletVol= 
                                        Handle<CapletVolatilityStructure>())
		: IborCouponPricer(capletVol) {};
        virtual ~BlackIborCouponPricer() {};
        virtual void initialize(const FloatingRateCoupon& coupon);
        /* */
        virtual Real swapletPrice() const;
        virtual Rate swapletRate() const;
        virtual Real capletPrice(Rate effectiveCap) const;
        virtual Rate capletRate(Rate effectiveCap) const;
        virtual Real floorletPrice(Rate effectiveFloor) const;
        virtual Rate floorletRate(Rate effectiveFloor) const;
      
      protected:
        /* */
        Real optionletPrice(Option::Type optionType, Real effStrike) const;

      private:
        Rate adjustedFixing() const;

        const IborCoupon* coupon_;
        Real discount_;
        Real gearing_;
        Spread spread_;
        Real spreadLegValue_;
    };   

    
    class CmsCoupon;
    //! pricer for vanilla Cms coupons
    class CmsCouponPricer: public FloatingRateCouponPricer {
      public:
		CmsCouponPricer(const Handle<SwaptionVolatilityStructure>& swaptionVol)
            : swaptionVol_(swaptionVol) {registerWith(swaptionVol_);}

		Handle<SwaptionVolatilityStructure> swaptionVolatility() const{
			return swaptionVol_;
	     }
		void setSwaptionVolatility(const Handle<SwaptionVolatilityStructure>& swaptionVol){
            unregisterWith(swaptionVol_);
            swaptionVol_ = swaptionVol;
            QL_REQUIRE(!swaptionVol_.empty(), "no adequate swaptionVol given");
            registerWith(swaptionVol_);
            update();
		}
      private:
        Handle<SwaptionVolatilityStructure> swaptionVol_;

    };
    
    /* In addition, we have in conundrumpricer.hpp:
    class ConundrumPricer
    class ConundrumPricer::ConundrumPricerByNumericalIntegration
    class ConundrumPricer::ConundrumPricerByBlack
    */

}

#endif

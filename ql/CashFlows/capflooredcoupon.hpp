/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2006 Cristina Duminuco

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

/*! \file capflooredcoupon.hpp
    \brief Floating rate coupon with additional cap/floor
*/

#ifndef quantlib_capped_floored_coupon_hpp
#define quantlib_capped_floored_coupon_hpp

#include <ql/CashFlows/capfloorlet.hpp>
#include <ql/CashFlows/upfrontindexedcoupon.hpp>
#include <ql/CashFlows/indexedcashflowvectors.hpp>


namespace QuantLib {

    /*! \file capflooredcoupon.hpp
        \brief Capped or/and floored floating rate coupon

        The payoff of a capped floating rate coupon is:
        \f[ Payoff = Nominal accrual min(a L + b, C). \f]
        The payoff of a floored floating rate coupon is:
        \f[ Payoff = Nominal accrual max(a L + b, F). \f]
        The payoff of a collared floating rate coupon is:
        \f[ Payoff = Nominal accrual min(max(a L + b, F), C). \f]

        where \f$ L \f$ is the floating rate, \f$ a \f$ is its 
        gearing, \f$ b \f$ is the spread and \f$ C \f$ and \f$ F \f$
        the strikes.

        They can be decomposed in the following manner.
        Decomposition of a capped floating rate coupon:
        \f[
        Payoff = min(a L + b, C) = //
               = (a L + b) + min(C - b - \csi |a| L, 0)
        \f] 
        where \f$ csi = sgn(a) \f$. Then:
        Payoff = (a L + b) + |a| min(\frac{C - b}{|a|} - \csi L, 0) //
        \f]

        \warning The evaluation is made using the Black model.

    */


    class CappedFlooredCoupon : public FloatingRateCoupon {
      public:
        CappedFlooredCoupon(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate cap = Null<Rate>(),
                  Rate floor = Null<Rate>());
        //! \name CashFlow interface
        //@{
        Real amount() const;
        //@}
        //! \name Coupon interface
        //@{
        Rate rate() const;
        DayCounter dayCounter() const;
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        Date fixingDate() const;
        Rate indexFixing() const;
        Rate cap() const;
        Rate floor() const;
        //! \name interface
        //@{
        Rate convexityAdjustment() const;
        Rate effectiveCap() const;
        Rate effectiveFloor() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //! \name Modifiers
        //@{
        void setCapletVolatility(const Handle<CapletVolatilityStructure>& vol);
        //@}            
    protected:
        // data
        boost::shared_ptr<FloatingRateCoupon> underlying_;
        boost::shared_ptr<Optionlet> cap_, floor_;
        Handle<CapletVolatilityStructure> volatility_;
        
    };

}

#endif

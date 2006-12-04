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


namespace QuantLib {

    //! Cap/floorlet valued using the Black model
    class CappedFlooredCoupon : public FloatingRateCoupon {
      public:
        CappedFlooredCoupon(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate cap = Null<Rate>(), Rate floor = Null<Rate>());
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

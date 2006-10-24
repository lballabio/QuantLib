/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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
                  Rate cap,
                  Rate floor,
                  const boost::shared_ptr<BlackCapFloorModel>& model);
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
        //@}
      protected:
        // data
        boost::shared_ptr<FloatingRateCoupon> underlying_;
        boost::shared_ptr<CashFlow> cap_, floor_;
    };


    class CappedCoupon : public CappedFlooredCoupon {
      public:
        CappedCoupon(const boost::shared_ptr<FloatingRateCoupon>&
                                                            underlying,
                     Rate cap,
                     const boost::shared_ptr<BlackCapFloorModel>& model)
        : CappedFlooredCoupon(underlying,cap,Null<double>(),model) {}
    };

    class FlooredCoupon : public CappedFlooredCoupon {
      public:
        FlooredCoupon(const boost::shared_ptr<FloatingRateCoupon>&
                                                            underlying,
                      Rate floor,
                      const boost::shared_ptr<BlackCapFloorModel>& model)
        : CappedFlooredCoupon(underlying,Null<double>(),floor,model) {}
    };


}

#endif

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

/*! \file capfloorlet.hpp
    \brief Cap/floorlet valued using the Black model
*/

#ifndef quantlib_cap_floorlet_hpp
#define quantlib_cap_floorlet_hpp

//#include <qk/Pricers/blackcapfloormodel.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>

namespace QuantLib {

    //! Cap/floorlet valued using the Black model
    class CapFloorlet : public FloatingRateCoupon {
      public:
        CapFloorlet(const boost::shared_ptr<FloatingRateCoupon>&,
                    Rate strike,
                    const boost::shared_ptr<BlackCapFloorModel>&);
        //! \name CashFlow interface
        //@{
        double amount() const;
        //@}
        //! \name Coupon interface
        //@{
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
        Rate strike_;
        boost::shared_ptr<BlackCapFloorModel> model_;
        // utilities
        Time startTime() const;
        double volatility() const;
    };

    //! Caplet valued using the Black model
    class Caplet : public CapFloorlet {
      public:
        Caplet(const boost::shared_ptr<FloatingRateCoupon>& underlying,
               Rate cap,
               const boost::shared_ptr<BlackCapFloorModel>& model);
        //! \name Coupon interface
        //@{
        Rate rate() const;
        //@}
    };

    //! Floorlet valued using the Black model
    class Floorlet : public CapFloorlet {
      public:
        Floorlet(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                 Rate floor,
                 const boost::shared_ptr<BlackCapFloorModel>& model);
        //! \name Coupon interface
        //@{
        Rate rate() const;
        //@}
    };

}

#endif

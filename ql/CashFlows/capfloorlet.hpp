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

/*! \file capfloorlet.hpp
    \brief Cap/floorlet valued using the Black formula
*/

#ifndef quantlib_cap_floorlet_hpp
#define quantlib_cap_floorlet_hpp

#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/PricingEngines/blackcalculator.hpp>
#include <ql/capvolstructures.hpp>

namespace QuantLib {

    //! Cap/floorlet valued using the Black model
    class Optionlet : public FloatingRateCoupon {
      public:
        Optionlet(const boost::shared_ptr<FloatingRateCoupon>&, Rate strike);
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
        void setCapletVolatility(const Handle<CapletVolatilityStructure>& vol);
      protected:
        // data
        boost::shared_ptr<FloatingRateCoupon> underlying_;
        Rate strike_;
        Handle<CapletVolatilityStructure> volatility_;
        // utilities
        Time startTime() const;
        double volatility() const;
    };

    //! Caplet valued using the Black model
    class Caplet : public Optionlet {
      public:
        Caplet(const boost::shared_ptr<FloatingRateCoupon>& underlying, 
               Rate cap);
        //! \name Coupon interface
        //@{
        Rate rate() const;
        //@}
    };

    //! Floorlet valued using the Black model
    class Floorlet : public Optionlet {
      public:
        Floorlet(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                 Rate floor);
        //! \name Coupon interface
        //@{
        Rate rate() const;
        //@}
    };
}

#endif

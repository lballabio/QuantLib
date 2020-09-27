/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file blackcapfloorengine.hpp
    \brief Black-formula cap/floor engine
*/

#ifndef quantlib_pricers_black_capfloor_hpp
#define quantlib_pricers_black_capfloor_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>

namespace QuantLib {

    class Quote;

    //! Black-formula cap/floor engine
    /*! \ingroup capfloorengines */
    class BlackCapFloorEngine : public CapFloor::engine {
      public:
        BlackCapFloorEngine(const Handle<YieldTermStructure>& discountCurve,
                            Volatility vol,
                            const DayCounter& dc = Actual365Fixed(),
                            Real displacement = 0.0);
        BlackCapFloorEngine(const Handle<YieldTermStructure>& discountCurve,
                            const Handle<Quote>& vol,
                            const DayCounter& dc = Actual365Fixed(),
                            Real displacement = 0.0);
        BlackCapFloorEngine(const Handle< YieldTermStructure > &discountCurve,
                            const Handle< OptionletVolatilityStructure > &vol,
                            Real displacement = Null< Real >());
        void calculate() const;
        Handle<YieldTermStructure> termStructure() { return discountCurve_; }
        Handle<OptionletVolatilityStructure> volatility() { return vol_; }
        Real displacement() const { return displacement_; }

      private:
        Handle<YieldTermStructure> discountCurve_;
        Handle<OptionletVolatilityStructure> vol_;
        Real displacement_;
    };

}

#endif

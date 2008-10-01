/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2007 Roland Lichters

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

/*! \file ratehelpers.hpp
    \brief deposit, FRA, futures, and swap rate helpers
*/

#ifndef quantlib_ratehelper_hpp
#define quantlib_ratehelper_hpp

#include <ql/termstructures/bootstraphelper.hpp>

namespace QuantLib {

    class Quote;
    class YieldTermStructure;

    class RateHelper : public BootstrapHelper<YieldTermStructure> {
      public:
        RateHelper(const Handle<Quote>& quote) : 
            BootstrapHelper<YieldTermStructure>(quote) {}
        RateHelper(Real quote) : 
            BootstrapHelper<YieldTermStructure>(quote) {}
        virtual Rate rate() const = 0;
    };

}

#endif

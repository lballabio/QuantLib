/*
 Copyright (C) 2025 Sotirios Papathanasopoulos
 
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

#ifndef quantlib_brlcdi_hpp
#define quantlib_brlcdi_hpp

#include <ql/currencies/america.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/daycounters/business252.hpp>

namespace QuantLib {

    //! BRL-CDI Index: relevant for https://en.wikipedia.org/wiki/Brazilian_Swap

    class Cdi : public OvernightIndex {
      public:
        explicit Cdi(const Handle<YieldTermStructure>& h = {})
        : OvernightIndex("CDI", 0, BRLCurrency(), Brazil(Brazil::Settlement), Business252(), h) {}

        Rate forecastFixing(const Date& fixingDate) const override;

    };

} 

#endif
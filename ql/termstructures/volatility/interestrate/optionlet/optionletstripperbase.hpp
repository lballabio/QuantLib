/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file optionletstripperbase.hpp

*/

#ifndef quantlib_optionletstripperbase_hpp
#define quantlib_optionletstripperbase_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatility/interestrate/capfloor/capfloortermvolsurface.hpp>
#include <vector>

namespace QuantLib {

    class OptionletStripperBase : public LazyObject {
      public:
        virtual const std::vector<Rate>& optionletStrikes(Size i) const = 0;
        virtual const std::vector<Volatility>& optionletVolatilities(Size i) const = 0;

        virtual const std::vector<Date>& optionletDates() const = 0;
        virtual const std::vector<Time>& optionletTimes() const = 0;
        virtual const std::vector<Rate>& atmOptionletRate() const = 0;

        virtual boost::shared_ptr<CapFloorTermVolSurface> termVolSurface() const = 0;
    };
}

#endif

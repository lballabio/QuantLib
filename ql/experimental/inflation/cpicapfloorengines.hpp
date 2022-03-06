/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Chris Kenyon


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

/*!
    \file cpicapfloorengines.hpp
    \brief Engines for CPI options
    \ingroup PricingEngines
*/

#ifndef quantlib_cpicapfloorengines_hpp
#define quantlib_cpicapfloorengines_hpp

#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/instruments/cpicapfloor.hpp>


namespace QuantLib {


    class CPICapFloorTermPriceSurface;


    //! This engine only adds timing functionality (e.g. different lag)
    //! w.r.t. an existing interpolated price surface.
    class InterpolatingCPICapFloorEngine : public CPICapFloor::engine {
        public:
          explicit InterpolatingCPICapFloorEngine(Handle<CPICapFloorTermPriceSurface>);

          void calculate() const override;
          virtual std::string name() const { return "InterpolatingCPICapFloorEngine"; }

          ~InterpolatingCPICapFloorEngine() override = default;

        protected:
          Handle<CPICapFloorTermPriceSurface> priceSurf_;
    };








} // namespace QuantLib

#endif // cpicapfloorengines_hpp


#ifndef id_18ee8637f54f37139d1fc705ea5dc5f6
#define id_18ee8637f54f37139d1fc705ea5dc5f6
inline bool test_18ee8637f54f37139d1fc705ea5dc5f6(int* i) { return i != 0; }
#endif

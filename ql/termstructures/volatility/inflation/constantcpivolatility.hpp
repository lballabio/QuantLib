/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Chris Kenyon

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

/*! \file constantcpivolatility.hpp
    \brief constant CPI volatility structure
 */

#ifndef quantlib_constant_cpi_volatility_hpp
#define quantlib_constant_cpi_volatility_hpp

#include <ql/termstructures/volatility/inflation/cpivolatilitystructure.hpp>

namespace QuantLib {

    //! Constant surface, no K or T dependence.
    class ConstantCPIVolatility : public CPIVolatilitySurface {
      public:
        //! \name Constructor
        //@{
        //! calculate the reference date based on the global evaluation date
        ConstantCPIVolatility(Volatility v,
                              Natural settlementDays,
                              const Calendar&,
                              BusinessDayConvention bdc,
                              const DayCounter& dc,
                              const Period& observationLag,
                              Frequency frequency,
                              bool indexIsInterpolated);
        //@}

        //! \name Limits
        //@{
        Date maxDate() const override { return Date::maxDate(); }
        //! the minimum strike for which the term structure can return vols
        Real minStrike() const override { return QL_MIN_REAL; }
        //! the maximum strike for which the term structure can return vols
        Real maxStrike() const override { return QL_MAX_REAL; }
        //@}

      private:
        Volatility volatilityImpl(Time length, Rate strike) const override;
        Volatility volatility_;
    };

}

#endif



#ifndef id_c605f032f2f84195ad0f75ccab7bd456
#define id_c605f032f2f84195ad0f75ccab7bd456
inline bool test_c605f032f2f84195ad0f75ccab7bd456(const int* i) {
    return i != nullptr;
}
#endif

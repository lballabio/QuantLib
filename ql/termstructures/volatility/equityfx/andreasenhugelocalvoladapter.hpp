/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017, 2018 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file andreasenhugelocalvoladapter.hpp
    \brief Implements the LocalVolTermStructure interface based on a
           Andreasen-Huge volatility interpolation
*/

#ifndef quantlib_andreasen_huge_local_volatility_adapter_hpp
#define quantlib_andreasen_huge_local_volatility_adapter_hpp

#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    class AndreasenHugeVolatilityInterpl;

    class AndreasenHugeLocalVolAdapter : public LocalVolTermStructure {
      public:
        explicit AndreasenHugeLocalVolAdapter(
            ext::shared_ptr<AndreasenHugeVolatilityInterpl> localVol);

        Date maxDate() const override;
        Real minStrike() const override;
        Real maxStrike() const override;

        Calendar calendar() const override;
        DayCounter dayCounter() const override;
        Natural settlementDays() const override;
        const Date& referenceDate() const override;

      protected:
        Volatility localVolImpl(Time t, Real strike) const override;

      private:
        const ext::shared_ptr<AndreasenHugeVolatilityInterpl> localVol_;
    };
}


#endif

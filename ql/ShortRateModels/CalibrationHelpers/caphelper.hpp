
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file caphelper.hpp
    \brief CapHelper calibration helper
*/

#ifndef quantlib_interest_rate_modelling_calibration_helpers_cap_h
#define quantlib_interest_rate_modelling_calibration_helpers_cap_h

#include <ql/ShortRateModels/calibrationhelper.hpp>
#include <ql/Instruments/capfloor.hpp>
#include <ql/Indexes/xibor.hpp>

namespace QuantLib {

    class CapHelper : public CalibrationHelper {
      public:
        //Constructor for ATM cap
        CapHelper(const Period& length,
                  const RelinkableHandle<Quote>& volatility,
                  const boost::shared_ptr<Xibor>& index,
                  const RelinkableHandle<TermStructure>& termStructure);

        virtual void addTimesTo(std::list<Time>& times) const;

        virtual Real modelValue() const;

        virtual Real blackPrice(Volatility volatility) const;

      private:
        boost::shared_ptr<Cap> cap_;
    };

}


#endif

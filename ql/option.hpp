
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file option.hpp
    \brief Base option class
*/

#ifndef quantlib_option_h
#define quantlib_option_h

#include <ql/instrument.hpp>

namespace QuantLib {

    //! base option class
    class Option : public Instrument {
      public:
        enum Type { Call, Put, Straddle };
        Option(const Handle<PricingEngine>& engine,
               const std::string& isinCode = "",
               const std::string& description = "")
        : Instrument(isinCode, description) {
            setPricingEngine(engine);
        }
    };

    //! additional %option results
    class Greeks : public virtual Results {
      public:
        Greeks() { reset(); }
        void reset() {
            delta =  gamma = theta = vega =
                rho = dividendRho = strikeSensitivity = Null<double>();
        }
        double delta, gamma;
        double theta;
        double vega;
        double rho, dividendRho;
        double strikeSensitivity;
    };

}


#endif

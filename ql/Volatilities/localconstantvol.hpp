
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file localconstantvol.hpp
    \brief Local constant volatility, no time dependence, no asset dependence

    \fullpath
    ql/Volatilities/%localconstantvol.hpp
*/

// $Id$

#ifndef quantlib_localconstantvol_hpp
#define quantlib_localconstantvol_hpp

#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Constant local volatility, no time-strike dependence
        /*! This class implements the LocalVolatilityTermStructure
            interface for a constant local volatility (no time/asset
            dependence).
            Local volatility and Black volatility are the same when
            volatility is at most time dependent, so this class is
            basically a proxy for BlackVolatilityTermStructure.
        */
        class LocalConstantVol : public LocalVolatilityTermStructure,
                                 public Patterns::Observer {
          public:
            // constructor
            LocalConstantVol(const Handle<BlackConstantVol>& blackConstantVol);
            // inspectors
            Date referenceDate() const { return blackConstantVol_->referenceDate(); }
            DayCounter dayCounter() const { return blackConstantVol_->dayCounter(); }
            Date maxDate() const { return blackConstantVol_->maxDate(); }
            // Observer interface
            void update();
          protected:
            double localVolImpl(Time t1, Time t2, double, bool extrapolate) const;
          private:
            Handle<BlackConstantVol> blackConstantVol_;
        };

        // inline definitions

        inline LocalConstantVol::LocalConstantVol(
            const Handle<BlackConstantVol>& blackConstantVol)
        : blackConstantVol_(blackConstantVol) {
            registerWith(blackConstantVol_);
        }

        inline void LocalConstantVol::update() {
            notifyObservers();
        }

        inline double LocalConstantVol::localVolImpl(Time t1, Time t2,
            double, bool extrapolate) const {
            return blackConstantVol_->blackForwardVol(t1, t2, 0.0, extrapolate);
        }
    }

}

#endif

/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file blackmodel.hpp
    \brief Abstract class for Black-type models (market models)

    \fullpath
    ql/InterestRateModelling/%blackmodel.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_black_model_h
#define quantlib_interest_rate_modelling_black_model_h

#include <ql/marketelement.hpp>
#include <ql/termstructure.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/Patterns/observable.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        //! Abstract short-rate model class
        class BlackModel : public Patterns::Observable,
                           public Patterns::Observer {
          public:
            BlackModel(
                const RelinkableHandle<MarketElement>& volatility,
                const RelinkableHandle<TermStructure>& termStructure)
            : volatility_(volatility), termStructure_(termStructure) {
                registerWith(volatility_);
                registerWith(termStructure_);
            }
            double volatility() const {
                return volatility_->value();
            }
            const RelinkableHandle<TermStructure>& termStructure() const {
                return termStructure_;
            }
            static double formula(double k, double f, double v, double w) {
                if (v == 0.0)
                    return QL_MAX(f*w - k*w, 0.0);
                double d1 = QL_LOG(f/k)/v + 0.5*v;
                double d2 = d1 - v;
                Math::CumulativeNormalDistribution phi;
                return f*w*phi(w*d1) - k*w*phi(w*d2);
            }
            void update() { notifyObservers(); }
          private:
            RelinkableHandle<MarketElement> volatility_;
            RelinkableHandle<TermStructure> termStructure_;
        };

    }

}
#endif

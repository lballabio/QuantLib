
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file capfloorpricer.hpp
    \brief cap and floor pricer class
*/

#ifndef quantlib_pricers_capfloor_pricer_h
#define quantlib_pricers_capfloor_pricer_h

#include <ql/Instruments/capfloor.hpp>
#include <ql/PricingEngines/genericengine.hpp>

namespace QuantLib {

    namespace Pricers {

        class DiscretizedCapFloor : public DiscretizedAsset {
          public:
            DiscretizedCapFloor(const Handle<NumericalMethod>& method,
                                const Instruments::CapFloorArguments& params)
            : DiscretizedAsset(method), arguments_(params) {}

            void reset(Size size) {
                values_ = Array(size, 0.0);
                adjustValues();
            }

            virtual void adjustValues();

            void addTimesTo(std::list<Time>& times) const {
                for (Size i=0; i<arguments_.startTimes.size(); i++) {
                    times.push_back(arguments_.startTimes[i]);
                    times.push_back(arguments_.endTimes[i]);
                }
            }

          private:
            Instruments::CapFloorArguments arguments_;
        };

    }

}

#endif

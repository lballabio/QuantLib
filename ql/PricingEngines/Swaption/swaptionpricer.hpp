
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

/*! \file swaptionpricer.hpp
    \brief Swaption pricer class
*/

#ifndef quantlib_pricers_swaption_pricer_h
#define quantlib_pricers_swaption_pricer_h

#include <ql/Instruments/swaption.hpp>
#include <ql/discretizedasset.hpp>

namespace QuantLib {

    class DiscretizedSwap : public DiscretizedAsset {
      public:
        DiscretizedSwap(const boost::shared_ptr<NumericalMethod>& method,
                        const SimpleSwap::arguments& params)
        : DiscretizedAsset(method), arguments_(params) {}

        void reset(Size size) {
            values_ = Array(size, 0.0);
            adjustValues();
        }

        void preAdjustValues();

        void addTimesTo(std::list<Time>& times) const {
            Time t;
            Size i;
            for (i=0; i<arguments_.fixedPayTimes.size(); i++) {
                t = arguments_.fixedPayTimes[i];
                if (t >= 0.0)
                    times.push_back(t);
            }
            for (i=0; i<arguments_.floatingResetTimes.size(); i++) {
                t = arguments_.floatingResetTimes[i];
                if (t >= 0.0)
                    times.push_back(t);
            }
            for (i=0; i<arguments_.floatingPayTimes.size(); i++) {
                t = arguments_.floatingPayTimes[i];
                if (t >= 0.0)
                    times.push_back(t);
            }
        }
      private:
        SimpleSwap::arguments arguments_;
    };

    class DiscretizedSwaption : public DiscretizedOption {
      public:
        DiscretizedSwaption(const boost::shared_ptr<DiscretizedSwap>& swap,
                            const Swaption::arguments& params)
        : DiscretizedOption(swap,
                            params.exercise->type(),
                            params.stoppingTimes) {}
    };

}


#endif


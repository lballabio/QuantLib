
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

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

/*! \file discretizedcapfloor.hpp
    \brief discretized cap/floor
*/

#ifndef quantlib_pricers_capfloor_pricer_h
#define quantlib_pricers_capfloor_pricer_h

#include <ql/Instruments/capfloor.hpp>
#include <ql/discretizedasset.hpp>

namespace QuantLib {

    class DiscretizedCapFloor : public DiscretizedAsset {
      public:
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use the constructor with no arguments */
        DiscretizedCapFloor(const boost::shared_ptr<NumericalMethod>& method,
                            const CapFloor::arguments& args)
        : DiscretizedAsset(method), arguments_(args) {}
        #endif

        DiscretizedCapFloor(const CapFloor::arguments& args)
        : arguments_(args) {}

        void reset(Size size) {
            values_ = Array(size, 0.0);
            adjustValues();
        }

        void preAdjustValues();

        void addTimesTo(std::list<Time>& times) const {
            for (Size i=0; i<arguments_.startTimes.size(); i++) {
                times.push_back(arguments_.startTimes[i]);
                times.push_back(arguments_.endTimes[i]);
            }
        }
      protected:
        void preAdjustValuesImpl();
      private:
        CapFloor::arguments arguments_;
    };

}


#endif

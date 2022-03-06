/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2007 StatPro Italia srl

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

/*! \file discretizedcapfloor.hpp
    \brief discretized cap/floor
*/

#ifndef quantlib_pricers_capfloor_pricer_h
#define quantlib_pricers_capfloor_pricer_h

#include <ql/instruments/capfloor.hpp>
#include <ql/discretizedasset.hpp>

namespace QuantLib {

    class DiscretizedCapFloor : public DiscretizedAsset {
      public:
        DiscretizedCapFloor(const CapFloor::arguments& args,
                            const Date& referenceDate,
                            const DayCounter& dayCounter);
        void reset(Size size) override;
        std::vector<Time> mandatoryTimes() const override;

      protected:
        void preAdjustValuesImpl() override;
        void postAdjustValuesImpl() override;

      private:
        CapFloor::arguments arguments_;
        std::vector<Time> startTimes_;
        std::vector<Time> endTimes_;
    };

}


#endif


#ifndef id_09f1e88657af7a69b9e9586e0fb3ade5
#define id_09f1e88657af7a69b9e9586e0fb3ade5
inline bool test_09f1e88657af7a69b9e9586e0fb3ade5(const int* i) {
    return i != nullptr;
}
#endif


/*
 Copyright (C) 2003, 2004 StatPro Italia srl

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

/*! \file Instruments/cliquetoption.hpp
    \brief Cliquet option
*/

#ifndef ql_cliquet_option_hpp
#define ql_cliquet_option_hpp

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    // currently just a container for arguments and results
    class CliquetOption {
      public:
        class arguments;
        // class results;
    };

    //! %Arguments for cliquet option calculation
    // should inherit from a strikeless version of VanillaOption::arguments
    class CliquetOption::arguments : public VanillaOption::arguments {
      public:
        arguments() : moneyness(Null<double>()),
                      accruedCoupon(Null<double>()),
                      lastFixing(Null<double>()),
                      localCap(Null<double>()),
                      localFloor(Null<double>()),
                      globalCap(Null<double>()),
                      globalFloor(Null<double>()) {}
        void validate() const;
        double moneyness, accruedCoupon, lastFixing;
        double localCap, localFloor, globalCap, globalFloor;
        std::vector<Date> resetDates;
    };

    //! Cliquet engine base class
    class CliquetEngine : public GenericEngine<CliquetOption::arguments,
                                               VanillaOption::results> {};


    // inline definitions

    inline void CliquetOption::arguments::validate() const {
        #if defined(QL_PATCH_MICROSOFT)
        VanillaOption::arguments copy = *this;
        copy.validate();
        #else
        VanillaOption::arguments::validate();
        #endif

        QL_REQUIRE(moneyness != Null<double>(),
                   "CliquetOption::arguments::validate() : "
                   "null moneyness given");
        QL_REQUIRE(moneyness > 0.0,
                   "CliquetOption::arguments::validate() : "
                   "negative or zero moneyness given");
        QL_REQUIRE(accruedCoupon == Null<double>() || accruedCoupon >= 0.0,
                   "CliquetOption::arguments::validate() : "
                   "negative accrued coupon");
        QL_REQUIRE(localCap == Null<double>() || localCap >= 0.0,
                   "CliquetOption::arguments::validate() : "
                   "negative local cap");
        QL_REQUIRE(localFloor == Null<double>() || localFloor >= 0.0,
                   "CliquetOption::arguments::validate() : "
                   "negative local floor");
        QL_REQUIRE(globalCap == Null<double>() || globalCap >= 0.0,
                   "CliquetOption::arguments::validate() : "
                   "negative global cap");
        QL_REQUIRE(globalFloor == Null<double>() || globalFloor >= 0.0,
                   "CliquetOption::arguments::validate() : "
                   "negative global floor");
        QL_REQUIRE(resetDates.size()>0,
                   "CliquetOption::arguments::validate() : "
                   "no reset dates given");
        // sort resetDates here ???
        for (Size i = 0; i < resetDates.size(); i++) {

            QL_REQUIRE(exercise->lastDate() >= resetDates[i],
                       "CliquetOption::arguments::validate() : "
                       "reset date greater than exercise last date");
        }
    }

}


#endif

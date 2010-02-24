/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl

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

/*! \file cliquetoption.hpp
    \brief Cliquet option
*/

#ifndef ql_cliquet_option_hpp
#define ql_cliquet_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/time/date.hpp>
#include <vector>

namespace QuantLib {

    class EuropeanExercise;

    //! cliquet (Ratchet) option
    /*! A cliquet option, also known as Ratchet option, is a series of
        forward-starting (a.k.a. deferred strike) options where the
        strike for each forward start option is set equal to a fixed
        percentage of the spot price at the beginning of each period.

        \todo
        - add local/global caps/floors
        - add accrued coupon and last fixing

        \ingroup instruments
    */
    class CliquetOption : public OneAssetOption {
      public:
        class arguments;
        class engine;
        CliquetOption(const boost::shared_ptr<PercentageStrikePayoff>&,
                      const boost::shared_ptr<EuropeanExercise>& maturity,
                      const std::vector<Date>& resetDates);
        void setupArguments(PricingEngine::arguments*) const;
      private:
        std::vector<Date> resetDates_;
    };

    //! %Arguments for cliquet option calculation
    // should inherit from a strikeless version of VanillaOption::arguments
    class CliquetOption::arguments : public OneAssetOption::arguments {
      public:
        arguments() : accruedCoupon(Null<Real>()),
                      lastFixing(Null<Real>()),
                      localCap(Null<Real>()),
                      localFloor(Null<Real>()),
                      globalCap(Null<Real>()),
                      globalFloor(Null<Real>()) {}
        void validate() const;
        Real accruedCoupon, lastFixing;
        Real localCap, localFloor, globalCap, globalFloor;
        std::vector<Date> resetDates;
    };

    //! Cliquet %engine base class
    class CliquetOption::engine
        : public GenericEngine<CliquetOption::arguments,
                               CliquetOption::results> {};

}


#endif

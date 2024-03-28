/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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

/*! \file swaptioncfs.hpp
    \brief translate swaption into deterministic fixed and float cash flows
*/

#ifndef quantlib_swaptioncfs_hpp
#define quantlib_swaptioncfs_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/option.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/date.hpp>

namespace QuantLib {

    class IborLegCashFlows {
      protected:
        Date refDate_; // today, base for time calculations w.r.t. Act/365 (Fixed)
        Leg floatLeg_;
        std::vector<Real> floatTimes_;
        std::vector<Real> floatWeights_;

      public:
        inline const Leg& floatLeg() const { return floatLeg_; }
        inline const std::vector<Real>& floatTimes() const { return floatTimes_; }
        inline const std::vector<Real>& floatWeights() const { return floatWeights_; }
        IborLegCashFlows(const Leg& iborLeg,
                         const Handle<YieldTermStructure>& discountCurve,
                         bool contTenorSpread = true);
        IborLegCashFlows() = default;
        ; // allow default constructor which does nothing
    };


    class SwapCashFlows : public IborLegCashFlows {
      protected:
        // resulting cash flows as leg
        Leg fixedLeg_;
        std::vector<Real> fixedTimes_;
        std::vector<Real> fixedWeights_;
        std::vector<Real> annuityWeights_;

      public:
        SwapCashFlows(const ext::shared_ptr<FixedVsFloatingSwap>& swap,
                      const Handle<YieldTermStructure>& discountCurve,
                      bool contTenorSpread = true);
        SwapCashFlows() = default;
        ; // allow default constructor which does nothing
          // inspectors
        inline const Leg& fixedLeg() const { return fixedLeg_; }
        inline const std::vector<Real>& fixedTimes() const { return fixedTimes_; }
        inline const std::vector<Real>& fixedWeights() const { return fixedWeights_; }
        inline const std::vector<Real>& annuityWeights() const { return annuityWeights_; }
    };


    class SwaptionCashFlows : public SwapCashFlows {
      protected:
        ext::shared_ptr<Swaption> swaption_;
        std::vector<Real> exerciseTimes_;

      public:
        SwaptionCashFlows(const ext::shared_ptr<Swaption>& swaption,
                          const Handle<YieldTermStructure>& discountCurve,
                          bool contTenorSpread = true);
        SwaptionCashFlows() = default;
        ; // allow default constructor which does nothing
        // inspectors
        inline ext::shared_ptr<Swaption> swaption() const { return swaption_; }
        inline const std::vector<Real>& exerciseTimes() const { return exerciseTimes_; }
    };


}

#endif

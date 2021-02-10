/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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

/*! \file dynprogvppintrinsicvalueengine.hpp
    \brief intrinsic value engine using dynamic programming
*/

#ifndef quantlib_dp_vpp_intrinsic_value_engine_hpp
#define quantlib_dp_vpp_intrinsic_value_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/experimental/finitedifferences/vanillavppoption.hpp>
#include <vector>

namespace QuantLib {

    class YieldTermStructure;

    class DynProgVPPIntrinsicValueEngine
        : public GenericEngine<VanillaVPPOption::arguments,
                               VanillaVPPOption::results> {
      public:
        DynProgVPPIntrinsicValueEngine(std::vector<Real> fuelPrices,
                                       std::vector<Real> powerPrices,
                                       Real fuelCostAddon,
                                       ext::shared_ptr<YieldTermStructure> rTS);

        void calculate() const override;

      private:
        const std::vector<Real> fuelPrices_;
        const std::vector<Real> powerPrices_;
        const Real fuelCostAddon_;
        const ext::shared_ptr<YieldTermStructure> rTS_;
    };
}

#endif


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file gaussian1dcapfloorengine.hpp
    \brief
*/

#ifndef quantlib_pricers_gaussian1d_capfloor_hpp
#define quantlib_pricers_gaussian1d_capfloor_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/experimental/models/gaussian1dmodel.hpp>

namespace QuantLib {

    //! Gaussian1d cap/floor engine
    /*! \ingroup capfloorengines
    */

    class Gaussian1dCapFloorEngine
        : public GenericModelEngine<Gaussian1dModel, CapFloor::arguments,
                                    CapFloor::results> {
      public:
        Gaussian1dCapFloorEngine(
            const boost::shared_ptr<Gaussian1dModel> &model,
            const int integrationPoints = 64, const Real stddevs = 7.0,
            const bool extrapolatePayoff = true,
            const bool flatPayoffExtrapolation = false,
            const Handle<YieldTermStructure> &discountCurve =
                Handle<YieldTermStructure>())
            : GenericModelEngine<Gaussian1dModel, CapFloor::arguments,
                                 CapFloor::results>(model),
              integrationPoints_(integrationPoints), stddevs_(stddevs),
              extrapolatePayoff_(extrapolatePayoff),
              flatPayoffExtrapolation_(flatPayoffExtrapolation),
              discountCurve_(discountCurve) {}
        void calculate() const;

      private:
        const int integrationPoints_;
        const Real stddevs_;
        const bool extrapolatePayoff_, flatPayoffExtrapolation_;
        const Handle<YieldTermStructure> discountCurve_;
    };
}

#endif


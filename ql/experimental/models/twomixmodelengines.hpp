/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file twomixmodelengines.hpp
    \brief Pricing engines for two-components mixture models
*/

#ifndef quantlib_two_mix_model_engines_hpp
#define quantlib_two_mix_model_engines_hpp

#include <ql/experimental/models/twomixmodel.hpp>
#include <ql/experimental/inflation/inflationcapfloorengines.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/instruments/capfloor.hpp>

namespace QuantLib {


    //! Mixture Bachelier engine
    /*! Normal model with two components.

        params: lambda, mu, sig1, sig2
    */
    class MixtureBachelierYoYInflationCapFloorEngine
        : public YoYInflationCapFloorEngine {
      public:
        MixtureBachelierYoYInflationCapFloorEngine(
                                            const Handle<YieldTermStructure>&,
                                            const std::vector<Time> &times,
                                            const std::vector<Real> &lambda,
                                            const std::vector<Real> &mu,
                                            const std::vector<Real> &sig1,
                                            const std::vector<Real> &sig2);

        virtual void setVolatility(
                              const Handle<YoYOptionletVolatilitySurface> &) {
            QL_FAIL("setVolatility is NOT valid for a mixture model");
        }
        void calculate() const;

      protected:
        // N.B. values at different times are not necessarily
        // the values for the same parameter, i.e. there is
        // ambiguity between sig1_ and sig2_.
        // Storing the values in PCP's is for convenience!
        mutable PiecewiseConstantParameter lambda_;
        mutable PiecewiseConstantParameter mu_;
        mutable PiecewiseConstantParameter sig1_;
        mutable PiecewiseConstantParameter sig2_;
    };


    /*! This prices only the caplet effective during time t. */
    class MNDMYoYInflationCapFloorModelEngine
        : public GenericModelEngine<TwoBachelierModel,
                                    YoYInflationCapFloor::arguments,
                                    YoYInflationCapFloor::results> {
      public:
        MNDMYoYInflationCapFloorModelEngine(
                              const boost::shared_ptr<TwoBachelierModel> &mod,
                              const Date &d)
        : GenericModelEngine<TwoBachelierModel,
                             YoYInflationCapFloor::arguments,
                             YoYInflationCapFloor::results>(mod),
          d_(d) {}

        void calculate() const;

      protected:
        Date d_;
    };


    //! Two-component SLMUPg model engine
    /*! The spread and gearing are taken from the original coupon. */
    class SLMUP2CapFloorModelEngine
        : public GenericModelEngine<TwoDDModel,
                                    CapFloor::arguments,
                                    CapFloor::results> {
      public:
        SLMUP2CapFloorModelEngine(const boost::shared_ptr<TwoDDModel> &mod)
        : GenericModelEngine<TwoDDModel,
                             CapFloor::arguments,
                             CapFloor::results>(mod) {}

        void calculate() const;
    };

}

#endif


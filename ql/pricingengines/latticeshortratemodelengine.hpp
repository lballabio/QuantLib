/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file latticeshortratemodelengine.hpp
    \brief Engine for a short-rate model specialized on a lattice
*/

#ifndef quantlib_short_rate_model_engine_hpp
#define quantlib_short_rate_model_engine_hpp

#include <ql/models/model.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>

namespace QuantLib {

    //! Engine for a short-rate model specialized on a lattice
    /*! Derived engines only need to implement the <tt>calculate()</tt>
        method
    */
    template <class Arguments, class Results>
    class LatticeShortRateModelEngine
        : public GenericModelEngine<ShortRateModel, Arguments, Results> {
      public:
        LatticeShortRateModelEngine(
                               const boost::shared_ptr<ShortRateModel>& model,
                               Size timeSteps);
        LatticeShortRateModelEngine(
                               const Handle<ShortRateModel>& model,
                               Size timeSteps);
        LatticeShortRateModelEngine(
                               const boost::shared_ptr<ShortRateModel>& model,
                               const TimeGrid& timeGrid);
        void update();
      protected:
        TimeGrid timeGrid_;
        Size timeSteps_;
        boost::shared_ptr<Lattice> lattice_;
    };

    template <class Arguments, class Results>
    LatticeShortRateModelEngine<Arguments, Results>::LatticeShortRateModelEngine(
            const boost::shared_ptr<ShortRateModel>& model,
            Size timeSteps)
    : GenericModelEngine<ShortRateModel, Arguments, Results>(model),
      timeSteps_(timeSteps) {
        QL_REQUIRE(timeSteps>0,
                   "timeSteps must be positive, " << timeSteps <<
                   " not allowed");
    }

    template <class Arguments, class Results>
    LatticeShortRateModelEngine<Arguments, Results>::LatticeShortRateModelEngine(
            const Handle<ShortRateModel>& model,
            Size timeSteps)
    : GenericModelEngine<ShortRateModel, Arguments, Results>(model),
      timeSteps_(timeSteps) {
        QL_REQUIRE(timeSteps>0,
                   "timeSteps must be positive, " << timeSteps <<
                   " not allowed");
    }

    template <class Arguments, class Results>
    LatticeShortRateModelEngine<Arguments, Results>::LatticeShortRateModelEngine(
            const boost::shared_ptr<ShortRateModel>& model,
            const TimeGrid& timeGrid)
    : GenericModelEngine<ShortRateModel, Arguments, Results>(model),
      timeGrid_(timeGrid), timeSteps_(0) {
        lattice_ = this->model_->tree(timeGrid);
    }

    template <class Arguments, class Results>
    void LatticeShortRateModelEngine<Arguments, Results>::update()
    {
        if (!timeGrid_.empty())
            lattice_ = this->model_->tree(timeGrid_);
        GenericModelEngine<ShortRateModel, Arguments, Results>::update();
    }

}


#endif

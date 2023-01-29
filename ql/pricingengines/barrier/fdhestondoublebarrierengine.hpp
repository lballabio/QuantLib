/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file fdhestondoublebarrierengine.hpp
    \brief Finite-Differences Heston double barrier option engine
*/

#ifndef quantlib_fd_heston_double_barrier_engine_hpp
#define quantlib_fd_heston_double_barrier_engine_hpp

#include <ql/instruments/doublebarrieroption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhestonsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>


namespace QuantLib {

    //! Finite-Differences Heston Double Barrier Option engine

    /*!
        \ingroup barrierengines
    */
    class FdHestonDoubleBarrierEngine
        : public GenericModelEngine<HestonModel,
                                    DoubleBarrierOption::arguments,
                                    DoubleBarrierOption::results> {
      public:
        // Constructor
        explicit FdHestonDoubleBarrierEngine(
            const ext::shared_ptr<HestonModel>& model,
            Size tGrid = 100,
            Size xGrid = 100,
            Size vGrid = 50,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer(),
            ext::shared_ptr<LocalVolTermStructure> leverageFct =
                ext::shared_ptr<LocalVolTermStructure>(),
            Real mixingFactor = 1.0);

        void calculate() const override;

      private:
        const Size tGrid_, xGrid_, vGrid_, dampingSteps_;
        const FdmSchemeDesc schemeDesc_;
        const ext::shared_ptr<LocalVolTermStructure> leverageFct_;
        const Real mixingFactor_;
    };


}

#endif

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file fdhestonbarrierengine.hpp
    \brief Finite-differences Heston barrier-option engine
*/

#ifndef quantlib_fd_heston_barrier_engine_hpp
#define quantlib_fd_heston_barrier_engine_hpp

#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhestonsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/instruments/dividendbarrieroption.hpp>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    //! Finite-differences Heston barrier-option engine
    /*! \ingroup barrierengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
              and comparison with Black pricing.
    */
    class FdHestonBarrierEngine
        : public GenericModelEngine<HestonModel,
                                    DividendBarrierOption::arguments,
                                    DividendBarrierOption::results> {
        QL_DEPRECATED_ENABLE_WARNING
      public:
        explicit FdHestonBarrierEngine(
            const ext::shared_ptr<HestonModel>& model,
            Size tGrid = 100,
            Size xGrid = 100,
            Size vGrid = 50,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer(),
            ext::shared_ptr<LocalVolTermStructure> leverageFct = {},
            Real mixingFactor = 1.0);

        explicit FdHestonBarrierEngine(
            const ext::shared_ptr<HestonModel>& model,
            DividendSchedule dividends,
            Size tGrid = 100,
            Size xGrid = 100,
            Size vGrid = 50,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer(),
            ext::shared_ptr<LocalVolTermStructure> leverageFct = {},
            Real mixingFactor = 1.0);

        void calculate() const override;

      private:
        DividendSchedule dividends_;
        bool explicitDividends_;
        Size tGrid_, xGrid_, vGrid_, dampingSteps_;
        FdmSchemeDesc schemeDesc_;
        ext::shared_ptr<LocalVolTermStructure> leverageFct_;
        Real mixingFactor_;
    };

}

#endif

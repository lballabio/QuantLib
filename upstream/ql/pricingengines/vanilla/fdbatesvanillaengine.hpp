/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdbatesvanillaengine.hpp
    \brief Partial integro finite-differences Bates vanilla option engine
*/

#ifndef quantlib_fd_bates_vanilla_engine_hpp
#define quantlib_fd_bates_vanilla_engine_hpp

#include <ql/models/equity/batesmodel.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    //! Partial integro finite-differences Bates vanilla option engine
    /*! \ingroup vanillaengines */
    class FdBatesVanillaEngine
        : public GenericModelEngine<BatesModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        explicit
        FdBatesVanillaEngine(
            const ext::shared_ptr<BatesModel>& model,
            Size tGrid = 100, Size xGrid = 100, 
            Size vGrid = 50, Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        FdBatesVanillaEngine(
            const ext::shared_ptr<BatesModel>& model,
            DividendSchedule dividends,
            Size tGrid = 100, Size xGrid = 100, 
            Size vGrid = 50, Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        void calculate() const override;

      private:
        DividendSchedule dividends_;
        const Size tGrid_, xGrid_, vGrid_, dampingSteps_;
        const FdmSchemeDesc schemeDesc_;
    };

}

#endif

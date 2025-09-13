/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file fdhestonhullwhitevanillaengine.hpp
    \brief Finite-differences Heston Hull-White vanilla option engine
*/

#ifndef quantlib_fd_heston_hull_white_vanilla_engine_hpp
#define quantlib_fd_heston_hull_white_vanilla_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmhestonhullwhitesolver.hpp>

namespace QuantLib {

    //! Finite-differences Heston Hull-White vanilla option engine
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
              and comparison with Black/Heston pricing.
    */
    class FdHestonHullWhiteVanillaEngine
        : public GenericModelEngine<HestonModel,
                                    VanillaOption::arguments,
                                    VanillaOption::results> {
      public:
        FdHestonHullWhiteVanillaEngine(
            const ext::shared_ptr<HestonModel>& model,
            ext::shared_ptr<HullWhiteProcess> hwProcess,
            Real corrEquityShortRate,
            Size tGrid = 50,
            Size xGrid = 100,
            Size vGrid = 40,
            Size rGrid = 20,
            Size dampingSteps = 0,
            bool controlVariate = true,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        FdHestonHullWhiteVanillaEngine(
            const ext::shared_ptr<HestonModel>& model,
            ext::shared_ptr<HullWhiteProcess> hwProcess,
            DividendSchedule dividends,
            Real corrEquityShortRate,
            Size tGrid = 50,
            Size xGrid = 100,
            Size vGrid = 40,
            Size rGrid = 20,
            Size dampingSteps = 0,
            bool controlVariate = true,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Hundsdorfer());

        void calculate() const override;

        // multiple strikes caching engine
        void update() override;
        void enableMultipleStrikesCaching(const std::vector<Real>& strikes);
        
      private:
        const ext::shared_ptr<HullWhiteProcess> hwProcess_;
        DividendSchedule dividends_;
        const Real corrEquityShortRate_;
        const Size tGrid_, xGrid_, vGrid_, rGrid_;
        const Size dampingSteps_;
        const FdmSchemeDesc schemeDesc_;
        const bool controlVariate_;
        
        std::vector<Real> strikes_;
        mutable std::vector<std::pair<VanillaOption::arguments,
                                      VanillaOption::results> >
                                                            cachedArgs2results_;
    };

}

#endif

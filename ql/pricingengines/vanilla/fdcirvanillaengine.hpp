/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

/*! \file fdcirvanillaengine.hpp
    \brief Finite-Differences CIR vanilla option engine
*/

#ifndef quantlib_fd_cir_vanilla_engine_hpp
#define quantlib_fd_cir_vanilla_engine_hpp

#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/solvers/fdmsolverdesc.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/processes/coxingersollrossprocess.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    //! Finite-Differences CIR Vanilla Option engine

    /*! \ingroup vanillaengines

        \test the engine has been tested to converge among different schemes.
    */
    class FdmQuantoHelper;

    class FdCIRVanillaEngine : public DividendVanillaOption::engine {
      public:
        explicit FdCIRVanillaEngine(ext::shared_ptr<CoxIngersollRossProcess> cirProcess,
                                    ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess,
                                    Size tGrid,
                                    Size xGrid,
                                    Size vGrid,
                                    Size dampingSteps,
                                    Real rho,
                                    const FdmSchemeDesc& schemeDesc,
                                    ext::shared_ptr<FdmQuantoHelper> quantoHelper);

        void calculate() const override;

        FdmSolverDesc getSolverDesc(Real equityScaleFactor) const;

      private:
        const Size tGrid_, xGrid_, rGrid_, dampingSteps_;
        const Real rho_;
        const FdmSchemeDesc schemeDesc_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess_;
        ext::shared_ptr<CoxIngersollRossProcess> cirProcess_;
        
        ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
    };

    class MakeFdCIRVanillaEngine {
      public:
        explicit MakeFdCIRVanillaEngine(ext::shared_ptr<CoxIngersollRossProcess> cirProcess,
                                        ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess,
                                        Real rho);

        MakeFdCIRVanillaEngine& withQuantoHelper(
            const ext::shared_ptr<FdmQuantoHelper>& quantoHelper);

        MakeFdCIRVanillaEngine& withTGrid(Size tGrid);
        MakeFdCIRVanillaEngine& withXGrid(Size xGrid);
        MakeFdCIRVanillaEngine& withRGrid(Size rGrid);
        MakeFdCIRVanillaEngine& withDampingSteps(
            Size dampingSteps);

        MakeFdCIRVanillaEngine& withFdmSchemeDesc(
            const FdmSchemeDesc& schemeDesc);

        operator ext::shared_ptr<PricingEngine>() const;

      private:
        ext::shared_ptr<CoxIngersollRossProcess> cirProcess_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess_;
        const Real rho_;
        Size tGrid_, xGrid_, rGrid_, dampingSteps_;
        ext::shared_ptr<FdmSchemeDesc> schemeDesc_;
        ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
    };
}

#endif


#ifndef id_adc3eeb1d29cc2c2ad698ea4e5801646
#define id_adc3eeb1d29cc2c2ad698ea4e5801646
inline bool test_adc3eeb1d29cc2c2ad698ea4e5801646(const int* i) {
    return i != nullptr;
}
#endif

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

/*! \file fdblackscholesvanillaengine.hpp
    \brief Finite-differences Black Scholes vanilla option engine
*/

#ifndef quantlib_fd_black_scholes_vanilla_engine_hpp
#define quantlib_fd_black_scholes_vanilla_engine_hpp

#include <ql/pricingengine.hpp>
#include <ql/pricingengines/vanilla/cashdividendeuropeanengine.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>

namespace QuantLib {

    class FdmQuantoHelper;
    class GeneralizedBlackScholesProcess;

    //! Finite-differences Black Scholes vanilla option engine
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
              and comparison with Black pricing.
    */
    class FdBlackScholesVanillaEngine : public VanillaOption::engine {
      public:
        enum CashDividendModel {
            Spot = CashDividendEuropeanEngine::Spot,
            Escrowed = CashDividendEuropeanEngine::Escrowed
        };

        explicit FdBlackScholesVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot);

        FdBlackScholesVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            DividendSchedule dividends,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot);

        FdBlackScholesVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            ext::shared_ptr<FdmQuantoHelper> quantoHelper,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot);

        FdBlackScholesVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess>,
            DividendSchedule dividends,
            ext::shared_ptr<FdmQuantoHelper> quantoHelper,
            Size tGrid = 100,
            Size xGrid = 100,
            Size dampingSteps = 0,
            const FdmSchemeDesc& schemeDesc = FdmSchemeDesc::Douglas(),
            bool localVol = false,
            Real illegalLocalVolOverwrite = -Null<Real>(),
            CashDividendModel cashDividendModel = Spot);

        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        DividendSchedule dividends_;
        Size tGrid_, xGrid_, dampingSteps_;
        FdmSchemeDesc schemeDesc_;
        bool localVol_;
        Real illegalLocalVolOverwrite_;
        ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
        CashDividendModel cashDividendModel_;
    };


    class MakeFdBlackScholesVanillaEngine {
      public:
        explicit MakeFdBlackScholesVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);

        MakeFdBlackScholesVanillaEngine& withQuantoHelper(
            const ext::shared_ptr<FdmQuantoHelper>& quantoHelper);

        MakeFdBlackScholesVanillaEngine& withTGrid(Size tGrid);
        MakeFdBlackScholesVanillaEngine& withXGrid(Size xGrid);
        MakeFdBlackScholesVanillaEngine& withDampingSteps(
            Size dampingSteps);

        MakeFdBlackScholesVanillaEngine& withFdmSchemeDesc(
            const FdmSchemeDesc& schemeDesc);

        MakeFdBlackScholesVanillaEngine& withLocalVol(bool localVol);
        MakeFdBlackScholesVanillaEngine& withIllegalLocalVolOverwrite(
            Real illegalLocalVolOverwrite);

        MakeFdBlackScholesVanillaEngine& withCashDividends(
            const std::vector<Date>& dividendDates,
            const std::vector<Real>& dividendAmounts);

        MakeFdBlackScholesVanillaEngine& withCashDividendModel(
            FdBlackScholesVanillaEngine::CashDividendModel cashDividendModel);

        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        DividendSchedule dividends_;
        Size tGrid_ = 100, xGrid_ = 100, dampingSteps_ = 0;
        ext::shared_ptr<FdmSchemeDesc> schemeDesc_;
        bool localVol_ = false;
        Real illegalLocalVolOverwrite_;
        ext::shared_ptr<FdmQuantoHelper> quantoHelper_;
        FdBlackScholesVanillaEngine::CashDividendModel cashDividendModel_ = FdBlackScholesVanillaEngine::Spot;
    };

}

#endif

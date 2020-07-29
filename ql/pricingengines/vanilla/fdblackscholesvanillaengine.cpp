/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

#include <ql/exercise.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/methods/finitedifferences/solvers/fdmblackscholessolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>

namespace QuantLib {

    FdBlackScholesVanillaEngine::FdBlackScholesVanillaEngine(
            const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Size tGrid, Size xGrid, Size dampingSteps, 
            const FdmSchemeDesc& schemeDesc,
            bool localVol, Real illegalLocalVolOverwrite,
            CashDividendModel cashDividendModel)
    : process_(process),
      tGrid_(tGrid), xGrid_(xGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc),
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite),
      quantoHelper_(ext::shared_ptr<FdmQuantoHelper>()),
      cashDividendModel_(cashDividendModel) {
        registerWith(process_);
    }

    FdBlackScholesVanillaEngine::FdBlackScholesVanillaEngine(
            const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
            const ext::shared_ptr<FdmQuantoHelper>& quantoHelper,
            Size tGrid, Size xGrid, Size dampingSteps,
            const FdmSchemeDesc& schemeDesc,
            bool localVol, Real illegalLocalVolOverwrite,
            CashDividendModel cashDividendModel)
    : process_(process),
      tGrid_(tGrid), xGrid_(xGrid), dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc), 
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite),
      quantoHelper_(quantoHelper),
      cashDividendModel_(cashDividendModel) {
        registerWith(process_);
        registerWith(quantoHelper_);
    }


    void FdBlackScholesVanillaEngine::calculate() const {

        // 0. Cash dividend model
        const Date exerciseDate = arguments_.exercise->lastDate();
        const Time maturity = process_->time(exerciseDate);
        const Date settlementDate = process_->riskFreeRate()->referenceDate();

        Real spotAdjustment = 0.0;
        DividendSchedule dividendSchedule = DividendSchedule();

        switch (cashDividendModel_) {
          case Spot:
            dividendSchedule = arguments_.cashFlow;
            break;
          case Escrowed:
            for (DividendSchedule::const_iterator
                    divIter = arguments_.cashFlow.begin();
                 divIter != arguments_.cashFlow.end(); ++divIter) {

                const Date divDate = (*divIter)->date();

                if (divDate <= exerciseDate && divDate >= settlementDate) {
                    const Real divAmount = (*divIter)->amount();

                    if (divAmount != 0.0) {
                        QL_REQUIRE(arguments_.exercise->type()
                                        == Exercise::European,
                             "Escrowed dividend model expects an European option");

                        const DiscountFactor discount =
                            process_->riskFreeRate()->discount(divDate) /
                            process_->dividendYield()->discount(divDate);

                        spotAdjustment -= divAmount * discount;
                    }
                }
            }

            QL_REQUIRE(process_->x0() + spotAdjustment > 0.0,
                    "spot minus dividends becomes negative");
            break;
          default:
              QL_FAIL("unknwon cash dividend model");
        }

        // 1. Mesher
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);

        const ext::shared_ptr<Fdm1dMesher> equityMesher(
            new FdmBlackScholesMesher(
                    xGrid_, process_, maturity, payoff->strike(), 
                    Null<Real>(), Null<Real>(), 0.0001, 1.5, 
                    std::pair<Real, Real>(payoff->strike(), 0.1),
                    dividendSchedule, quantoHelper_,
                    spotAdjustment));
        
        const ext::shared_ptr<FdmMesher> mesher (
            new FdmMesherComposite(equityMesher));
        
        // 2. Calculator
        const ext::shared_ptr<FdmInnerValueCalculator> calculator(
                                      new FdmLogInnerValue(payoff, mesher, 0));

        // 3. Step conditions
        const ext::shared_ptr<FdmStepConditionComposite> conditions = 
            FdmStepConditionComposite::vanillaComposite(
                dividendSchedule, arguments_.exercise, mesher, calculator,
                process_->riskFreeRate()->referenceDate(),
                process_->riskFreeRate()->dayCounter());

        // 4. Boundary conditions
        const FdmBoundaryConditionSet boundaries;

        // 5. Solver
        FdmSolverDesc solverDesc = { mesher, boundaries, conditions, calculator,
                                     maturity, tGrid_, dampingSteps_ };

        const ext::shared_ptr<FdmBlackScholesSolver> solver(
            ext::make_shared<FdmBlackScholesSolver>(
                Handle<GeneralizedBlackScholesProcess>(process_),
                payoff->strike(), solverDesc, schemeDesc_,
                localVol_, illegalLocalVolOverwrite_,
                Handle<FdmQuantoHelper>(quantoHelper_)));

        const Real spot = process_->x0() + spotAdjustment;

        results_.value = solver->valueAt(spot);
        results_.delta = solver->deltaAt(spot);
        results_.gamma = solver->gammaAt(spot);
        results_.theta = solver->thetaAt(spot);
    }

    MakeFdBlackScholesVanillaEngine::MakeFdBlackScholesVanillaEngine(
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
      : process_(process),
        tGrid_(100),
        xGrid_(100),
        dampingSteps_(0),
        schemeDesc_(ext::make_shared<FdmSchemeDesc>(FdmSchemeDesc::Douglas())),
        localVol_(false),
        illegalLocalVolOverwrite_(-Null<Real>()),
        quantoHelper_(ext::shared_ptr<FdmQuantoHelper>()),
        cashDividendModel_(FdBlackScholesVanillaEngine::Spot) {}

    MakeFdBlackScholesVanillaEngine&
    MakeFdBlackScholesVanillaEngine::withQuantoHelper(
        const ext::shared_ptr<FdmQuantoHelper>& quantoHelper) {
        quantoHelper_ = quantoHelper;
        return *this;
    }

    MakeFdBlackScholesVanillaEngine&
    MakeFdBlackScholesVanillaEngine::withTGrid(Size tGrid) {
        tGrid_ = tGrid;
        return *this;
    }

    MakeFdBlackScholesVanillaEngine&
    MakeFdBlackScholesVanillaEngine::withXGrid(Size xGrid) {
        xGrid_ = xGrid;
        return *this;
    }

    MakeFdBlackScholesVanillaEngine&
    MakeFdBlackScholesVanillaEngine::withDampingSteps(Size dampingSteps) {
        dampingSteps_ = dampingSteps;
        return *this;
    }

    MakeFdBlackScholesVanillaEngine&
    MakeFdBlackScholesVanillaEngine::withFdmSchemeDesc(
        const FdmSchemeDesc& schemeDesc) {
        schemeDesc_ = ext::make_shared<FdmSchemeDesc>(schemeDesc);
        return *this;
    }

    MakeFdBlackScholesVanillaEngine&
    MakeFdBlackScholesVanillaEngine::withLocalVol(bool localVol) {
        localVol_ = localVol;
        return *this;
    }

    MakeFdBlackScholesVanillaEngine&
    MakeFdBlackScholesVanillaEngine::withIllegalLocalVolOverwrite(
        Real illegalLocalVolOverwrite) {
        illegalLocalVolOverwrite_ = illegalLocalVolOverwrite;
        return *this;
    }

    MakeFdBlackScholesVanillaEngine&
    MakeFdBlackScholesVanillaEngine::withCashDividendModel(
        FdBlackScholesVanillaEngine::CashDividendModel cashDividendModel) {
        cashDividendModel_ = cashDividendModel;
        return *this;
    }

    MakeFdBlackScholesVanillaEngine::operator
    ext::shared_ptr<PricingEngine>() const {
        return ext::make_shared<FdBlackScholesVanillaEngine>(
            process_,
            quantoHelper_,
            tGrid_, xGrid_, dampingSteps_,
            *schemeDesc_,
            localVol_,
            illegalLocalVolOverwrite_,
            cashDividendModel_);
    }
}

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

/*! \file fdmhestonsolver.cpp
*/

#include <ql/methods/finitedifferences/multidim/fdmhestonsolver.hpp>
#include <ql/methods/finitedifferences/multidim/hundsdorferscheme.hpp>
#include <ql/methods/finitedifferences/multidim/craigsneydscheme.hpp>
#include <ql/methods/finitedifferences/multidim/douglasscheme.hpp>

namespace QuantLib {

    namespace {
        const boost::shared_ptr<FdmStepConditionComposite> addCondition(
                     const boost::shared_ptr<FdmSnapshotCondition>& c1,
                     const boost::shared_ptr<FdmStepConditionComposite>& c2) {

             std::list<std::vector<Time> > stoppingTimes;
             stoppingTimes.push_back(std::vector<Time>(1, c1->getTime()));
             stoppingTimes.push_back(c2->stoppingTimes());

             FdmStepConditionComposite::Conditions conditions;
             conditions.push_back(c1);
             conditions.push_back(c2);

             return boost::shared_ptr<FdmStepConditionComposite>(
                 new FdmStepConditionComposite(stoppingTimes, conditions));
        }
    }

    FdmHestonSolver::FdmHestonSolver(
        const Handle<HestonProcess>& process,
        const boost::shared_ptr<FdmMesher>& mesher,
        const FdmHestonSolver::BoundaryConditionSet& bcSet,
        const boost::shared_ptr<FdmStepConditionComposite> & condition,
        const boost::shared_ptr<Payoff>& payoff,
        const Time maturity,
        const Size timeSteps,
        FdmHestonSolver::FdmSchemeType schemeType, Real theta, Real mu)
    : process_(process),
      mesher_(mesher),
      bcSet_(bcSet),
      thetaCondition_(new FdmSnapshotCondition(
        0.99*std::min(1.0/365.0, 
                      condition->stoppingTimes().empty() ? maturity : 
                                 condition->stoppingTimes().front()))),
      condition_(addCondition(thetaCondition_, condition)),
      maturity_(maturity),
      timeSteps_(timeSteps),
      schemeType_(schemeType),
      theta_(theta),
      mu_(mu),
      initialValues_(mesher->layout()->size()),
      resultValues_(mesher->layout()->dim()[1], mesher->layout()->dim()[0]) {
        registerWith(process_);

        x_.reserve(mesher->layout()->dim()[0]);
        v_.reserve(mesher->layout()->dim()[1]);

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter; 
             ++iter) {
            initialValues_[iter.index()] = payoff->operator()(
                                        std::exp(mesher->location(iter, 0)));

            if (!iter.coordinates()[1]) {
                x_.push_back(mesher->location(iter, 0));
            }
            if (!iter.coordinates()[0]) {
                v_.push_back(mesher->location(iter, 1));
            }
        }
    }

    void FdmHestonSolver::performCalculations() const {
        boost::shared_ptr<FdmHestonOp> map(
                new FdmHestonOp(mesher_, process_.currentLink()));

        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        switch (schemeType_) {
          case HundsdorferScheme: 
            {
                QuantLib::HundsdorferScheme hsEvolver(theta_, mu_, map, bcSet_);
                FiniteDifferenceModel<QuantLib::HundsdorferScheme> hsModel(
                    hsEvolver, condition_->stoppingTimes());
                hsModel.rollback(rhs, maturity_, 0.0, timeSteps_, *condition_);
            }
            break;
          case DouglasScheme:
            {
                QuantLib::DouglasScheme dsEvolver(theta_, map, bcSet_);
                FiniteDifferenceModel<QuantLib::DouglasScheme> dsModel(
                    dsEvolver, condition_->stoppingTimes());
                dsModel.rollback(rhs, maturity_, 0.0, timeSteps_, *condition_);
            }
            break;
          case CraigSneydScheme:
            {
                QuantLib::CraigSneydScheme csEvolver(theta_, mu_, map, bcSet_);
                FiniteDifferenceModel<QuantLib::CraigSneydScheme> csModel(
                    csEvolver, condition_->stoppingTimes());
                csModel.rollback(rhs, maturity_, 0.0, timeSteps_, *condition_);
            }
            break;
          default:
            QL_FAIL("Unknown scheme type");
            break;
        }

        for (Size j=0; j < v_.size(); ++j)
            std::copy(rhs.begin()+j*x_.size(), rhs.begin()+(j+1)*x_.size(), 
                      resultValues_.row_begin(j));

        interpolation_ = boost::shared_ptr<BicubicSpline> (
            new BicubicSpline(x_.begin(), x_.end(), 
                              v_.begin(), v_.end(),
                              resultValues_));
    }

    Real FdmHestonSolver::valueAt(Real s, Real v) const {
        calculate();
        return interpolation_->operator()(std::log(s), v);
    }

    Real FdmHestonSolver::deltaAt(Real s, Real v, Real eps) const {
        return (valueAt(s+eps, v) - valueAt(s-eps, v))/(2*eps);
    }

    Real FdmHestonSolver::gammaAt(Real s, Real v, Real eps) const {
        return (valueAt(s+eps, v)+valueAt(s-eps, v)-2*valueAt(s,v))/(eps*eps);
    }

    Real FdmHestonSolver::thetaAt(Real s, Real v) const {
        QL_REQUIRE(condition_->stoppingTimes().front() > 0.0,
                   "stopping time at zero-> can't calculate theta");
                   
        calculate();
        Matrix thetaValues(resultValues_.rows(), resultValues_.columns());

        const Array& rhs = thetaCondition_->getValues();
        for (Size j=0; j < v_.size(); ++j)
            std::copy(rhs.begin()+j*x_.size(), rhs.begin()+(j+1)*x_.size(), 
                      thetaValues.row_begin(j));

        return (BicubicSpline(x_.begin(), x_.end(), v_.begin(), v_.end(),
                              thetaValues)(std::log(s), v) - valueAt(s, v))
              / thetaCondition_->getTime();
    }
}

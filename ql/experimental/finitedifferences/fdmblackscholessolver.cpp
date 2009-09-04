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

#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesop.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholessolver.hpp>

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

    FdmBlackScholesSolver::FdmBlackScholesSolver(
        const Handle<GeneralizedBlackScholesProcess>& process,
        const boost::shared_ptr<FdmMesher>& mesher,
        const FdmBlackScholesSolver::BoundaryConditionSet& bcSet,
        const boost::shared_ptr<FdmStepConditionComposite> & condition,
        const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
        Real strike,
        Time maturity,
        Size timeSteps,
        Size dampingSteps,
        Real theta,
        bool localVol,
        Real illegalLocalVolOverwrite)
    : process_(process),
      mesher_(mesher),
      bcSet_(bcSet),
      thetaCondition_(new FdmSnapshotCondition(
        0.99*std::min(1.0/365.0,
                      condition->stoppingTimes().empty() ? maturity :
                                 condition->stoppingTimes().front()))),
      condition_(addCondition(thetaCondition_, condition)),
      strike_(strike),
      maturity_(maturity),
      timeSteps_(timeSteps),
      dampingSteps_(dampingSteps),
      theta_(theta),
      localVol_(localVol),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite),
      initialValues_(mesher->layout()->size()),
      resultValues_(mesher->layout()->dim()[0]) {
        registerWith(process_);

        x_.reserve(mesher->layout()->dim()[0]);

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            initialValues_[iter.index()] = calculator->avgInnerValue(iter);
            x_.push_back(mesher->location(iter, 0));
        }
    }

    void FdmBlackScholesSolver::performCalculations() const {
        boost::shared_ptr<FdmBlackScholesOp> map(new FdmBlackScholesOp(
                mesher_, process_.currentLink(), strike_,
                localVol_, illegalLocalVolOverwrite_));

        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(map, bcSet_, condition_, 
                          FdmBackwardSolver::Douglas, theta_, 0.0)
            .rollback(rhs, maturity_, 0.0, timeSteps_, dampingSteps_);

        std::copy(rhs.begin(), rhs.end(), resultValues_.begin());

        interpolation_ = boost::shared_ptr<CubicInterpolation>(new
            MonotonicCubicNaturalSpline(x_.begin(), x_.end(),
                                        resultValues_.begin()));
    }

    Real FdmBlackScholesSolver::valueAt(Real s) const {
        calculate();
        return interpolation_->operator()(std::log(s));
    }

    Real FdmBlackScholesSolver::deltaAt(Real s) const {
        calculate();
        return interpolation_->derivative(std::log(s))/s;
    }

    Real FdmBlackScholesSolver::gammaAt(Real s) const {
        calculate();
        return (interpolation_->secondDerivative(std::log(s))
                -interpolation_->derivative(std::log(s)))/(s*s);
    }

    Real FdmBlackScholesSolver::thetaAt(Real s) const {
        QL_REQUIRE(condition_->stoppingTimes().front() > 0.0,
                   "stopping time at zero-> can't calculate theta");

        calculate();
        Array thetaValues(resultValues_.size());

        const Array& rhs = thetaCondition_->getValues();
        std::copy(rhs.begin(), rhs.end(), thetaValues.begin());

        Real temp = MonotonicCubicNaturalSpline(
            x_.begin(), x_.end(), thetaValues.begin())(std::log(s)); 
        return ( temp - valueAt(s) ) / thetaCondition_->getTime();
    }
}

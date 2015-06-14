/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file hestonslvmodel.hpp
    \brief Heston stochastic local volatility model
*/

#ifndef quantlib_heston_slv_model_hpp
#define quantlib_heston_slv_model_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/experimental/finitedifferences/fdmhestongreensfct.hpp>


namespace QuantLib {

class SimpleQuote;
    class HestonModel;
    class LocalVolTermStructure;

    struct HestonSLVFokkerPlanckFdmParams {
        const Date finalCalibrationMaturity;
        const Size xGrid, vGrid;
        const Size tMaxStepsPerYear, tMinStepsPerYear;
        const Real tStepNumberDecay;

        // local volatility forward equation
        const Real epsProbability;
        const Real undefinedlLocalVolOverwrite;
        const Size maxIntegrationIterations;

        // algorithm to get to the start configuration at time point one
        const FdmHestonGreensFct::Algorithm greensAlgorithm;
    };

    class HestonSLVModel : public LazyObject {
      public:
        HestonSLVModel(
            const Handle<LocalVolTermStructure>& localVol,
             const Handle<HestonModel>& hestonModel,
             const HestonSLVFokkerPlanckFdmParams& params,
            const std::vector<Date>& mandatoryDates = std::vector<Date>());

        void update();

        boost::shared_ptr<HestonProcess> hestonProcess() const;
        boost::shared_ptr<LocalVolTermStructure> localVol() const;
        boost::shared_ptr<LocalVolTermStructure> leverageFunction() const;

      protected:
        void performCalculations() const;

        const Handle<LocalVolTermStructure> localVol_;
        const Handle<HestonModel> hestonModel_;
        const HestonSLVFokkerPlanckFdmParams params_;
        const std::vector<Date> mandatoryDates_;

        mutable boost::shared_ptr<LocalVolTermStructure> leverageFunction_;
    };
}


#endif


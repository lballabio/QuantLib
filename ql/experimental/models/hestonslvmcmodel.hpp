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

/*! \file hestonslvmcmodel.hpp
    \brief Calibration of a Heston stochastic local volatility model based on MC
*/

#ifndef quantlib_heston_slv_mc_model_hpp
#define quantlib_heston_slv_mc_model_hpp

#include <ql/handle.hpp>
#include <ql/timegrid.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/marketmodels/browniangenerator.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>

namespace QuantLib {
    /*! References:

        Anthonie W. van der Stoep,Lech A. Grzelak, Cornelis W. Oosterlee, 2013,
        The Heston Stochastic-Local Volatility Model: Efficient Monte Carlo Simulation
        http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2278122
    */

    class HestonSLVMCModel : public LazyObject {
      public:
        HestonSLVMCModel(
            const Handle<LocalVolTermStructure>& localVol,
            const Handle<HestonModel>& hestonModel,
            const boost::shared_ptr<BrownianGeneratorFactory>& brownianGeneratorFactory,
            const Date& endDate,
            Size timeStepsPerYear = 365,
            Size nBins = 201,
            Size calibrationPaths = (1 << 15),
            const std::vector<Date>& mandatoryDates = std::vector<Date>());

        boost::shared_ptr<HestonProcess> hestonProcess() const;
        boost::shared_ptr<LocalVolTermStructure> localVol() const;
        boost::shared_ptr<LocalVolTermStructure> leverageFunction() const;

      protected:
        void performCalculations() const;

      private:
        const Handle<LocalVolTermStructure> localVol_;
        const Handle<HestonModel> hestonModel_;
        const boost::shared_ptr<BrownianGeneratorFactory> brownianGeneratorFactory_;
        const Date endDate_;
        const Size nBins_, calibrationPaths_;
        boost::shared_ptr<TimeGrid> timeGrid_;

        mutable boost::shared_ptr<FixedLocalVolSurface> leverageFunction_;
    };
}

#endif

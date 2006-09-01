/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file calibratedmarketmodel.hpp
    \brief calibrated market model
*/

#ifndef quantlib_calibrated_market_model_hpp
#define quantlib_calibrated_market_model_hpp

#include <ql/ShortRateModels/model.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmvolmodel.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmcorrmodel.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lfmcovarproxy.hpp>
#include <ql/MarketModels/marketmodel.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>

namespace QuantLib {

    //! Calibrate Market Model
    /*! References:

    */

    //class CalibratedMarketModel : public MarketModel,
    //                              public CalibratedModel {
    //  public:
    //    CalibratedMarketModel(
    //            const boost::shared_ptr<LmVolatilityModel>& volModel,
    //            const boost::shared_ptr<LmCorrelationModel>& corrModel,
    //            const EvolutionDescription& evolution,
    //            const Size numberOfFactors,
    //            const std::vector<Rate>& initialRates,
    //            const std::vector<Rate>& displacements);
    //    void setParams(const Array& params);
    //    //! \name MarketModel interface
    //    //@{
    //    const std::vector<Rate>& initialRates() const;
    //    const std::vector<Rate>& displacements() const;
    //    Size numberOfRates() const;
    //    Size numberOfFactors() const; 
    //    const Matrix& pseudoRoot(Size i) const;
    //    //@}
    //  protected:
    //    const boost::shared_ptr<LfmCovarianceProxy> covarProxy_;
    //  private:
    //    std::vector<Time> rateTimes_, evolutionTimes_;
    //    Size numberOfFactors_;
    //    std::vector<Rate> initialRates_;
    //    std::vector<Rate> displacements_;
    //    std::vector<Matrix> pseudoRoots_;
    //};

    //// inline

    //inline const std::vector<Rate>&
    //CalibratedMarketModel::initialRates() const {
    //    return initialRates_;
    //}

    //inline const std::vector<Rate>&
    //CalibratedMarketModel::displacements() const {
    //    return displacements_;
    //}

    //inline Size CalibratedMarketModel::numberOfRates() const {
    //    return initialRates_.size();
    //}

    //inline Size CalibratedMarketModel::numberOfFactors() const {
    //    return numberOfFactors_;
    //}

    //inline const Matrix& CalibratedMarketModel::pseudoRoot(Size i) const {
    //    return pseudoRoots_[i];
    //}

}

#endif

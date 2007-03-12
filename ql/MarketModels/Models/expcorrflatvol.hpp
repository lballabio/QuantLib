/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2006, 2007 StatPro Italia srl
 Copyright (C) 2006 Katiuscia Manzoni

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


#ifndef quantlib_exp_corr_flat_vol_hpp
#define quantlib_exp_corr_flat_vol_hpp

#include <ql/MarketModels/marketmodel.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/interpolation.hpp>
#include <ql/yieldtermstructure.hpp>
#include <ql/handle.hpp>
#include <vector>

namespace QuantLib {

    class ExpCorrFlatVol : public MarketModel {
      public:
        ExpCorrFlatVol(
            Real longTermCorr,
            Real beta,
            const std::vector<Volatility>& volatilities,
            const EvolutionDescription& evolution,
            Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Spread>& displacements);
        //! \name MarketModel interface
        //@{
        const std::vector<Rate>& initialRates() const;
        const std::vector<Spread>& displacements() const;
        const EvolutionDescription& evolution() const;
        Size numberOfRates() const;
        Size numberOfFactors() const;
        Size numberOfSteps() const;
        const Matrix& pseudoRoot(Size i) const;
        const Matrix& covariance(Size i) const;
        const Matrix& totalCovariance(Size endIndex) const;
        //@}
      private:
        Size numberOfFactors_, numberOfRates_, numberOfSteps_;
        std::vector<Rate> initialRates_;
        std::vector<Spread> displacements_;
        EvolutionDescription evolution_;
        std::vector<Matrix> pseudoRoots_, covariance_, totalCovariance_;
    };

    class ExpCorrFlatVolFactory : public MarketModelFactory,
                                  public Observer {
      public:
        ExpCorrFlatVolFactory(Real longTermCorr,
                              Real beta,
                              // this is just to make it work---it
                              // should be replaced with something
                              // else (such as some kind of volatility
                              // structure)
                              const std::vector<Time>& times,
                              const std::vector<Volatility>& vols,
                              // this is OK
                              const Handle<YieldTermStructure>& yieldCurve,
                              // this might have a structure
                              Spread displacement);
        boost::shared_ptr<MarketModel> create(const EvolutionDescription&,
                                              Size numberOfFactors) const;
        void update();
      private:
        Real longTermCorr_, beta_;
        // <to be changed>
        std::vector<Time> times_;
        std::vector<Volatility> vols_;
        Interpolation volatility_;
        // </to be changed>
        Handle<YieldTermStructure> yieldCurve_;
        Spread displacement_;
    };


    // inline definitions

    inline const std::vector<Rate>& ExpCorrFlatVol::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Spread>& ExpCorrFlatVol::displacements() const {
        return displacements_;
    }

    inline const EvolutionDescription& ExpCorrFlatVol::evolution() const {
        return evolution_;
    }

    inline Size ExpCorrFlatVol::numberOfRates() const {
        return initialRates_.size();
    }

    inline Size ExpCorrFlatVol::numberOfFactors() const {
        return numberOfFactors_;
    }

    inline Size ExpCorrFlatVol::numberOfSteps() const {
        return numberOfSteps_;
    }

    inline const Matrix& ExpCorrFlatVol::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

    inline const Matrix& ExpCorrFlatVol::covariance(Size i) const {
        return covariance_[i];
    }

    inline const Matrix& ExpCorrFlatVol::totalCovariance(Size endIndex) const {
        return totalCovariance_[endIndex];
    }

}

#endif

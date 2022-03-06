/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2006, 2007 StatPro Italia srl
 Copyright (C) 2006 Katiuscia Manzoni

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


#ifndef quantlib_exp_corr_flat_vol_hpp
#define quantlib_exp_corr_flat_vol_hpp

#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/handle.hpp>
#include <vector>

namespace QuantLib {

    class PiecewiseConstantCorrelation;

    class FlatVol : public MarketModel {
      public:
        FlatVol(
            const std::vector<Volatility>& volatilities,
            const ext::shared_ptr<PiecewiseConstantCorrelation>& corr,
            const EvolutionDescription& evolution,
            Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Spread>& displacements);
        //! \name MarketModel interface
        //@{
        const std::vector<Rate>& initialRates() const override;
        const std::vector<Spread>& displacements() const override;
        const EvolutionDescription& evolution() const override;
        Size numberOfRates() const override;
        Size numberOfFactors() const override;
        Size numberOfSteps() const override;
        const Matrix& pseudoRoot(Size i) const override;
        //@}
      private:
        Size numberOfFactors_, numberOfRates_, numberOfSteps_;
        std::vector<Rate> initialRates_;
        std::vector<Spread> displacements_;
        EvolutionDescription evolution_;
        std::vector<Matrix> pseudoRoots_;
    };

    class FlatVolFactory : public MarketModelFactory,
                                  public Observer {
      public:
        FlatVolFactory(Real longTermCorrelation,
                       Real beta,
                       // this is just to make it work---it
                       // should be replaced with something
                       // else (such as some kind of volatility
                       // structure)
                       std::vector<Time> times,
                       std::vector<Volatility> vols,
                       // this is OK
                       Handle<YieldTermStructure> yieldCurve,
                       // this might have a structure
                       Spread displacement);
        ext::shared_ptr<MarketModel> create(const EvolutionDescription&,
                                            Size numberOfFactors) const override;
        void update() override;

      private:
        Real longTermCorrelation_, beta_;
        // <to be changed>
        std::vector<Time> times_;
        std::vector<Volatility> vols_;
        Interpolation volatility_;
        // </to be changed>
        Handle<YieldTermStructure> yieldCurve_;
        Spread displacement_;
    };


    // inline definitions

    inline const std::vector<Rate>& FlatVol::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Spread>& FlatVol::displacements() const {
        return displacements_;
    }

    inline const EvolutionDescription& FlatVol::evolution() const {
        return evolution_;
    }

    inline Size FlatVol::numberOfRates() const {
        return initialRates_.size();
    }

    inline Size FlatVol::numberOfFactors() const {
        return numberOfFactors_;
    }

    inline Size FlatVol::numberOfSteps() const {
        return numberOfSteps_;
    }

    inline const Matrix& FlatVol::pseudoRoot(Size i) const {
        QL_REQUIRE(i<numberOfSteps_,
                   "the index " << i << " is invalid: it must be less than "
                   "number of steps (" << numberOfSteps_ << ")");
        return pseudoRoots_[i];
    }
}

#endif


#ifndef id_419e47d94a6c35c4f9fd25ed6d1b03c9
#define id_419e47d94a6c35c4f9fd25ed6d1b03c9
inline bool test_419e47d94a6c35c4f9fd25ed6d1b03c9(const int* i) {
    return i != nullptr;
}
#endif

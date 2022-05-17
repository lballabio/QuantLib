/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Ruilong Xu

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

/*! \file constparamprocess.hpp
    \brief constant parameter stochastic process for speeding up MC
*/

#ifndef quantlib_constant_parameter_process_hpp
#define quantlib_constant_parameter_process_hpp

#include <ql/experimental/processes/vegastressedblackscholesprocess.hpp>
#include <ql/processes/batesprocess.hpp>
#include <ql/processes/gjrgarchprocess.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/localconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/types.hpp>

namespace QuantLib {
    template <class BSM>
    class ConstParam : public BSM {
      public:
        template <class... ARGS>
        ConstParam(ARGS... args) : BSM(args...), fetched_(false) {
            isStrikeIndependent_ = true;
            forceDiscretization_ = false;
        }

        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real variance(Time t0, Real x0, Time dt) const override;
        Real evolve(Time t0, Real x0, Time dt, Real dw) const override;
        void update() override;

      private:
        mutable Real x0Value_;
        mutable Rate dividendYieldValue_, riskFreeRateValue_;
        mutable Spread carryCost_;
        mutable Volatility blackVolatilityValue_;
        mutable bool fetched_;

        void fetchValue() const;
    };

    template <class BSM>
    void ConstParam<BSM>::fetchValue() const {
        if (fetched_ == false) {
            ext::shared_ptr<FlatForward> flatDividendTS =
                ext::dynamic_pointer_cast<FlatForward>(dividendYield_.currentLink());
            QL_REQUIRE(flatDividendTS != nullptr,
                       "flat dividend yield term structure is required in ConstParam");
            QL_REQUIRE(flatDividendTS->compounding() != SimpleThenCompounded &&
                           flatDividendTS->compounding() != CompoundedThenSimple,
                       "dividend yield term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            ext::shared_ptr<FlatForward> flatRiskFreeTS =
                ext::dynamic_pointer_cast<FlatForward>(riskFreeRate_.currentLink());
            QL_REQUIRE(flatRiskFreeTS != nullptr,
                       "flat risk free rate term structure is required in ConstParam");
            QL_REQUIRE(flatRiskFreeTS->compounding() != SimpleThenCompounded &&
                           flatRiskFreeTS->compounding() != CompoundedThenSimple,
                       "risk free rate term structure without SimpleThenCompounded or "
                       "CompoundedThenSimple is required in ConstParam");

            ext::shared_ptr<BlackConstantVol> flatBlackVol =
                ext::dynamic_pointer_cast<BlackConstantVol>(blackVolatility_.currentLink());
            QL_REQUIRE(flatBlackVol != nullptr,
                       "flat volatility term structure is required in ConstParam");

            x0Value_ = x0_->value();

            static Real t1 = 0.0, t2 = 1.0;
            dividendYieldValue_ =
                flatDividendTS->forwardRate(t1, t2, Continuous, NoFrequency, true);
            riskFreeRateValue_ = flatRiskFreeTS->forwardRate(t1, t2, Continuous, NoFrequency, true);
            blackVolatilityValue_ = flatBlackVol->blackVol(0.0, x0_->value());

            carryCost_ = riskFreeRateValue_ - dividendYieldValue_;
            fetched_ = true;
        } else {
            return;
        }
    }

    template <class BSM>
    inline Real ConstParam<BSM>::x0() const {
        fetchValue();
        return x0Value_;
    }

    template <class BSM>
    inline Real ConstParam<BSM>::drift(Time t, Real x) const {
        fetchValue();
        return carryCost_ - 0.5 * blackVolatilityValue_ * blackVolatilityValue_;
    }

    template <class BSM>
    inline Real ConstParam<BSM>::diffusion(Time t, Real x) const {
        fetchValue();
        return blackVolatilityValue_;
    }

    template <class BSM>
    inline Real ConstParam<BSM>::expectation(Time t0, Real x0, Time dt) const {
        fetchValue();
        return x0 * std::exp(dt * carryCost_);
    }

    template <class BSM>
    inline Real ConstParam<BSM>::stdDeviation(Time t0, Real x0, Time dt) const {
        fetchValue();
        return blackVolatilityValue_ * std::sqrt(dt);
    }

    template <class BSM>
    inline Real ConstParam<BSM>::variance(Time t0, Real x0, Time dt) const {
        fetchValue();
        return blackVolatilityValue_ * blackVolatilityValue_ * dt;
    }

    template <class BSM>
    inline Real ConstParam<BSM>::evolve(Time t0, Real x0, Time dt, Real dw) const {
        fetchValue();
        Real var = blackVolatilityValue_ * blackVolatilityValue_ * dt;
        Real drift = carryCost_ * dt - 0.5 * var;
        return x0 * std::exp(std::sqrt(var) * dw + drift);
    }

    template <class BSM>
    inline void ConstParam<BSM>::update() {
        fetched_ = false;
        // update x0, dividendYield, riskFreeRate and blackVol
        fetchValue();
        BSM::update();
    }
}

namespace QuantLib {
    template <>
    class ConstParam<VegaStressedBlackScholesProcess> : public VegaStressedBlackScholesProcess {
      public:
        template <class... ARGS>
        ConstParam(ARGS... args) : VegaStressedBlackScholesProcess(args...), fetched_(false) {
            isStrikeIndependent_ = true;
            forceDiscretization_ = false;
        }

        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real variance(Time t0, Real x0, Time dt) const override;
        Real evolve(Time t0, Real x0, Time dt, Real dw) const override;
        void update() override;

      private:
        mutable Real x0Value_;
        mutable Rate dividendYieldValue_, riskFreeRateValue_;
        mutable Spread carryCost_;
        mutable Volatility blackVolatilityValue_;
        mutable bool fetched_;

        void fetchValue() const;
    };

    inline Real ConstParam<VegaStressedBlackScholesProcess>::x0() const {
        fetchValue();
        return x0Value_;
    }

    inline Real ConstParam<VegaStressedBlackScholesProcess>::drift(Time t, Real x) const {
        fetchValue();
        Real sigma = diffusion(t, x);
        return carryCost_ - 0.5 * sigma * sigma;
    }

    inline Real
    ConstParam<VegaStressedBlackScholesProcess>::expectation(Time t0, Real x0, Time dt) const {
        fetchValue();
        return x0 * std::exp(dt * carryCost_);
    }

    inline Real
    ConstParam<VegaStressedBlackScholesProcess>::stdDeviation(Time t0, Real x0, Time dt) const {
        fetchValue();
        return std::sqrt(variance(t0, x0, dt));
    }

    inline Real
    ConstParam<VegaStressedBlackScholesProcess>::variance(Time t0, Real x0, Time dt) const {
        fetchValue();
        Real sigma = blackVolatilityValue_;
        return sigma * sigma * dt;
    }

    inline Real
    ConstParam<VegaStressedBlackScholesProcess>::evolve(Time t0, Real x0, Time dt, Real dw) const {
        fetchValue();
        Real var = variance(t0, x0, dt);
        Real drift = carryCost_ * dt - 0.5 * var;
        return x0 * std::exp(std::sqrt(var) * dw + drift);
    }

    inline void ConstParam<VegaStressedBlackScholesProcess>::update() {
        fetched_ = false;
        // update x0, dividendYield, riskFreeRate and blackVol
        fetchValue();
        VegaStressedBlackScholesProcess::update();
    }
}

namespace QuantLib {
    template <>
    class ConstParam<HestonProcess> : public HestonProcess {
      public:
        template <class... ARGS>
        ConstParam(ARGS... args) : HestonProcess(args...), fetched_(false) {}

        void update() override {
            fetched_ = false;
            // update dividendYield, riskFreeRate and carryCost
            fetchValue();
            HestonProcess::update();
        }

        Spread forwardCarryCost(Time t1,
                                Time t2,
                                Compounding comp,
                                Frequency freq = Annual,
                                bool extrapolate = false) const override {
            fetchValue();
            return carryCost_;
        }

      private:
        mutable Rate dividendYieldValue_, riskFreeRateValue_;
        mutable Spread carryCost_;
        mutable bool fetched_;

        void fetchValue() const;
    };

    template <>
    class ConstParam<BatesProcess> : public BatesProcess {
      public:
        template <class... ARGS>
        ConstParam(ARGS... args) : BatesProcess(args...), fetched_(false) {}

        void update() override {
            fetched_ = false;
            // update dividendYield, riskFreeRate and carryCost
            fetchValue();
            BatesProcess::update();
        }

        Spread forwardCarryCost(Time t1,
                                Time t2,
                                Compounding comp,
                                Frequency freq = Annual,
                                bool extrapolate = false) const override {
            fetchValue();
            return carryCost_;
        }

      private:
        mutable Rate dividendYieldValue_, riskFreeRateValue_;
        mutable Spread carryCost_;
        mutable bool fetched_;

        void fetchValue() const;
    };
}

namespace QuantLib {
    template <>
    class ConstParam<GJRGARCHProcess> : public GJRGARCHProcess {
      public:
        template <class... ARGS>
        ConstParam(ARGS... args) : GJRGARCHProcess(args...), fetched_(false) {}

        void update() override {
            fetched_ = false;
            // update dividendYield, riskFreeRate and carryCost
            fetchValue();
            GJRGARCHProcess::update();
        }

        Spread forwardCarryCost(Time t1,
                                Time t2,
                                Compounding comp,
                                Frequency freq = Annual,
                                bool extrapolate = false) const override {
            fetchValue();
            return carryCost_;
        }

      private:
        mutable Rate dividendYieldValue_, riskFreeRateValue_;
        mutable Spread carryCost_;
        mutable bool fetched_;

        void fetchValue() const;
    };
}
#endif
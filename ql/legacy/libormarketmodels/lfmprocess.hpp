/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

/*! \file lfmprocess.hpp
    \brief stochastic process of a libor forward model
*/

#ifndef quantlib_libor_forward_model_process_hpp
#define quantlib_libor_forward_model_process_hpp

#include <ql/cashflow.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/legacy/libormarketmodels/lfmcovarparam.hpp>

namespace QuantLib {

    //! libor-forward-model process
    /*! stochastic process of a libor forward model using the
        rolling forward measure incl. predictor-corrector step

        References:

        Glasserman, Paul, 2004, Monte Carlo Methods in Financial Engineering,
        Springer, Section 3.7

        Antoon Pelsser, 2000, Efficient Methods for Valuing Interest Rate
        Derivatives, Springer, 8

        Hull, John, White, Alan, 1999, Forward Rate Volatilities, Swap Rate
        Volatilities and the Implementation of the Libor Market Model
        (<http://www.rotman.utoronto.ca/~amackay/fin/libormktmodel2.pdf>)

        \test the correctness is tested by Monte-Carlo reproduction of
              caplet & ratchet NPVs and comparison with Black pricing.

        \warning this class does not work correctly with Visual C++ 6.

        \ingroup processes
    */
    class LiborForwardModelProcess : public StochasticProcess {
      public:
        LiborForwardModelProcess(Size size, std::shared_ptr<IborIndex> index);

        Array initialValues() const override;
        Array drift(Time t, const Array& x) const override;
        Matrix diffusion(Time t, const Array& x) const override;
        Matrix covariance(Time t0, const Array& x0, Time dt) const override;
        Array apply(const Array& x0, const Array& dx) const override;

        // implements the predictor-corrector schema
        Array evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;

        Size size() const override;
        Size factors() const override;

        std::shared_ptr<IborIndex> index() const;
        Leg cashFlows(
                                                     Real amount = 1.0) const;

        void setCovarParam(
               const std::shared_ptr<LfmCovarianceParameterization>& param);
        std::shared_ptr<LfmCovarianceParameterization> covarParam() const;

        // convenience support methods
        Size nextIndexReset(Time t) const;
        const std::vector<Time> & fixingTimes() const;
        const std::vector<Date> & fixingDates() const;
        const std::vector<Time> & accrualStartTimes() const;
        const std::vector<Time> & accrualEndTimes() const;

        std::vector<DiscountFactor> discountBond(
                                       const std::vector<Rate> & rates) const;

      private:
        Size size_;

        const std::shared_ptr<IborIndex> index_;
        std::shared_ptr<LfmCovarianceParameterization> lfmParam_;

        Array initialValues_;

        std::vector<Time> fixingTimes_;
        std::vector<Date> fixingDates_;
        std::vector<Time> accrualStartTimes_;
        std::vector<Time> accrualEndTimes_;
        std::vector<Time> accrualPeriod_;

        mutable Array m1, m2;
    };

}


#endif

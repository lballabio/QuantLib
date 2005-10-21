/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file capletlmmprocess.hpp
    \brief stochastic process of a (cap) libor market model
*/

#ifndef quantlib_caplet_libor_market_model_process_hpp
#define quantlib_caplet_libor_market_model_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/capvolstructures.hpp>
#include <ql/Indexes/xibor.hpp>
#include <map>

namespace QuantLib {

    //! caplet libor-market-model process
    /*! stochastic process of a (cap) libor market model using the
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
              caplet & ratchet npvs and comparison with Black pricing.
    */
    class CapletLiborMarketModelProcess : public StochasticProcess {
      public:
        /*!
          \param fixings             number of rate fixing
          \param underlyingIndex     underlying Libor index

          \param capletVol cap volatility term structure. Used to
                           bootstrap volatilities \f$ \Lambda_i \f$ of
                           \f$ F_i \f$.

          \param volatilityComponents \f$ \lambda_{i,q}/\Lambda_i \f$,
                     the ratio of the \f$ q \f$-th component of the
                     volatility of the forward rate to the total
                     volatility of the forward rate. The number of
                     columns of this matrix defines the number of
                     factors of the model.
        */
        CapletLiborMarketModelProcess(
            Size fixings,
            const boost::shared_ptr<Xibor>& underlyingIndex,
            const boost::shared_ptr<CapletVolatilityStructure>& capletVol,
            const Matrix& volatilityComponents = Matrix());

        Size size() const;
        Size factors() const;

        Disposable<Array>  initialValues() const;
        Disposable<Array>  drift(Time t, const Array& x) const;
        Disposable<Matrix> diffusion(Time t, const Array& x) const;
        Disposable<Array>  apply(const Array& x0, const Array& dx) const;

        // implements the predictor-corrector schema
        Disposable<Array>  evolve(Time t0, const Array& x0,
                                  Time dt, const Array& dw) const;

        std::vector<Time> fixingTimes() const;
        Time              accrualPeriod(Size i) const;

        /*! volatility matrix \f$ \lambda_{i,j} \f$, i-th fixing, j-th
            volatility factor, see equation 20 in Hull White paper */
        Volatility lambda(Size i, Size j=0) const;

        //! discount factor until the j-th fixing period
        DiscountFactor discountBond(const std::vector<Rate>& rates,
                                    Size j) const;

      protected:
        Size nextResetDate(Time t) const;

      private:
        Size len_;
        Size factors_;
        Matrix lambda_, lambdaSquare_;

        boost::shared_ptr<Xibor> index_;

        Array initialValues_;

        std::vector<Time> delta_;
        std::vector<Date> fixingDates_;
        std::vector<Time> accrualPeriod_;
        std::map <Time, Size, std::less<Time> > m_;
    };

}


#endif

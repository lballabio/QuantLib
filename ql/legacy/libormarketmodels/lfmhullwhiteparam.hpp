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

/*! \file lfmhullwhiteparam.hpp
    \brief libor market model parameterization based on Hull White
*/

#ifndef quantlib_libor_market_hull_white_parameterization_hpp
#define quantlib_libor_market_hull_white_parameterization_hpp

#include <ql/legacy/libormarketmodels/lfmprocess.hpp>
#include <ql/legacy/libormarketmodels/lfmcovarparam.hpp>

namespace QuantLib {

    //! %Libor market model parameterization based on Hull White paper
    /*! Hull, John, White, Alan, 1999, Forward Rate Volatilities, Swap Rate
        Volatilities and the Implementation of the Libor Market Model
        (<http://www.rotman.utoronto.ca/~amackay/fin/libormktmodel2.pdf>)

        \test the correctness is tested by Monte-Carlo reproduction of
              caplet & ratchet npvs and comparison with Black pricing.
    */

    class LfmHullWhiteParameterization : public LfmCovarianceParameterization {
      public:
        LfmHullWhiteParameterization(
            const std::shared_ptr<LiborForwardModelProcess> & process,
            const std::shared_ptr<OptionletVolatilityStructure> & capletVol,
            const Matrix& correlation = Matrix(), Size factors = 1);

        Matrix diffusion(Time t, const Array& x = Null<Array>()) const override;
        Matrix covariance(Time t, const Array& x = Null<Array>()) const override;
        Matrix integratedCovariance(Time t, const Array& x = Null<Array>()) const override;

      protected:
        Size nextIndexReset(Time t) const;

        Matrix diffusion_, covariance_;
        std::vector<Time> fixingTimes_;
    };

}


#endif


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen

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

/*! \file hybridhestonhullwhiteprocess.hpp
    \brief hybrid equity (heston model) 
           with stochastic interest rates (hull white model)
*/

#ifndef quantlib_hybrid_heston_hull_white_process_hpp
#define quantlib_hybrid_heston_hull_white_process_hpp

#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/processes/jointstochasticprocess.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>

namespace QuantLib {
    //! Hybrid Heston Hull-White stochastic process
    /*! This class implements a three factor Heston Hull-White model

        \bug This class was not tested enough to guarantee
             its functionality... work in progress

        \ingroup processes
    */
    class HybridHestonHullWhiteProcess : public JointStochasticProcess {
      public:
        HybridHestonHullWhiteProcess(
          const boost::shared_ptr<HestonProcess> & hestonProcess,
          const boost::shared_ptr<HullWhiteForwardProcess> & hullWhiteProcess,
          Real corrEquityShortRate,
          Size factors);

        void preEvolve(Time t0, const Array& x0,
                       Time dt, const Array& dw) const;
        Disposable<Array> postEvolve(Time t0, const Array& x0,
                                     Time dt, const Array& dw,
                                     const Array& y0) const;

        DiscountFactor numeraire(Time t, const Array& x) const;
        bool correlationIsStateDependend() const;
        Disposable<Matrix> crossModelCorrelation(Time t0, 
                                                 const Array& x0) const;

        void update();

      private:
        //model is used to calculate P(t,T)
        const boost::shared_ptr<HullWhite> hullWhiteModel_;

        const Real corrEquityShortRate_;
        const Time T_;
        DiscountFactor endDiscount_;
    };

}


#endif

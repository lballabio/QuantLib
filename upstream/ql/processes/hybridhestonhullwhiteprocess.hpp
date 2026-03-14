/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

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
    class HybridHestonHullWhiteProcess : public StochasticProcess {
      public:
        enum Discretization { Euler, BSMHullWhite };

        HybridHestonHullWhiteProcess(
          const ext::shared_ptr<HestonProcess> & hestonProcess,
          const ext::shared_ptr<HullWhiteForwardProcess> & hullWhiteProcess,
          Real corrEquityShortRate,
          Discretization discretization = BSMHullWhite);

        Size size() const override;
        Array initialValues() const override;
        Array drift(Time t, const Array& x) const override;
        Matrix diffusion(Time t, const Array& x) const override;
        Array apply(const Array& x0, const Array& dx) const override;

        Array evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;

        DiscountFactor numeraire(Time t, const Array& x) const;

        const ext::shared_ptr<HestonProcess>& hestonProcess() const;
        const ext::shared_ptr<HullWhiteForwardProcess>& hullWhiteProcess() const;

        Real eta() const;
        Time time(const Date& date) const override;
        Discretization discretization() const;
        void update() override;

      protected:
        const ext::shared_ptr<HestonProcess> hestonProcess_;
        const ext::shared_ptr<HullWhiteForwardProcess> hullWhiteProcess_;
        
        //model is used to calculate P(t,T)
        const ext::shared_ptr<HullWhite> hullWhiteModel_;

        const Real corrEquityShortRate_;
        const Discretization discretization_;
        const Real maxRho_;
        const Time T_;
        DiscountFactor endDiscount_;
    };

}


#endif

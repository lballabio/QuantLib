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

/*! \file hestonslvprocess.hpp
    \brief Heston stochastic local volatility process
*/

#ifndef quantlib_heston_slv_process_hpp
#define quantlib_heston_slv_process_hpp

#include <ql/processes/hestonprocess.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    class HestonSLVProcess : public StochasticProcess {
      public:
        HestonSLVProcess(
            const ext::shared_ptr<HestonProcess>& hestonProcess,
            const ext::shared_ptr<LocalVolTermStructure>& leverageFct);

        Size size()    const { return Size(2); }
        Size factors() const { return Size(2); }

        void update();

        Disposable<Array> initialValues() const {
            return hestonProcess_->initialValues();
        }
        Disposable<Array> apply(const Array& x0, const Array& dx) const {
            return hestonProcess_->apply(x0, dx);
        }

        Disposable<Array> drift(Time t, const Array& x) const;
        Disposable<Matrix> diffusion(Time t, const Array& x) const;
        Disposable<Array> evolve(Time t0, const Array& x0,
                                 Time dt, const Array& dw) const;

        Real v0()    const { return v0_; }
        Real rho()   const { return rho_; }
        Real kappa() const { return kappa_; }
        Real theta() const { return theta_; }
        Real sigma() const { return sigma_; }
        ext::shared_ptr<LocalVolTermStructure> leverageFct() const {
            return leverageFct_;
        }

        const Handle<Quote>& s0() const { return hestonProcess_->s0(); }
        const Handle<YieldTermStructure>& dividendYield() const {
            return hestonProcess_->dividendYield();
        }
        const Handle<YieldTermStructure>& riskFreeRate() const {
            return hestonProcess_->riskFreeRate();
        }

        Time time(const Date& d) const { return hestonProcess_->time(d); }

      private:
        Real kappa_, theta_, sigma_, rho_, v0_;

        const ext::shared_ptr<HestonProcess> hestonProcess_;
        const ext::shared_ptr<LocalVolTermStructure> leverageFct_;
    };
}
#endif

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/processes/klugeextouprocess.hpp>
#include <ql/experimental/processes/extouwithjumpsprocess.hpp>
#include <ql/experimental/processes/extendedornsteinuhlenbeckprocess.hpp>

namespace QuantLib {

    KlugeExtOUProcess::KlugeExtOUProcess(
        Real rho,
        const boost::shared_ptr<ExtOUWithJumpsProcess>& klugeProcess,
        const boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess>& ouProcess)
    : rho_         (rho),
      sqrtMRho_    (std::sqrt(1-rho*rho)),
      klugeProcess_(klugeProcess),
      ouProcess_   (ouProcess) {
    }

    Size KlugeExtOUProcess::size() const {
        return klugeProcess_->size() + 1;
    }

    Size KlugeExtOUProcess::factors() const {
        return klugeProcess_->factors() + 1;
    }

    Disposable<Array> KlugeExtOUProcess::initialValues() const {
        Array retVal(size());
        const Array x0 = klugeProcess_->initialValues();
        std::copy(x0.begin(), x0.end(), retVal.begin());
        retVal.back() = ouProcess_->x0();

        return retVal;
    }

    Disposable<Array> KlugeExtOUProcess::drift(Time t, const Array& x) const {
        Array retVal(size());
        Array mu = klugeProcess_->drift(t, x);
        std::copy(mu.begin(), mu.end(), retVal.begin());
        retVal.back() = ouProcess_->drift(t, x.back());

        return retVal;
    }

    Disposable<Matrix> KlugeExtOUProcess::diffusion(Time t, const Array& x)
        const{
        Matrix retVal(size(), factors(), 0.0);

        Volatility vol = ouProcess_->diffusion(t, x.back());

        retVal[0][0]              = klugeProcess_->diffusion(t, x)[0][0];
        retVal[size()][0]         = rho_*vol;
        retVal[size()][factors()] = sqrtMRho_*vol;

        return retVal;
    }

    Disposable<Array> KlugeExtOUProcess::evolve(Time t0, const Array& x0,
                                                Time dt, const Array& dw) const{
        Array retVal(size());

        Array ev = klugeProcess_->evolve(t0, x0, dt, dw);
        std::copy(ev.begin(), ev.end(), retVal.begin());

        const Real dz = dw.back()*sqrtMRho_ + dw.front()*rho_;
        retVal.back() = ouProcess_->evolve(t0, x0.back(), dt, dz);

        return retVal;
    }

    boost::shared_ptr<ExtOUWithJumpsProcess>
        KlugeExtOUProcess::getKlugeProcess() const {
        return klugeProcess_;
    }
    boost::shared_ptr<ExtendedOrnsteinUhlenbeckProcess>
        KlugeExtOUProcess::getExtOUProcess() const {
        return ouProcess_;
    }

    Real KlugeExtOUProcess::rho() const {
        return rho_;
    }

}


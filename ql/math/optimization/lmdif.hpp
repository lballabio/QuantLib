/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2015 Peter Caspers

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

/*! \file lmdif.hpp
    \brief wrapper for MINPACK minimization routine
*/

#ifndef quantlib_optimization_lmdif_hpp
#define quantlib_optimization_lmdif_hpp

#include <ql/types.hpp>
#include <ql/functional.hpp>

namespace QuantLib::MINPACK {
        typedef std::function<void (int,
                                      int, 
                                      Real*,
                                      Real*,
                                      int*)> LmdifCostFunction;

        void lmdif(int m,int n,Real* x,Real* fvec,Real ftol,
                   Real xtol,Real gtol,int maxfev,Real epsfcn,
                   Real* diag, int mode, Real factor,
                   int nprint, int* info,int* nfev,Real* fjac,
                   int ldfjac,int* ipvt,Real* qtf,
                   Real* wa1,Real* wa2,Real* wa3,Real* wa4,
                   const LmdifCostFunction& fcn,
                   const LmdifCostFunction& jacFcn);

        void qrsolv(int n,
                    Real* r,
                    int ldr,
                    const int* ipvt,
                    const Real* diag,
                    const Real* qtb,
                    Real* x,
                    Real* sdiag,
                    Real* wa);
        void qrfac(int m,int n,Real* a,int, int pivot,int* ipvt,
                   int,Real* rdiag,Real* acnorm,Real* wa);
    }
#endif

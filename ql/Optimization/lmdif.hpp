/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

/*! \file lmdif.hpp
    \brief wrapper for MINPACK minimization routine
*/

#ifndef quantlib_optimization_lmdif_hpp
#define quantlib_optimization_lmdif_hpp

namespace QuantLib {

    namespace MINPACK {

        void lmdif(int m,int n,double* x,double* fvec,double ftol,
                   double xtol,double gtol,int maxfev,double epsfcn,
                   double* diag, int mode, double factor,
                   int nprint, int* info,int* nfev,double* fjac,
                   int ldfjac,int* ipvt,double* qtf,
                   double* wa1,double* wa2,double* wa3,double* wa4);
    }

}


#endif

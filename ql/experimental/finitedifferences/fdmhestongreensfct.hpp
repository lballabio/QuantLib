/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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

/*! \file fdmhestongreensfct.hpp
    \brief Heston Fokker-Planck Green's function
*/

#ifndef quantlib_fdm_heston_greens_fct_hpp
#define quantlib_fdm_heston_greens_fct_hpp

#include <ql/experimental/finitedifferences/fdmsquarerootfwdop.hpp>

namespace QuantLib {

    class HestonProcess;
    class FdmHestonGreensFct {
      public:
        enum Algorithm { ZeroCorrelation, Gaussian, SemiAnalytical };

        FdmHestonGreensFct(ext::shared_ptr<FdmMesher> mesher,
                           ext::shared_ptr<HestonProcess> process,
                           FdmSquareRootFwdOp::TransformationType trafoType_,
                           Real l0 = 1.0);

        Disposable<Array> get(Time t, Algorithm algorithm) const;

      private:
        const Real l0_;
        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<HestonProcess> process_;
        const FdmSquareRootFwdOp::TransformationType trafoType_;
    };
}

#endif


#ifndef id_620fda968b53701d610a0eeaddfa19e2
#define id_620fda968b53701d610a0eeaddfa19e2
inline bool test_620fda968b53701d610a0eeaddfa19e2(const int* i) {
    return i != nullptr;
}
#endif

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

/*! \file levenbergmarquardt.hpp
    \brief Levenberg-Marquardt optimization method
*/

#ifndef quantlib_optimization_levenberg_marquardt_hpp
#define quantlib_optimization_levenberg_marquardt_hpp

#include <ql/Patterns/singleton.hpp>
#include <ql/Optimization/problem.hpp>

namespace QuantLib {

    //! Levenberg-Marquardt optimization method
    /*! This implementation is based on MINPACK
        (<http://www.netlib.org/minpack>,
        <http://www.netlib.org/cephes/linalg.tgz>)
    */
    class LevenbergMarquardt : public OptimizationMethod {
      public:
        LevenbergMarquardt(Real epsfcn = 1.0e-8,
                           Real xtol = 1.0e-8,
                           Real gtol = 1.0e-8);
        virtual EndCriteria::Type minimize(Problem& P,
                                           const EndCriteria& endCriteria //= EndCriteria()
                                           );
                                           //      = EndCriteria(400, 1.0e-8, 1.0e-8)
        virtual Integer getInfo() const;
        static void fcn(int m,
                        int n,
                        double* x,
                        double* fvec,
                        int* iflag);
      private:
        // class is needed to make the Levenberg-Marquardt
        // algorithm sessionId() safe (or multi threading safe).
        class ProblemData : public Singleton<ProblemData> {
          public:
            Problem* & problem() { return thisP_; }
            Array& initCostValues()    { return initCostValues_; }
          private:
            Problem* thisP_;
            Array initCostValues_;
        };

        mutable Integer info_;

        const Real epsfcn_, xtol_, gtol_;
    };

}


#endif

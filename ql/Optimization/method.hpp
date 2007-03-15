/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2007 François du Vignaud

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

/*! \file method.hpp
    \brief Abstract optimization method class
*/

#ifndef quantlib_optimization_method_h
#define quantlib_optimization_method_h

#include <boost/timer.hpp>
#include <ql/Utilities/null.hpp>
#include <ql/Optimization/constraint.hpp>
#include <ql/Optimization/costfunction.hpp>
#include <ql/Optimization/endcriteria.hpp>

namespace QuantLib {

    class Problem;

    //! Abstract class for constrained optimization method
    class OptimizationMethod {
      public:
        virtual ~OptimizationMethod() {}

        //! minimize the optimization problem P
        virtual EndCriteria::Type minimize(Problem& P,
                                           const EndCriteria& endCriteria //= EndCriteria()
                                           ) = 0;
        std::vector<Real> performance(){return performance_;}

      protected:
        void startTimer() { timer_.restart(); }
        void stopTimer() {
            performance_ = std::vector<Real>(3,0.);
            Real seconds = timer_.elapsed();
            int hours = int(seconds/3600);
            seconds -= hours * 3600;
            int minutes = int(seconds/60);
            seconds -= minutes * 60;
            performance_[0] = hours;
            performance_[1] = minutes;
            performance_[2] = seconds;
        }

        std::vector<Real> performance_;
        boost::timer timer_;
        
    };

}

#endif

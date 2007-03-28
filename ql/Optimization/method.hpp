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

    inline std::string secondsToString(Real elapsed) {
            Integer seconds = static_cast<Integer>(elapsed);
            Integer hours = seconds/3600;
            seconds -= hours * 3600;
            Integer minutes = seconds/60;
            seconds -= minutes * 60;
            std::ostringstream out;
            out << hours << ":" << minutes << ":" << seconds;
            return out.str();
    }

    //! Abstract class for constrained optimization method
    class OptimizationMethod {
      public:
        virtual ~OptimizationMethod() {}

        //! minimize the optimization problem P
        virtual EndCriteria::Type minimize(Problem& P,
                                           const EndCriteria& endCriteria) = 0;
        Real elapsed() { return elapsed_; }
      protected:
        void startTimer() { timer_.restart(); }
        void stopTimer() { elapsed_ = timer_.elapsed(); }

        boost::timer timer_;
        Real elapsed_;
    };

}

#endif

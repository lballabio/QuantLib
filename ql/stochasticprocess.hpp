
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy_ of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file stochasticprocess.hpp
    \brief Stochastic process

    \fullpath
    ql/%stochasticprocess.hpp
*/

// $Id$

#ifndef quantlib_stochastic_process_h
#define quantlib_stochastic_process_h

namespace QuantLib {
    //describes a process goverved by dx = \mu(t, x)dt + \sigma(t, x)dW_t
    class StochasticProcess {
      public:
        enum Variable { ShortRate, LogShortRate };
        StochasticProcess(Variable variable) : variable_(variable) {}
        virtual double drift(double t, double x) const = 0;
        virtual double diffusion(double t, double x) const = 0;
        virtual Variable variable() { return variable_; }
      private:
        Variable variable_;
    };
}

#endif

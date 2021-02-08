/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Banca Profilo S.p.A.

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

/*! \file forwardmeasureprocess.hpp
    \brief forward-measure stochastic processes
*/

#ifndef quantlib_forward_measure_processes_hpp
#define quantlib_forward_measure_processes_hpp

#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    //! forward-measure stochastic process
    /*! stochastic process whose dynamics are expressed in the forward
        measure.

        \ingroup processes
    */
    class ForwardMeasureProcess : public StochasticProcess {
      public:
        virtual void setForwardMeasureTime(Time);
        Time getForwardMeasureTime() const;
      protected:
        ForwardMeasureProcess() = default;
        explicit ForwardMeasureProcess(Time T) : T_(T) {}
        explicit ForwardMeasureProcess(
                                   const ext::shared_ptr<discretization>&);
        Time T_;
    };

    //! forward-measure 1-D stochastic process
    /*! 1-D stochastic process whose dynamics are expressed in the
        forward measure.

        \ingroup processes
    */
    class ForwardMeasureProcess1D : public StochasticProcess1D {
      public:
        virtual void setForwardMeasureTime(Time);
        Time getForwardMeasureTime() const;
      protected:
        ForwardMeasureProcess1D() = default;
        explicit ForwardMeasureProcess1D(Time T) : T_(T) {}
        explicit ForwardMeasureProcess1D(
                                    const ext::shared_ptr<discretization>&);
        Time T_;
    };

}


#endif



/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file diffusionprocess.hpp
    \brief Diffusion process

    \fullpath
    ql/%diffusionprocess.hpp
*/

// $Id$

#ifndef quantlib_diffusion_process_h
#define quantlib_diffusion_process_h

#include <ql/qldefines.hpp>

namespace QuantLib {

    //describes a process goverved by dx = \mu(t, x)dt + \sigma(t, x)dW_t
    class DiffusionProcess {
      public:
        virtual double drift(double t, double x) const = 0;
        virtual double diffusion(double t, double x) const = 0;
    };

}


#endif



/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file shortrateprocess.hpp
    \brief Short rate process

    \fullpath
    ql/InterestRateModelling/%shortrateprocess.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_short_rate_process_h
#define quantlib_interest_rate_modelling_short_rate_process_h

#include <ql/diffusionprocess.hpp>
#include <ql/types.hpp>

namespace QuantLib {

    //describes a process followed by the short rate or a function of it
    class ShortRateProcess : public DiffusionProcess {
      public:
        virtual double variable(Time t, Rate r) const = 0;
        virtual Rate shortRate(Time t, double variable) const = 0;
    };

}


#endif

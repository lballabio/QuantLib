/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file singlepathcontrolvariatedpricer.h
    
    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/01/05 11:02:37  lballabio
    Renamed SinglePathPricer to PathPricer

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.
                
*/

#ifndef quantlib_montecarlo_single_path_control_variated_pricer_h
#define quantlib_montecarlo_single_path_control_variated_pricer_h

#include "qldefines.h"
#include "handle.h"
#include "pathpricer.h"

namespace QuantLib {

    namespace MonteCarlo {

        class SinglePathControlVariatedPricer: public PathPricer {
        public:
            SinglePathControlVariatedPricer() : PathPricer() {}
            SinglePathControlVariatedPricer(Handle<PathPricer > pricer,
                Handle<PathPricer > controlVariate, double controlVariateValue);
            double value(const Path &path) const;
        private:
            Handle<PathPricer> pricer_, controlVariate_;
            double controlVariateValue_;
        };

    }

}


#endif

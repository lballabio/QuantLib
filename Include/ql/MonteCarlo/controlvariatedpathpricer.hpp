
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
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file controlvariatedpathpricer.hpp 
    \brief generic control variated path pricer

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_montecarlo_control_variated_path_pricer_h
#define quantlib_montecarlo_control_variated_path_pricer_h

#include "ql/handle.hpp"
#include "ql/MonteCarlo/pathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {
        
        //! generic control variated path pricer
        /*! ControlVariatedPathPricer combines two PathPricer and a value into a
            control variated path pricer. To be used with McPricer, see
            McAsianPricer for an example.
        */
        class ControlVariatedPathPricer: public PathPricer {
          public:
            ControlVariatedPathPricer() {}
            ControlVariatedPathPricer(Handle<PathPricer> pricer,
                Handle<PathPricer> controlVariate, double controlVariateValue);
            double value(const Path &path) const;
          private:
            Handle<PathPricer> pricer_, controlVariate_;
            double controlVariateValue_;
        };

    }

}


#endif


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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/
/*! \file mcoptionsample.hpp

    \fullpath
    Include/ql/MonteCarlo/%mcoptionsample.hpp
    \brief Create a sample generator from a path generator and a path pricer

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:56:11  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.15  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.14  2001/08/22 11:18:06  nando
// removed unused default constructor
// added a few typedef for argument type and value type
//
// Revision 1.13  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.12  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.11  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.10  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.9  2001/07/20 13:06:57  marmar
// Monte Carlo interfaces imporved
//
// Revision 1.8  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.7  2001/07/13 14:29:08  sigmud
// removed a few gcc compile warnings
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_montecarlo_option_sample_h
#define quantlib_montecarlo_option_sample_h

#include "ql/handle.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! %Option value sampler
        /*! Given a path generator class PG, together with an instance 
            "samplePath", and a path pricer PP, again with an instance 
            "pathpricer", a sample generator OptionSample<PG, PP> returns, 
            at each next(), a value for the option price.
    
            Minimal interfaces for PG and PP:
            \code
            class PG {
                typedef ... SampleType;
                SampleType next() const;
                double weight() const;
            };
        
            class PP {
                typedef ... ValueType;
                typedef ... PathType;
                ValueType value(PathType &) const;
            };
            \endcode
            Also, PG::SampleType must be equal or convertible into PP::PathType.
        */
        template<class PG, class PP>
        class OptionSample {
          public:
            typedef typename PP::ValueType SampleType;

            OptionSample(){}
            OptionSample(Handle<PG> samplePath, Handle<PP> pathPricer);
            SampleType next() const; 
            double weight() const;
          private:
            mutable Handle<PG> samplePath_;
            Handle<PP> pathPricer_;
            mutable double weight_;
        };

        // inline definitions
        
        template<class PG, class PP>
        inline OptionSample<PG, PP>::OptionSample(Handle<PG> samplePath,
               Handle<PP> pathPricer): samplePath_(samplePath),
               pathPricer_(pathPricer), weight_(0){}

        template<class PG, class PP>
        inline OptionSample<PG, PP>::SampleType OptionSample<PG, PP>::next() const {
            typename PG::SampleType a = samplePath_->next();
            double price = pathPricer_->value(a);
            weight_ = samplePath_->weight();
            return price;
        }

        template<class PG, class PP>
        inline double OptionSample<PG, PP>::weight() const{
            return weight_;
        }

    }

}
#endif

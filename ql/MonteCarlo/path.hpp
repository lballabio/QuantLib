
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
/*! \file path.hpp
    \brief Monte Carlo path

    \fullpath
    ql/MonteCarlo/%path.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_path_h
#define quantlib_montecarlo_path_h

#include "ql/array.hpp"
#include "ql/handle.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! single random walk
        class Path {
          public:
            Path(unsigned int size);
            Path(const Array& drift, const Array& randomComponent);
            //! \name inspectors
            //@{
            double operator[](int i) const;
            unsigned int size() const;
            //@}
            //! \name read/write access to components
            //@{
            const Array& drift() const;
            Array& drift();
            const Array& randomComponent() const;
            Array& randomComponent();
            //@}
          private:
            Array drift_;
            Array randomComponent_;
        };

        // inline definitions

        inline Path::Path(unsigned int size)
        : drift_(size), randomComponent_(size) {}
        
        inline Path::Path(const Array& drift, const Array& randomComponent)
        : drift_(drift), randomComponent_(randomComponent) {
            QL_REQUIRE(drift_.size() == randomComponent_.size(),
                "Path: drift and random components have different size");
        }

        inline double Path::operator[](int i) const { 
            return drift_[i] + randomComponent_[i]; 
        }
        
        inline unsigned int Path::size() const {
            return drift_.size(); 
        }

        inline const Array& Path::drift() const { 
            return drift_; 
        }

        inline Array& Path::drift() { 
            return drift_; 
        }

        inline const Array& Path::randomComponent() const {
            return randomComponent_; 
        }
        
        inline Array& Path::randomComponent() {
            return randomComponent_; 
        }
        
    }

}


#endif

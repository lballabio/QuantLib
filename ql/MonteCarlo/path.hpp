
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

#include <ql/array.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! single factor random walk
        /*! \todo make it time-aware
        */
        class Path {
          public:
            Path(size_t size);
            Path(const Array& drift, const Array& diffusion);
            //! \name inspectors
            //@{
            double operator[](int i) const;
            size_t size() const;
            //@}
            //! \name read/write access to components
            //@{
            const Array& drift() const;
            Array& drift();
            const Array& diffusion() const;
            Array& diffusion();
            //@}
          private:
            Array drift_;
            Array diffusion_;
        };

        // inline definitions

        inline Path::Path(size_t size)
        : drift_(size), diffusion_(size) {}

        inline Path::Path(const Array& drift, const Array& diffusion)
        : drift_(drift), diffusion_(diffusion) {
            QL_REQUIRE(drift_.size() == diffusion_.size(),
                "Path: drift and diffusion have different size");
        }

        inline double Path::operator[](int i) const {
            return drift_[i] + diffusion_[i];
        }

        inline size_t Path::size() const {
            return drift_.size();
        }

        inline const Array& Path::drift() const {
            return drift_;
        }

        inline Array& Path::drift() {
            return drift_;
        }

        inline const Array& Path::diffusion() const {
            return diffusion_;
        }

        inline Array& Path::diffusion() {
            return diffusion_;
        }

    }

}


#endif

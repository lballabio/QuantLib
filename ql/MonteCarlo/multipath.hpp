
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
/*! \file multipath.hpp
    \brief Multiple paths evolving at the same time

    \fullpath
    ql/MonteCarlo/%multipath.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_multi_path_h
#define quantlib_montecarlo_multi_path_h

#include "ql/MonteCarlo/path.hpp"
#include <vector>

namespace QuantLib {

    namespace MonteCarlo {

        /*!
        \typedef MultiPath
        \brief MultiPath implements multiple paths evolving at the same time

        MultiPath contains the list of variations for each asset,
        \f[
            \log \frac{Y^j_{i+1}}{Y^j_i} \mathrm{for} i = 0, \ldots, n-1
            \qquad \mathrm{and} \qquad j = 0, \ldots, m-1
        \f]
        where \f$ Y^j_i \f$ is the value of the underlying \f$ j \f$
        at discretized time \f$ t_i \f$. The first index refers to the
        underlying, the second to the time position MultiPath[j,i]
        */
//        typedef QuantLib::Math::Matrix MultiPath;

        //! single random walk
        class MultiPath {
          public:
            MultiPath(unsigned int nAsset,
                      unsigned int pathSize);
            MultiPath(const std::vector<Path>& multiPath);
            //! \name inspectors
            //@{
            const Path& operator[](int j) const {return multiPath_[j]; }
            Path& operator[](int j) {return multiPath_[j]; }
            unsigned int assetNumber() const {return multiPath_.size(); }
            unsigned int pathSize() const {return multiPath_[0].size(); }
            //@}
          private:
            std::vector<Path> multiPath_;
        };

        // inline definitions

        inline MultiPath::MultiPath(unsigned int nAsset, unsigned int pathSize)
            : multiPath_(nAsset,Path(pathSize)) {
            QL_REQUIRE(nAsset > 0,
                "MultiPath: number of asset must be > zero");
            QL_REQUIRE(pathSize > 0,
                "MultiPath: pathSize must be > zero");
        }

        inline MultiPath::MultiPath(const std::vector<Path>& multiPath)
            : multiPath_(multiPath) {}

    
    }

}


#endif

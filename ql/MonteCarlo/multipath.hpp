
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file multipath.hpp
    \brief Correlated multiple asset paths
*/

#ifndef quantlib_montecarlo_multi_path_h
#define quantlib_montecarlo_multi_path_h

#include <ql/MonteCarlo/path.hpp>

namespace QuantLib {

    //! Correlated multiple asset paths
    /*! MultiPath contains the list of variations for each asset,
        \f[
            \log \frac{Y^j_{i+1}}{Y^j_i} \mathrm{for} i = 0, \ldots, n-1
            \qquad \mathrm{and} \qquad j = 0, \ldots, m-1
        \f]
        where \f$ Y^j_i \f$ is the value of the underlying \f$ j \f$
        at discretized time \f$ t_i \f$. The first index refers to the
        underlying, the second to the time position MultiPath[j,i]
        
        \todo rename as MultiAssetPath

        \ingroup mcarlo
    */
    class MultiPath {
      public:
        MultiPath(Size nAsset,
                  const TimeGrid& timeGrid);
        MultiPath(const std::vector<Path>& multiPath);
        //! \name inspectors
        //@{
        Size assetNumber() const { return multiPath_.size(); }
        Size pathSize() const { return multiPath_[0].size(); }
        //@}
        //! \name read/write access to components
        //@{
        const Path& operator[](Size j) const {return multiPath_[j]; }
        Path& operator[](Size j) {return multiPath_[j]; }
        //@}
      private:
        std::vector<Path> multiPath_;
    };


    // inline definitions

    inline MultiPath::MultiPath(Size nAsset, const TimeGrid& timeGrid)
    : multiPath_(nAsset,Path(timeGrid)) {
        QL_REQUIRE(nAsset > 0, "number of asset must be positive");
    }

    inline MultiPath::MultiPath(const std::vector<Path>& multiPath)
    : multiPath_(multiPath) {}

}


#endif

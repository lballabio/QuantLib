
/*
  Copyright (C) 2005 Joseph Wang

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

#include <ql/FiniteDifferences/bsmtermoperator.hpp>
#include <ql/Utilities/tracing.hpp>

namespace QuantLib {

    BSMTermOperator::BSMTermOperator(
                        const Array& grid,
                        const boost::shared_ptr<BlackScholesProcess>& process,
                        Time residualTime)
    : TridiagonalOperator(grid.size()) {
        timeSetter_ = boost::shared_ptr<TimeSetter>(
                                               new TimeSetter(grid, process));
        setTime(residualTime);
    }

    BSMTermOperator::TimeSetter::TimeSetter(
                        const Array& grid,
                        const boost::shared_ptr<BlackScholesProcess>& process)
    : priceGrid_(grid),
      logPriceGrid_(Log(priceGrid_)),
      dxp_(grid.size()),
      dxm_(grid.size()),
      process_(process) {
        for (Size i=1; i < logPriceGrid_.size() -1 ; i++) {
            dxm_[i] = logPriceGrid_[i] - logPriceGrid_[i-1];
            dxp_[i] = logPriceGrid_[i+1] - logPriceGrid_[i];
        }
    }

    void BSMTermOperator::TimeSetter::setTime(Time t,
                                              TridiagonalOperator& L) const {
        if (std::fabs(t) < 1e-8) t = 0;
        Real r = process_->riskFreeRate()->forwardRate(t,t,Continuous);
        Real q = process_->dividendYield()->forwardRate(t,t,Continuous);
        QL_TRACE("BSMTermOperator::TimeSetter " << r << " " << q );
        for (Size i=1; i < logPriceGrid_.size() - 1; i++) {
            Real sigma =
                process_->localVolatility()->localVol(t, priceGrid_[i]);
            QL_TRACE("Sigma: " << i << sigma);
            Real sigma2 = sigma * sigma;
            Real nu = r-q-sigma2/2;
            Real pd = -(sigma2/dxm_[i]-nu)/(dxm_[i] + dxp_[i]);
            Real pu = -(sigma2/dxp_[i]+nu)/(dxm_[i] + dxp_[i]);
            Real pm = sigma2/(dxm_[i] * dxp_[i])+r;
            L.setMidRow(i, pd,pm,pu);
        }
    }

}


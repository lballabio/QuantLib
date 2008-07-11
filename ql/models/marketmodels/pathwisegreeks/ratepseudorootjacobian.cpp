/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

Copyright (C) 2008 Mark Joshi

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/



#include <ql/models/marketmodels/pathwisegreeks/ratepseudorootjacobian.hpp>

namespace QuantLib
{


    RatePseudoRootJacobianNumerical::RatePseudoRootJacobianNumerical(const Matrix& pseudoRoot,
        Size aliveIndex, 
        Size numeraire,
        const std::vector<Time>& taus,
        const std::vector<Matrix>& pseudoBumps,
        const std::vector<Spread>& displacements)
        :
    pseudoRoot_(pseudoRoot),        
        aliveIndex_(aliveIndex), 
        taus_(taus),
        displacements_(displacements),
        numberBumps_(pseudoBumps.size()),
        drifts_(taus.size()),
        bumpedRates_(taus.size()),
        factors_(pseudoRoot.columns())
    {
        Size numberRates= taus.size();

        QL_REQUIRE(pseudoRoot_.rows()==numberRates,
            "pseudoRoot_.rows()<> taus.size()");

        QL_REQUIRE(displacements_.size()==numberRates,
            "displacements_.size()<> taus.size()");

        QL_REQUIRE(drifts_.size()==numberRates,
            "drifts_.size()<> taus.size()");

        for (Size i=0; i < pseudoBumps.size(); ++i)
        {
            QL_REQUIRE(pseudoBumps[i].rows()==numberRates,
                "pseudoBumps[i].rows()<> taus.size() with i =" << i);

            QL_REQUIRE(pseudoBumps[i].columns()==factors_,
                "pseudoBumps[i].columns()<> factors with i = " << i);


            Matrix pseudo(pseudoRoot_);
            pseudo += pseudoBumps[i];
            pseudoBumped_.push_back(pseudo);
            driftsComputers_.push_back(LMMDriftCalculator(pseudo,
                displacements,
                taus,
                numeraire,
                aliveIndex));
        }

    }


    void RatePseudoRootJacobianNumerical::getBumps(const std::vector<Rate>& oldRates,
        const std::vector<Real>& , // not used in the numerical implementation 
        const std::vector<Rate>& newRates,   
        const std::vector<Real>& gaussians,
        Matrix& B)
    {
        Size numberRates = taus_.size();

        QL_REQUIRE(B.rows()==numberBumps_,
            "B.rows()<> numberBumps_");

        QL_REQUIRE(B.columns()==taus_.size(),
            "B.columns()<> number of rates");

        for (Size i =0; i < numberBumps_; ++i)
        {
            const Matrix& pseudo = pseudoBumped_[i];
            driftsComputers_[i].compute(oldRates,
                drifts_); 

            for (Size j =0; j < aliveIndex_; ++j)
                B[i][j]=0.0;

            for (Size j=aliveIndex_; j < numberRates; ++j)
            {
                bumpedRates_[j] = log(oldRates[j]+displacements_[j]);

                for (Size k=0; k < factors_; ++k)
                    bumpedRates_[j] += -0.5*pseudo[j][k]*pseudo[j][k];

                bumpedRates_[j] +=drifts_[j];

                for (Size k=0; k < factors_; ++k)
                    bumpedRates_[j] += pseudo[j][k]*gaussians[k];

                bumpedRates_[j] =exp(bumpedRates_[j]);
                bumpedRates_[j] -= displacements_[j];
                Real tmp = bumpedRates_[j] - newRates[j];

                B[i][j] =  tmp;
            }
        }

    }

    RatePseudoRootJacobian::RatePseudoRootJacobian(const Matrix& pseudoRoot,
        Size aliveIndex, 
        Size numeraire,
        const std::vector<Time>& taus,
        const std::vector<Matrix>& pseudoBumps,
        const std::vector<Spread>& displacements)
        :
    pseudoRoot_(pseudoRoot),        
        aliveIndex_(aliveIndex), 
        taus_(taus),
        pseudoBumps_(pseudoBumps),
        displacements_(displacements),
        numberBumps_(pseudoBumps.size()),
        bumpedRates_(taus.size()),
        factors_(pseudoRoot.columns()),
        e_(pseudoRoot.rows(), pseudoRoot.columns()),
        ratios_(taus_.size())
    {
        Size numberRates= taus.size();

        QL_REQUIRE(aliveIndex == numeraire,
            "we can do only do discretely compounding MM acount so aliveIndex must equal numeraire");

        QL_REQUIRE(pseudoRoot_.rows()==numberRates,
            "pseudoRoot_.rows()<> taus.size()");

        QL_REQUIRE(displacements_.size()==numberRates,
            "displacements_.size()<> taus.size()");


        for (Size i=0; i < pseudoBumps.size(); ++i)
        {
            QL_REQUIRE(pseudoBumps[i].rows()==numberRates,
                "pseudoBumps[i].rows()<> taus.size() with i =" << i);

            QL_REQUIRE(pseudoBumps[i].columns()==factors_,
                "pseudoBumps[i].columns()<> factors with i = " << i);




        }

        for (Size i=0; i < numberRates; ++i)
        {
            allDerivatives_.push_back(Matrix(numberRates,factors_));
        }


    }


    void RatePseudoRootJacobian::getBumps(const std::vector<Rate>& oldRates,
        const std::vector<Real>& discountRatios, // not used in the numerical implementation 
        const std::vector<Rate>& newRates,   
        const std::vector<Real>& gaussians,
        Matrix& B)
    {
        Size numberRates = taus_.size();

        for (Size j=aliveIndex_; j < numberRates; ++j)
            ratios_[j] = (oldRates[j] + displacements_[j])*discountRatios[j+1];

        for (Size f=0; f < factors_; ++f)
        {
            e_[aliveIndex_][f] = 0;

            for (Size j= aliveIndex_+1; j < numberRates; ++j)
                e_[j][f] = e_[j-1][f] + ratios_[j-1]*pseudoRoot_[j-1][f];
        }


        for (Size f=0; f < factors_; ++f)
            for (Size j=aliveIndex_; j < numberRates; ++j)
            {
                for (Size k= aliveIndex_; k < j ; ++k)
                    allDerivatives_[j][k][f] = newRates[j]*ratios_[k]*taus_[k]*pseudoRoot_[j][f];

                // GG don't seem to have the 2, this term is miniscule in any case
                Real tmp = //2*
                    2*ratios_[j]*taus_[j]*pseudoRoot_[j][f];
                tmp -=  pseudoRoot_[j][f];
                tmp += e_[j][f]*taus_[j];
                tmp += gaussians[f];
                tmp *= (newRates[j]+displacements_[j]);


                allDerivatives_[j][j][f] =tmp;

                for (Size k= j+1; k < numberRates ; ++k)
                    allDerivatives_[j][k][f]=0.0;


            }



            for (Size i =0; i < numberBumps_; ++i)
            {
                Size j=0;

                for (; j < aliveIndex_; ++j)
                {
                    B[i][j]=0.0;        
                }
                for (; j < numberRates; ++j)
                {
                    Real sum =0.0;

                    for (Size k=aliveIndex_; k < numberRates; ++k)
                        for (Size f=0; f < factors_; ++f)
                            sum += pseudoBumps_[i][k][f]*allDerivatives_[j][k][f];
                    B[i][j] =sum;

                }      

            }

    }
}



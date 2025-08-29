/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/randomnumbers/knuthuniformrng.hpp>
#include <ql/math/randomnumbers/seedgenerator.hpp>

namespace QuantLib {

    const int KnuthUniformRng::KK = 100;
    const int KnuthUniformRng::LL = 37;
    const int KnuthUniformRng::TT = 70;
    const int KnuthUniformRng::QUALITY = 1009;

    KnuthUniformRng::KnuthUniformRng(long seed)
    : ranf_arr_buf(QUALITY), ran_u(QUALITY) {
        ranf_arr_ptr = ranf_arr_sentinel = ranf_arr_buf.size();
        ranf_start(seed != 0 ? seed : SeedGenerator::instance().get());
    }

    void KnuthUniformRng::ranf_start(long seed) {
        int t,s,j;
        std::vector<double> u(KK+KK-1),ul(KK+KK-1);
        double ulp=(1.0/(1L<<30))/(1L<<22);                // 2 to the -52
        double ss=2.0*ulp*((seed&0x3fffffff)+2);

        for (j=0;j<KK;j++) {
            u[j]=ss; ul[j]=0.0;                    // bootstrap the buffer
            ss+=ss; if (ss>=1.0) ss-=1.0-2*ulp; // cyclic shift of 51 bits
        }
        for (;j<KK+KK-1;j++) u[j]=ul[j]=0.0;
        u[1]+=ulp;ul[1]=ulp;            // make u[1] (and only u[1]) "odd"
        s=seed&0x3fffffff;
        t=TT-1;
        while (t != 0) {
            for (j=KK-1;j>0;--j) ul[j+j]=ul[j],u[j+j]=u[j];    // "square"
            for (j=KK+KK-2;j>KK-LL;j-=2)
                ul[KK+KK-1-j]=0.0,u[KK+KK-1-j]=u[j]-ul[j];
            for (j=KK+KK-2;j>=KK;--j)
                if (ul[j] != 0.0) {
                    ul[j - (KK - LL)] = ulp - ul[j - (KK - LL)],
                                 u[j - (KK - LL)] = mod_sum(u[j - (KK - LL)], u[j]);
                    ul[j - KK] = ulp - ul[j - KK], u[j - KK] = mod_sum(u[j - KK], u[j]);
            }
            if (is_odd(s)) {                            // "multiply by z"
                for (j=KK;j>0;--j)  ul[j]=ul[j-1],u[j]=u[j-1];
                ul[0]=ul[KK],u[0]=u[KK];    // shift the buffer cyclically
                if (ul[KK] != 0.0)
                    ul[LL] = ulp - ul[LL], u[LL] = mod_sum(u[LL], u[KK]);
            }
            if (s != 0)
                s >>= 1;
            else
                t--;
        }
        for (j=0;j<LL;j++) ran_u[j+KK-LL]=u[j];
        for (;j<KK;j++) ran_u[j-LL]=u[j];
    }

    void KnuthUniformRng::ranf_array(std::vector<double>& aa,
                                     int n) const {
        int i,j;
        for (j=0;j<KK;j++) aa[j]=ran_u[j];
        for (;j<n;j++) aa[j]=mod_sum(aa[j-KK],aa[j-LL]);
        for (i=0;i<LL;i++,j++) ran_u[i]=mod_sum(aa[j-KK],aa[j-LL]);
        for (;i<KK;i++,j++) ran_u[i]=mod_sum(aa[j-KK],ran_u[i-LL]);
    }

    double KnuthUniformRng::ranf_arr_cycle() const {
        ranf_array(ranf_arr_buf,QUALITY);
        ranf_arr_ptr = 1;
        ranf_arr_sentinel = 100;
        return ranf_arr_buf[0];
    }

}

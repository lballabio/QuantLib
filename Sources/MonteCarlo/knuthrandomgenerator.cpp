
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

/*! \file knuthrandomgenerator.cpp

    \fullpath
    Sources/MonteCarlo/%knuthrandomgenerator.cpp
    \brief Knuth uniform random number generator

*/

// $Id$
// $Log$
// Revision 1.10  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.9  2001/08/30 17:24:37  nando
// clock substitued by time
//
// Revision 1.8  2001/08/30 16:51:08  nando
// clock substitued by time
//
// Revision 1.7  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.6  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.5  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.4  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.3  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.2  2001/07/04 08:05:15  uid37043
// Worked around Visual C++ being off-standard again
//
// Revision 1.1  2001/07/03 13:19:38  lballabio
// Added Knuth random generator after doubts were casted on the NR one
//

#include "ql/MonteCarlo/knuthrandomgenerator.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        const int KnuthRandomGenerator::KK = 100;
        const int KnuthRandomGenerator::LL = 37;
        const int KnuthRandomGenerator::TT = 70;
        const int KnuthRandomGenerator::QUALITY = 1009;

        KnuthRandomGenerator::KnuthRandomGenerator(long seed)
        : ranf_arr_buf(QUALITY), ran_u(QUALITY) {
            ranf_arr_ptr = ranf_arr_sentinel = ranf_arr_buf.end();
            ranf_start(seed != 0 ? seed : long(QL_TIME(0)));
        }
        
        void KnuthRandomGenerator::ranf_start(long seed) {
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
            t=TT-1; while (t) {
                for (j=KK-1;j>0;j--) ul[j+j]=ul[j],u[j+j]=u[j];    // "square"
                for (j=KK+KK-2;j>KK-LL;j-=2)
                    ul[KK+KK-1-j]=0.0,u[KK+KK-1-j]=u[j]-ul[j];
                for (j=KK+KK-2;j>=KK;j--) if(ul[j]) {
                    ul[j-(KK-LL)]=ulp-ul[j-(KK-LL)],
                    u[j-(KK-LL)]=mod_sum(u[j-(KK-LL)],u[j]);
                    ul[j-KK]=ulp-ul[j-KK],u[j-KK]=mod_sum(u[j-KK],u[j]);
                }
                if (is_odd(s)) {                            // "multiply by z"
                    for (j=KK;j>0;j--)  ul[j]=ul[j-1],u[j]=u[j-1];
                    ul[0]=ul[KK],u[0]=u[KK];    // shift the buffer cyclically
                    if (ul[KK]) ul[LL]=ulp-ul[LL],u[LL]=mod_sum(u[LL],u[KK]);
                }
                if (s) s>>=1; else t--;
            }
            for (j=0;j<LL;j++) ran_u[j+KK-LL]=u[j];
            for (;j<KK;j++) ran_u[j-LL]=u[j];
        }
        
        void KnuthRandomGenerator::ranf_array(std::vector<double>& aa, 
          int n) const {
            int i,j;
            for (j=0;j<KK;j++) aa[j]=ran_u[j];
            for (;j<n;j++) aa[j]=mod_sum(aa[j-KK],aa[j-LL]);
            for (i=0;i<LL;i++,j++) ran_u[i]=mod_sum(aa[j-KK],aa[j-LL]);
            for (;i<KK;i++,j++) ran_u[i]=mod_sum(aa[j-KK],ran_u[i-LL]);
        }
        
        double KnuthRandomGenerator::ranf_arr_cycle() const {
            ranf_array(ranf_arr_buf,QUALITY);
            ranf_arr_ptr=ranf_arr_buf.begin()+1;
            return ranf_arr_buf[0];
        }
        
    }

}


/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano

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

#include <ql/RandomNumbers/sobolrsg.hpp>
#include <ql/RandomNumbers/primitivepolynomials.h>
#include <ql/RandomNumbers/mt19937uniformrng.hpp>
#include <ql/basicdataformatters.hpp>
#include <iostream>

namespace QuantLib {

    namespace {

        /* Sobol' Levitan coefficients of the free direction integers as given
           by Bratley, P., Fox, B.L. (1988)
        */
        const unsigned long dim02SLinitializers[] = {
            1UL, 0UL };
        const unsigned long dim03SLinitializers[] = {
            1UL, 1UL, 0UL };
        const unsigned long dim04SLinitializers[] = {
            1UL, 3UL, 7UL, 0UL };
        const unsigned long dim05SLinitializers[] = {
            1UL, 1UL, 5UL, 0UL };
        const unsigned long dim06SLinitializers[] = {
            1UL, 3UL, 1UL, 1UL, 0UL };
        const unsigned long dim07SLinitializers[] = {
            1UL, 1UL, 3UL, 7UL, 0UL };
        const unsigned long dim08SLinitializers[] = {
            1UL, 3UL, 3UL, 9UL, 9UL, 0UL };
        const unsigned long dim09SLinitializers[] = {
            1UL, 3UL, 7UL, 13UL, 3UL, 0UL };
        const unsigned long dim10SLinitializers[] = {
            1UL, 1UL, 5UL, 11UL, 27UL, 0UL };
        const unsigned long dim11SLinitializers[] = {
            1UL, 3UL, 5UL, 1UL, 15UL, 0UL };
        const unsigned long dim12SLinitializers[] = {
            1UL, 1UL, 7UL, 3UL, 29UL, 0UL };
        const unsigned long dim13SLinitializers[] = {
            1UL, 3UL, 7UL, 7UL, 21UL, 0UL };
        const unsigned long dim14SLinitializers[] = {
            1UL, 1UL, 1UL, 9UL, 23UL, 37UL, 0UL };
        const unsigned long dim15SLinitializers[] = {
            1UL, 3UL, 3UL, 5UL, 19UL, 33UL, 0UL };
        const unsigned long dim16SLinitializers[] = {
            1UL, 1UL, 3UL, 13UL, 11UL, 7UL, 0UL };
        const unsigned long dim17SLinitializers[] = {
            1UL, 1UL, 7UL, 13UL, 25UL, 5UL, 0UL };
        const unsigned long dim18SLinitializers[] = {
            1UL, 3UL, 5UL, 11UL, 7UL, 11UL, 0UL };
        const unsigned long dim19SLinitializers[] = {
            1UL, 1UL, 1UL, 3UL, 13UL, 39UL, 0UL };
        const unsigned long dim20SLinitializers[] = {
            1UL, 3UL, 1UL, 15UL, 17UL, 63UL, 13UL, 0UL };
        const unsigned long dim21SLinitializers[] = {
            1UL, 1UL, 5UL, 5UL, 1UL, 27UL, 33UL, 0UL };
        const unsigned long dim22SLinitializers[] = {
            1UL, 3UL, 3UL, 3UL, 25UL, 17UL, 115UL, 0UL };
        const unsigned long dim23SLinitializers[] = {
            1UL, 1UL, 3UL, 15UL, 29UL, 15UL, 41UL, 0UL };
        const unsigned long dim24SLinitializers[] = {
            1UL, 3UL, 1UL, 7UL, 3UL, 23UL, 79UL, 0UL };
        const unsigned long dim25SLinitializers[] = {
            1UL, 3UL, 7UL, 9UL, 31UL, 29UL, 17UL, 0UL };
        const unsigned long dim26SLinitializers[] = {
            1UL, 1UL, 5UL, 13UL, 11UL, 3UL, 29UL, 0UL };
        const unsigned long dim27SLinitializers[] = {
            1UL, 3UL, 1UL, 9UL, 5UL, 21UL, 119UL, 0UL };
        const unsigned long dim28SLinitializers[] = {
            1UL, 1UL, 3UL, 1UL, 23UL, 13UL, 75UL, 0UL };
        const unsigned long dim29SLinitializers[] = {
            1UL, 3UL, 3UL, 11UL, 27UL, 31UL, 73UL, 0UL };
        const unsigned long dim30SLinitializers[] = {
            1UL, 1UL, 7UL, 7UL, 19UL, 25UL, 105UL, 0UL };
        const unsigned long dim31SLinitializers[] = {
            1UL, 3UL, 5UL, 5UL, 21UL, 9UL, 7UL, 0UL };
        const unsigned long dim32SLinitializers[] = {
            1UL, 1UL, 1UL, 15UL, 5UL, 49UL, 59UL, 0UL };
        const unsigned long dim33SLinitializers[] = {
            1UL, 1UL, 1UL, 1UL, 1UL, 33UL, 65UL, 0UL };
        const unsigned long dim34SLinitializers[] = {
            1UL, 3UL, 5UL, 15UL, 17UL, 19UL, 21UL, 0UL };
        const unsigned long dim35SLinitializers[] = {
            1UL, 1UL, 7UL, 11UL, 13UL, 29UL, 3UL, 0UL };
        const unsigned long dim36SLinitializers[] = {
            1UL, 3UL, 7UL, 5UL, 7UL, 11UL, 113UL, 0UL };
        const unsigned long dim37SLinitializers[] = {
            1UL, 1UL, 5UL, 3UL, 15UL, 19UL, 61UL, 0UL };
        const unsigned long dim38SLinitializers[] = {
            1UL, 3UL, 1UL, 1UL, 9UL, 27UL, 89UL, 7UL, 0UL };
        const unsigned long dim39SLinitializers[] = {
            1UL, 1UL, 3UL, 7UL, 31UL, 15UL, 45UL, 23UL, 0UL };
        const unsigned long dim40SLinitializers[] = {
            1UL, 3UL, 3UL, 9UL, 9UL, 25UL, 107UL, 39UL, 0UL };

        const unsigned long * const SLinitializers[39] = {
            dim02SLinitializers,
            dim03SLinitializers,
            dim04SLinitializers,
            dim05SLinitializers,
            dim06SLinitializers,
            dim07SLinitializers,
            dim08SLinitializers,
            dim09SLinitializers,
            dim10SLinitializers,
            dim11SLinitializers,
            dim12SLinitializers,
            dim13SLinitializers,
            dim14SLinitializers,
            dim15SLinitializers,
            dim16SLinitializers,
            dim17SLinitializers,
            dim18SLinitializers,
            dim19SLinitializers,
            dim20SLinitializers,
            dim21SLinitializers,
            dim22SLinitializers,
            dim23SLinitializers,
            dim24SLinitializers,
            dim25SLinitializers,
            dim26SLinitializers,
            dim27SLinitializers,
            dim28SLinitializers,
            dim29SLinitializers,
            dim30SLinitializers,
            dim31SLinitializers,
            dim32SLinitializers,
            dim33SLinitializers,
            dim34SLinitializers,
            dim35SLinitializers,
            dim36SLinitializers,
            dim37SLinitializers,
            dim38SLinitializers,
            dim39SLinitializers,
            dim40SLinitializers
        };

        /* coefficients of the free direction integers as given in
           "Monte Carlo Methods in Finance", by Peter Jäckel, section 8.3
        */
        const unsigned long dim09initializers[] = {
            1UL, 3UL, 7UL, 7UL, 21UL, 0UL };
        const unsigned long dim10initializers[] = {
            1UL, 1UL, 5UL, 11UL, 27UL, 0UL };
        const unsigned long dim11initializers[] = {
            1UL, 1UL, 7UL, 3UL, 29UL, 0UL };
        const unsigned long dim12initializers[] = {
            1UL, 3UL, 7UL, 13UL, 3UL, 0UL };
        const unsigned long dim13initializers[] = {
            1UL, 3UL, 5UL, 1UL, 15UL, 0UL };
        const unsigned long dim14initializers[] = {
            1UL, 1UL, 1UL, 9UL, 23UL, 37UL, 0UL };
        const unsigned long dim15initializers[] = {
            1UL, 1UL, 3UL, 13UL, 11UL, 7UL, 0UL };
        const unsigned long dim16initializers[] = {
            1UL, 3UL, 3UL, 5UL, 19UL, 33UL, 0UL };
        const unsigned long dim17initializers[] = {
            1UL, 1UL, 7UL, 13UL, 25UL, 5UL, 0UL };
        const unsigned long dim18initializers[] = {
            1UL, 1UL, 1UL, 3UL, 13UL, 39UL, 0UL };
        const unsigned long dim19initializers[] = {
            1UL, 3UL, 5UL, 11UL, 7UL, 11UL, 0UL };
        const unsigned long dim20initializers[] = {
            1UL, 3UL, 1UL, 7UL, 3UL, 23UL, 79UL, 0UL };
        const unsigned long dim21initializers[] = {
            1UL, 3UL, 1UL, 15UL, 17UL, 63UL, 13UL, 0UL };
        const unsigned long dim22initializers[] = {
            1UL, 3UL, 3UL, 3UL, 25UL, 17UL, 115UL, 0UL };
        const unsigned long dim23initializers[] = {
            1UL, 3UL, 7UL, 9UL, 31UL, 29UL, 17UL, 0UL };
        const unsigned long dim24initializers[] = {
            1UL, 1UL, 3UL, 15UL, 29UL, 15UL, 41UL, 0UL };
        const unsigned long dim25initializers[] = {
            1UL, 3UL, 1UL, 9UL, 5UL, 21UL, 119UL, 0UL };
        const unsigned long dim26initializers[] = {
            1UL, 1UL, 5UL, 5UL, 1UL, 27UL, 33UL, 0UL };
        const unsigned long dim27initializers[] = {
            1UL, 1UL, 3UL, 1UL, 23UL, 13UL, 75UL, 0UL };
        const unsigned long dim28initializers[] = {
            1UL, 1UL, 7UL, 7UL, 19UL, 25UL, 105UL, 0UL };
        const unsigned long dim29initializers[] = {
            1UL, 3UL, 5UL, 5UL, 21UL, 9UL, 7UL, 0UL };
        const unsigned long dim30initializers[] = {
            1UL, 1UL, 1UL, 15UL, 5UL, 49UL, 59UL, 0UL };
        const unsigned long dim31initializers[] = {
            1UL, 3UL, 5UL, 15UL, 17UL, 19UL, 21UL, 0UL };
        const unsigned long dim32initializers[] = {
            1UL, 1UL, 7UL, 11UL, 13UL, 29UL, 3UL, 0UL };

        const unsigned long * const initializers[31] = {
            dim02SLinitializers,
            dim03SLinitializers,
            dim04SLinitializers,
            dim05SLinitializers,
            dim06SLinitializers,
            dim07SLinitializers,
            dim08SLinitializers,
            dim09initializers,
            dim10initializers,
            dim11initializers,
            dim12initializers,
            dim13initializers,
            dim14initializers,
            dim15initializers,
            dim16initializers,
            dim17initializers,
            dim18initializers,
            dim19initializers,
            dim20initializers,
            dim21initializers,
            dim22initializers,
            dim23initializers,
            dim24initializers,
            dim25initializers,
            dim26initializers,
            dim27initializers,
            dim28initializers,
            dim29initializers,
            dim30initializers,
            dim31initializers,
            dim32initializers
        };

        /* Lemieux coefficients of the free direction integers as given
           in Christiane Lemieux, private communication, September 2004
        */
        const unsigned long dim041Linitializers[] = {
            1UL,1UL,3UL,13UL,7UL,35UL,61UL,91UL,0UL};
        const unsigned long dim042Linitializers[] = {
            1UL,1UL,7UL,11UL,5UL,35UL,55UL,75UL,0UL};
        const unsigned long dim043Linitializers[] = {
            1UL,3UL,5UL,5UL,11UL,23UL,29UL,139UL,0UL};
        const unsigned long dim044Linitializers[] = {
            1UL,1UL,1UL,7UL,11UL,15UL,17UL,81UL,0UL};
        const unsigned long dim045Linitializers[] = {
            1UL,1UL,7UL,9UL,5UL,57UL,79UL,103UL,0UL};
        const unsigned long dim046Linitializers[] = {
            1UL,1UL,7UL,13UL,19UL,5UL,5UL,185UL,0UL};
        const unsigned long dim047Linitializers[] = {
            1UL,3UL,1UL,3UL,13UL,57UL,97UL,131UL,0UL};
        const unsigned long dim048Linitializers[] = {
            1UL,1UL,5UL,5UL,21UL,25UL,125UL,197UL,0UL};
        const unsigned long dim049Linitializers[] = {
            1UL,3UL,3UL,9UL,31UL,11UL,103UL,201UL,0UL};
        const unsigned long dim050Linitializers[] = {
            1UL,1UL,5UL,3UL,7UL,25UL,51UL,121UL,0UL};
        const unsigned long dim051Linitializers[] = {
            1UL,3UL,7UL,15UL,19UL,53UL,73UL,189UL,0UL};
        const unsigned long dim052Linitializers[] = {
            1UL,1UL,1UL,15UL,19UL,55UL,27UL,183UL,0UL};
        const unsigned long dim053Linitializers[] = {
            1UL,1UL,7UL,13UL,3UL,29UL,109UL,69UL,0UL};
        const unsigned long dim054Linitializers[] = {
            1UL,1UL,5UL,15UL,15UL,23UL,15UL,1UL,57UL,0UL};
        const unsigned long dim055Linitializers[] = {
            1UL,3UL,1UL,3UL,23UL,55UL,43UL,143UL,397UL,0UL};
        const unsigned long dim056Linitializers[] = {
            1UL,1UL,3UL,11UL,29UL,9UL,35UL,131UL,411UL,0UL};
        const unsigned long dim057Linitializers[] = {
            1UL,3UL,1UL,7UL,27UL,39UL,103UL,199UL,277UL,0UL};
        const unsigned long dim058Linitializers[] = {
            1UL,3UL,7UL,3UL,19UL,55UL,127UL,67UL,449UL,0UL};
        const unsigned long dim059Linitializers[] = {
            1UL,3UL,7UL,3UL,5UL,29UL,45UL,85UL,3UL,0UL};
        const unsigned long dim060Linitializers[] = {
            1UL,3UL,5UL,5UL,13UL,23UL,75UL,245UL,453UL,0UL};
        const unsigned long dim061Linitializers[] = {
            1UL,3UL,1UL,15UL,21UL,47UL,3UL,77UL,165UL,0UL};
        const unsigned long dim062Linitializers[] = {
            1UL,1UL,7UL,9UL,15UL,5UL,117UL,73UL,473UL,0UL};
        const unsigned long dim063Linitializers[] = {
            1UL,3UL,1UL,9UL,1UL,21UL,13UL,173UL,313UL,0UL};
        const unsigned long dim064Linitializers[] = {
            1UL,1UL,7UL,3UL,11UL,45UL,63UL,77UL,49UL,0UL};
        const unsigned long dim065Linitializers[] = {
            1UL,1UL,1UL,1UL,1UL,25UL,123UL,39UL,259UL,0UL};
        const unsigned long dim066Linitializers[] = {
            1UL,1UL,1UL,5UL,23UL,11UL,59UL,11UL,203UL,0UL};
        const unsigned long dim067Linitializers[] = {
            1UL,3UL,3UL,15UL,21UL,1UL,73UL,71UL,421UL,0UL};
        const unsigned long dim068Linitializers[] = {
            1UL,1UL,5UL,11UL,15UL,31UL,115UL,95UL,217UL,0UL};
        const unsigned long dim069Linitializers[] = {
            1UL,1UL,3UL,3UL,7UL,53UL,37UL,43UL,439UL,0UL};
        const unsigned long dim070Linitializers[] = {
            1UL,1UL,1UL,1UL,27UL,53UL,69UL,159UL,321UL,0UL};
        const unsigned long dim071Linitializers[] = {
            1UL,1UL,5UL,15UL,29UL,17UL,19UL,43UL,449UL,0UL};
        const unsigned long dim072Linitializers[] = {
            1UL,1UL,3UL,9UL,1UL,55UL,121UL,205UL,255UL,0UL};
        const unsigned long dim073Linitializers[] = {
            1UL,1UL,3UL,11UL,9UL,47UL,107UL,11UL,417UL,0UL};
        const unsigned long dim074Linitializers[] = {
            1UL,1UL,1UL,5UL,17UL,25UL,21UL,83UL,95UL,0UL};
        const unsigned long dim075Linitializers[] = {
            1UL,3UL,5UL,13UL,31UL,25UL,61UL,157UL,407UL,0UL};
        const unsigned long dim076Linitializers[] = {
            1UL,1UL,7UL,9UL,25UL,33UL,41UL,35UL,17UL,0UL};
        const unsigned long dim077Linitializers[] = {
            1UL,3UL,7UL,15UL,13UL,39UL,61UL,187UL,461UL,0UL};
        const unsigned long dim078Linitializers[] = {
            1UL,3UL,7UL,13UL,5UL,57UL,23UL,177UL,435UL,0UL};
        const unsigned long dim079Linitializers[] = {
            1UL,1UL,3UL,15UL,11UL,27UL,115UL,5UL,337UL,0UL};
        const unsigned long dim080Linitializers[] = {
            1UL,3UL,7UL,3UL,15UL,63UL,61UL,171UL,339UL,0UL};
        const unsigned long dim081Linitializers[] = {
            1UL,3UL,3UL,13UL,15UL,61UL,59UL,47UL,1UL,0UL};
        const unsigned long dim082Linitializers[] = {
            1UL,1UL,5UL,15UL,13UL,5UL,39UL,83UL,329UL,0UL};
        const unsigned long dim083Linitializers[] = {
            1UL,1UL,5UL,5UL,5UL,27UL,25UL,39UL,301UL,0UL};
        const unsigned long dim084Linitializers[] = {
            1UL,1UL,5UL,11UL,31UL,41UL,35UL,233UL,27UL,0UL};
        const unsigned long dim085Linitializers[] = {
            1UL,3UL,5UL,15UL,7UL,37UL,119UL,171UL,419UL,0UL};
        const unsigned long dim086Linitializers[] = {
            1UL,3UL,5UL,5UL,3UL,29UL,21UL,189UL,417UL,0UL};
        const unsigned long dim087Linitializers[] = {
            1UL,1UL,1UL,1UL,21UL,41UL,117UL,119UL,351UL,0UL};
        const unsigned long dim088Linitializers[] = {
            1UL,1UL,3UL,1UL,7UL,27UL,87UL,19UL,213UL,0UL};
        const unsigned long dim089Linitializers[] = {
            1UL,1UL,1UL,1UL,17UL,7UL,97UL,217UL,477UL,0UL};
        const unsigned long dim090Linitializers[] = {
            1UL,1UL,7UL,1UL,29UL,61UL,103UL,231UL,269UL,0UL};
        const unsigned long dim091Linitializers[] = {
            1UL,1UL,7UL,13UL,9UL,27UL,107UL,207UL,311UL,0UL};
        const unsigned long dim092Linitializers[] = {
            1UL,1UL,7UL,5UL,25UL,21UL,107UL,179UL,423UL,0UL};
        const unsigned long dim093Linitializers[] = {
            1UL,3UL,5UL,11UL,7UL,1UL,17UL,245UL,281UL,0UL};
        const unsigned long dim094Linitializers[] = {
            1UL,3UL,5UL,9UL,1UL,5UL,53UL,59UL,125UL,0UL};
        const unsigned long dim095Linitializers[] = {
            1UL,1UL,7UL,1UL,31UL,57UL,71UL,245UL,125UL,0UL};
        const unsigned long dim096Linitializers[] = {
            1UL,1UL,7UL,5UL,5UL,57UL,53UL,253UL,441UL,0UL};
        const unsigned long dim097Linitializers[] = {
            1UL,3UL,1UL,13UL,19UL,35UL,119UL,235UL,381UL,0UL};
        const unsigned long dim098Linitializers[] = {
            1UL,3UL,1UL,7UL,19UL,59UL,115UL,33UL,361UL,0UL};
        const unsigned long dim099Linitializers[] = {
            1UL,1UL,3UL,5UL,13UL,1UL,49UL,143UL,501UL,0UL};
        const unsigned long dim100Linitializers[] = {
            1UL,1UL,3UL,5UL,1UL,63UL,101UL,85UL,189UL,0UL};
        const unsigned long dim101Linitializers[] = {
            1UL,1UL,5UL,11UL,27UL,63UL,13UL,131UL,5UL,0UL};
        const unsigned long dim102Linitializers[] = {
            1UL,1UL,5UL,7UL,15UL,45UL,75UL,59UL,455UL,585UL,0UL};
        const unsigned long dim103Linitializers[] = {
            1UL,3UL,1UL,3UL,7UL,7UL,111UL,23UL,119UL,959UL,0UL};
        const unsigned long dim104Linitializers[] = {
            1UL,3UL,3UL,9UL,11UL,41UL,109UL,163UL,161UL,879UL,0UL};
        const unsigned long dim105Linitializers[] = {
            1UL,3UL,5UL,1UL,21UL,41UL,121UL,183UL,315UL,219UL,0UL};
        const unsigned long dim106Linitializers[] = {
            1UL,1UL,3UL,9UL,15UL,3UL,9UL,223UL,441UL,929UL,0UL};
        const unsigned long dim107Linitializers[] = {
            1UL,1UL,7UL,9UL,3UL,5UL,93UL,57UL,253UL,457UL,0UL};
        const unsigned long dim108Linitializers[] = {
            1UL,1UL,7UL,13UL,15UL,29UL,83UL,21UL,35UL,45UL,0UL};
        const unsigned long dim109Linitializers[] = {
            1UL,1UL,3UL,7UL,13UL,61UL,119UL,219UL,85UL,505UL,0UL};
        const unsigned long dim110Linitializers[] = {
            1UL,1UL,3UL,3UL,17UL,13UL,35UL,197UL,291UL,109UL,0UL};
        const unsigned long dim111Linitializers[] = {
            1UL,1UL,3UL,3UL,5UL,1UL,113UL,103UL,217UL,253UL,0UL};
        const unsigned long dim112Linitializers[] = {
            1UL,1UL,7UL,1UL,15UL,39UL,63UL,223UL,17UL,9UL,0UL};
        const unsigned long dim113Linitializers[] = {
            1UL,3UL,7UL,1UL,17UL,29UL,67UL,103UL,495UL,383UL,0UL};
        const unsigned long dim114Linitializers[] = {
            1UL,3UL,3UL,15UL,31UL,59UL,75UL,165UL,51UL,913UL,0UL};
        const unsigned long dim115Linitializers[] = {
            1UL,3UL,7UL,9UL,5UL,27UL,79UL,219UL,233UL,37UL,0UL};
        const unsigned long dim116Linitializers[] = {
            1UL,3UL,5UL,15UL,1UL,11UL,15UL,211UL,417UL,811UL,0UL};
        const unsigned long dim117Linitializers[] = {
            1UL,3UL,5UL,3UL,29UL,27UL,39UL,137UL,407UL,231UL,0UL};
        const unsigned long dim118Linitializers[] = {
            1UL,1UL,3UL,5UL,29UL,43UL,125UL,135UL,109UL,67UL,0UL};
        const unsigned long dim119Linitializers[] = {
            1UL,1UL,1UL,5UL,11UL,39UL,107UL,159UL,323UL,381UL,0UL};
        const unsigned long dim120Linitializers[] = {
            1UL,1UL,1UL,1UL,9UL,11UL,33UL,55UL,169UL,253UL,0UL};
        const unsigned long dim121Linitializers[] = {
            1UL,3UL,5UL,5UL,11UL,53UL,63UL,101UL,251UL,897UL,0UL};
        const unsigned long dim122Linitializers[] = {
            1UL,3UL,7UL,1UL,25UL,15UL,83UL,119UL,53UL,157UL,0UL};
        const unsigned long dim123Linitializers[] = {
            1UL,3UL,5UL,13UL,5UL,5UL,3UL,195UL,111UL,451UL,0UL};
        const unsigned long dim124Linitializers[] = {
            1UL,3UL,1UL,15UL,11UL,1UL,19UL,11UL,307UL,777UL,0UL};
        const unsigned long dim125Linitializers[] = {
            1UL,3UL,7UL,11UL,5UL,5UL,17UL,231UL,345UL,981UL,0UL};
        const unsigned long dim126Linitializers[] = {
            1UL,1UL,3UL,3UL,1UL,33UL,83UL,201UL,57UL,475UL,0UL};
        const unsigned long dim127Linitializers[] = {
            1UL,3UL,7UL,7UL,17UL,13UL,35UL,175UL,499UL,809UL,0UL};
        const unsigned long dim128Linitializers[] = {
            1UL,1UL,5UL,3UL,3UL,17UL,103UL,119UL,499UL,865UL,0UL};
        const unsigned long dim129Linitializers[] = {
            1UL,1UL,1UL,11UL,27UL,25UL,37UL,121UL,401UL,11UL,0UL};
        const unsigned long dim130Linitializers[] = {
            1UL,1UL,1UL,11UL,9UL,25UL,25UL,241UL,403UL,3UL,0UL};
        const unsigned long dim131Linitializers[] = {
            1UL,1UL,1UL,1UL,11UL,1UL,39UL,163UL,231UL,573UL,0UL};
        const unsigned long dim132Linitializers[] = {
            1UL,1UL,1UL,13UL,13UL,21UL,75UL,185UL,99UL,545UL,0UL};
        const unsigned long dim133Linitializers[] = {
            1UL,1UL,1UL,15UL,3UL,63UL,69UL,11UL,173UL,315UL,0UL};
        const unsigned long dim134Linitializers[] = {
            1UL,3UL,5UL,15UL,11UL,3UL,95UL,49UL,123UL,765UL,0UL};
        const unsigned long dim135Linitializers[] = {
            1UL,1UL,1UL,15UL,3UL,63UL,77UL,31UL,425UL,711UL,0UL};
        const unsigned long dim136Linitializers[] = {
            1UL,1UL,7UL,15UL,1UL,37UL,119UL,145UL,489UL,583UL,0UL};
        const unsigned long dim137Linitializers[] = {
            1UL,3UL,5UL,15UL,3UL,49UL,117UL,211UL,165UL,323UL,0UL};
        const unsigned long dim138Linitializers[] = {
            1UL,3UL,7UL,1UL,27UL,63UL,77UL,201UL,225UL,803UL,0UL};
        const unsigned long dim139Linitializers[] = {
            1UL,1UL,1UL,11UL,23UL,35UL,67UL,21UL,469UL,357UL,0UL};
        const unsigned long dim140Linitializers[] = {
            1UL,1UL,7UL,7UL,9UL,7UL,25UL,237UL,237UL,571UL,0UL};
        const unsigned long dim141Linitializers[] = {
            1UL,1UL,3UL,15UL,29UL,5UL,107UL,109UL,241UL,47UL,0UL};
        const unsigned long dim142Linitializers[] = {
            1UL,3UL,5UL,11UL,27UL,63UL,29UL,13UL,203UL,675UL,0UL};
        const unsigned long dim143Linitializers[] = {
            1UL,1UL,3UL,9UL,9UL,11UL,103UL,179UL,449UL,263UL,0UL};
        const unsigned long dim144Linitializers[] = {
            1UL,3UL,5UL,11UL,29UL,63UL,53UL,151UL,259UL,223UL,0UL};
        const unsigned long dim145Linitializers[] = {
            1UL,1UL,3UL,7UL,9UL,25UL,5UL,197UL,237UL,163UL,0UL};
        const unsigned long dim146Linitializers[] = {
            1UL,3UL,7UL,13UL,5UL,57UL,67UL,193UL,147UL,241UL,0UL};
        const unsigned long dim147Linitializers[] = {
            1UL,1UL,5UL,15UL,15UL,33UL,17UL,67UL,161UL,341UL,0UL};
        const unsigned long dim148Linitializers[] = {
            1UL,1UL,3UL,13UL,17UL,43UL,21UL,197UL,441UL,985UL,0UL};
        const unsigned long dim149Linitializers[] = {
            1UL,3UL,1UL,5UL,15UL,33UL,33UL,193UL,305UL,829UL,0UL};
        const unsigned long dim150Linitializers[] = {
            1UL,1UL,1UL,13UL,19UL,27UL,71UL,187UL,477UL,239UL,0UL};
        const unsigned long dim151Linitializers[] = {
            1UL,1UL,1UL,9UL,9UL,17UL,41UL,177UL,229UL,983UL,0UL};
        const unsigned long dim152Linitializers[] = {
            1UL,3UL,5UL,9UL,15UL,45UL,97UL,205UL,43UL,767UL,0UL};
        const unsigned long dim153Linitializers[] = {
            1UL,1UL,1UL,9UL,31UL,31UL,77UL,159UL,395UL,809UL,0UL};
        const unsigned long dim154Linitializers[] = {
            1UL,3UL,3UL,3UL,29UL,19UL,73UL,123UL,165UL,307UL,0UL};
        const unsigned long dim155Linitializers[] = {
            1UL,3UL,1UL,7UL,5UL,11UL,77UL,227UL,355UL,403UL,0UL};
        const unsigned long dim156Linitializers[] = {
            1UL,3UL,5UL,5UL,25UL,31UL,1UL,215UL,451UL,195UL,0UL};
        const unsigned long dim157Linitializers[] = {
            1UL,3UL,7UL,15UL,29UL,37UL,101UL,241UL,17UL,633UL,0UL};
        const unsigned long dim158Linitializers[] = {
            1UL,1UL,5UL,1UL,11UL,3UL,107UL,137UL,489UL,5UL,0UL};
        const unsigned long dim159Linitializers[] = {
            1UL,1UL,1UL,7UL,19UL,19UL,75UL,85UL,471UL,355UL,0UL};
        const unsigned long dim160Linitializers[] = {
            1UL,1UL,3UL,3UL,9UL,13UL,113UL,167UL,13UL,27UL,0UL};
        const unsigned long dim161Linitializers[] = {
            1UL,3UL,5UL,11UL,21UL,3UL,89UL,205UL,377UL,307UL,0UL};
        const unsigned long dim162Linitializers[] = {
            1UL,1UL,1UL,9UL,31UL,61UL,65UL,9UL,391UL,141UL,867UL,0UL};
        const unsigned long dim163Linitializers[] = {
            1UL,1UL,1UL,9UL,19UL,19UL,61UL,227UL,241UL,55UL,161UL,0UL};
        const unsigned long dim164Linitializers[] = {
            1UL,1UL,1UL,11UL,1UL,19UL,7UL,233UL,463UL,171UL,1941UL,0UL};
        const unsigned long dim165Linitializers[] = {
            1UL,1UL,5UL,7UL,25UL,13UL,103UL,75UL,19UL,1021UL,1063UL,0UL};
        const unsigned long dim166Linitializers[] = {
            1UL,1UL,1UL,15UL,17UL,17UL,79UL,63UL,391UL,403UL,1221UL,0UL};
        const unsigned long dim167Linitializers[] = {
            1UL,3UL,3UL,11UL,29UL,25UL,29UL,107UL,335UL,475UL,963UL,0UL};
        const unsigned long dim168Linitializers[] = {
            1UL,3UL,5UL,1UL,31UL,33UL,49UL,43UL,155UL,9UL,1285UL,0UL};
        const unsigned long dim169Linitializers[] = {
            1UL,1UL,5UL,5UL,15UL,47UL,39UL,161UL,357UL,863UL,1039UL,0UL};
        const unsigned long dim170Linitializers[] = {
            1UL,3UL,7UL,15UL,1UL,39UL,47UL,109UL,427UL,393UL,1103UL,0UL};
        const unsigned long dim171Linitializers[] = {
            1UL,1UL,1UL,9UL,9UL,29UL,121UL,233UL,157UL,99UL,701UL,0UL};
        const unsigned long dim172Linitializers[] = {
            1UL,1UL,1UL,7UL,1UL,29UL,75UL,121UL,439UL,109UL,993UL,0UL};
        const unsigned long dim173Linitializers[] = {
            1UL,1UL,1UL,9UL,5UL,1UL,39UL,59UL,89UL,157UL,1865UL,0UL};
        const unsigned long dim174Linitializers[] = {
            1UL,1UL,5UL,1UL,3UL,37UL,89UL,93UL,143UL,533UL,175UL,0UL};
        const unsigned long dim175Linitializers[] = {
            1UL,1UL,3UL,5UL,7UL,33UL,35UL,173UL,159UL,135UL,241UL,0UL};
        const unsigned long dim176Linitializers[] = {
            1UL,1UL,1UL,15UL,17UL,37UL,79UL,131UL,43UL,891UL,229UL,0UL};
        const unsigned long dim177Linitializers[] = {
            1UL,1UL,1UL,1UL,1UL,35UL,121UL,177UL,397UL,1017UL,583UL,0UL};
        const unsigned long dim178Linitializers[] = {
            1UL,1UL,3UL,15UL,31UL,21UL,43UL,67UL,467UL,923UL,1473UL,0UL};
        const unsigned long dim179Linitializers[] = {
            1UL,1UL,1UL,7UL,1UL,33UL,77UL,111UL,125UL,771UL,1975UL,0UL};
        const unsigned long dim180Linitializers[] = {
            1UL,3UL,7UL,13UL,1UL,51UL,113UL,139UL,245UL,573UL,503UL,0UL};
        const unsigned long dim181Linitializers[] = {
            1UL,3UL,1UL,9UL,21UL,49UL,15UL,157UL,49UL,483UL,291UL,0UL};
        const unsigned long dim182Linitializers[] = {
            1UL,1UL,1UL,1UL,29UL,35UL,17UL,65UL,403UL,485UL,1603UL,0UL};
        const unsigned long dim183Linitializers[] = {
            1UL,1UL,1UL,7UL,19UL,1UL,37UL,129UL,203UL,321UL,1809UL,0UL};
        const unsigned long dim184Linitializers[] = {
            1UL,3UL,7UL,15UL,15UL,9UL,5UL,77UL,29UL,485UL,581UL,0UL};
        const unsigned long dim185Linitializers[] = {
            1UL,1UL,3UL,5UL,15UL,49UL,97UL,105UL,309UL,875UL,1581UL,0UL};
        const unsigned long dim186Linitializers[] = {
            1UL,3UL,5UL,1UL,5UL,19UL,63UL,35UL,165UL,399UL,1489UL,0UL};
        const unsigned long dim187Linitializers[] = {
            1UL,3UL,5UL,3UL,23UL,5UL,79UL,137UL,115UL,599UL,1127UL,0UL};
        const unsigned long dim188Linitializers[] = {
            1UL,1UL,7UL,5UL,3UL,61UL,27UL,177UL,257UL,91UL,841UL,0UL};
        const unsigned long dim189Linitializers[] = {
            1UL,1UL,3UL,5UL,9UL,31UL,91UL,209UL,409UL,661UL,159UL,0UL};
        const unsigned long dim190Linitializers[] = {
            1UL,3UL,1UL,15UL,23UL,39UL,23UL,195UL,245UL,203UL,947UL,0UL};
        const unsigned long dim191Linitializers[] = {
            1UL,1UL,3UL,1UL,15UL,59UL,67UL,95UL,155UL,461UL,147UL,0UL};
        const unsigned long dim192Linitializers[] = {
            1UL,3UL,7UL,5UL,23UL,25UL,87UL,11UL,51UL,449UL,1631UL,0UL};
        const unsigned long dim193Linitializers[] = {
            1UL,1UL,1UL,1UL,17UL,57UL,7UL,197UL,409UL,609UL,135UL,0UL};
        const unsigned long dim194Linitializers[] = {
            1UL,1UL,1UL,9UL,1UL,61UL,115UL,113UL,495UL,895UL,1595UL,0UL};
        const unsigned long dim195Linitializers[] = {
            1UL,3UL,7UL,15UL,9UL,47UL,121UL,211UL,379UL,985UL,1755UL,0UL};
        const unsigned long dim196Linitializers[] = {
            1UL,3UL,1UL,3UL,7UL,57UL,27UL,231UL,339UL,325UL,1023UL,0UL};
        const unsigned long dim197Linitializers[] = {
            1UL,1UL,1UL,1UL,19UL,63UL,63UL,239UL,31UL,643UL,373UL,0UL};
        const unsigned long dim198Linitializers[] = {
            1UL,3UL,1UL,11UL,19UL,9UL,7UL,171UL,21UL,691UL,215UL,0UL};
        const unsigned long dim199Linitializers[] = {
            1UL,1UL,5UL,13UL,11UL,57UL,39UL,211UL,241UL,893UL,555UL,0UL};
        const unsigned long dim200Linitializers[] = {
            1UL,1UL,7UL,5UL,29UL,21UL,45UL,59UL,509UL,223UL,491UL,0UL};
        const unsigned long dim201Linitializers[] = {
            1UL,1UL,7UL,9UL,15UL,61UL,97UL,75UL,127UL,779UL,839UL,0UL};
        const unsigned long dim202Linitializers[] = {
            1UL,1UL,7UL,15UL,17UL,33UL,75UL,237UL,191UL,925UL,681UL,0UL};
        const unsigned long dim203Linitializers[] = {
            1UL,3UL,5UL,7UL,27UL,57UL,123UL,111UL,101UL,371UL,1129UL,0UL};
        const unsigned long dim204Linitializers[] = {
            1UL,3UL,5UL,5UL,29UL,45UL,59UL,127UL,229UL,967UL,2027UL,0UL};
        const unsigned long dim205Linitializers[] = {
            1UL,1UL,1UL,1UL,17UL,7UL,23UL,199UL,241UL,455UL,135UL,0UL};
        const unsigned long dim206Linitializers[] = {
            1UL,1UL,7UL,15UL,27UL,29UL,105UL,171UL,337UL,503UL,1817UL,0UL};
        const unsigned long dim207Linitializers[] = {
            1UL,1UL,3UL,7UL,21UL,35UL,61UL,71UL,405UL,647UL,2045UL,0UL};
        const unsigned long dim208Linitializers[] = {
            1UL,1UL,1UL,1UL,1UL,15UL,65UL,167UL,501UL,79UL,737UL,0UL};
        const unsigned long dim209Linitializers[] = {
            1UL,1UL,5UL,1UL,3UL,49UL,27UL,189UL,341UL,615UL,1287UL,0UL};
        const unsigned long dim210Linitializers[] = {
            1UL,1UL,1UL,9UL,1UL,7UL,31UL,159UL,503UL,327UL,1613UL,0UL};
        const unsigned long dim211Linitializers[] = {
            1UL,3UL,3UL,3UL,3UL,23UL,99UL,115UL,323UL,997UL,987UL,0UL};
        const unsigned long dim212Linitializers[] = {
            1UL,1UL,1UL,9UL,19UL,33UL,93UL,247UL,509UL,453UL,891UL,0UL};
        const unsigned long dim213Linitializers[] = {
            1UL,1UL,3UL,1UL,13UL,19UL,35UL,153UL,161UL,633UL,445UL,0UL};
        const unsigned long dim214Linitializers[] = {
            1UL,3UL,5UL,15UL,31UL,5UL,87UL,197UL,183UL,783UL,1823UL,0UL};
        const unsigned long dim215Linitializers[] = {
            1UL,1UL,7UL,5UL,19UL,63UL,69UL,221UL,129UL,231UL,1195UL,0UL};
        const unsigned long dim216Linitializers[] = {
            1UL,1UL,5UL,5UL,13UL,23UL,19UL,231UL,245UL,917UL,379UL,0UL};
        const unsigned long dim217Linitializers[] = {
            1UL,3UL,1UL,15UL,19UL,43UL,27UL,223UL,171UL,413UL,125UL,0UL};
        const unsigned long dim218Linitializers[] = {
            1UL,1UL,1UL,9UL,1UL,59UL,21UL,15UL,509UL,207UL,589UL,0UL};
        const unsigned long dim219Linitializers[] = {
            1UL,3UL,5UL,3UL,19UL,31UL,113UL,19UL,23UL,733UL,499UL,0UL};
        const unsigned long dim220Linitializers[] = {
            1UL,1UL,7UL,1UL,19UL,51UL,101UL,165UL,47UL,925UL,1093UL,0UL};
        const unsigned long dim221Linitializers[] = {
            1UL,3UL,3UL,9UL,15UL,21UL,43UL,243UL,237UL,461UL,1361UL,0UL};
        const unsigned long dim222Linitializers[] = {
            1UL,1UL,1UL,9UL,17UL,15UL,75UL,75UL,113UL,715UL,1419UL,0UL};
        const unsigned long dim223Linitializers[] = {
            1UL,1UL,7UL,13UL,17UL,1UL,99UL,15UL,347UL,721UL,1405UL,0UL};
        const unsigned long dim224Linitializers[] = {
            1UL,1UL,7UL,15UL,7UL,27UL,23UL,183UL,39UL,59UL,571UL,0UL};
        const unsigned long dim225Linitializers[] = {
            1UL,3UL,5UL,9UL,7UL,43UL,35UL,165UL,463UL,567UL,859UL,0UL};
        const unsigned long dim226Linitializers[] = {
            1UL,3UL,3UL,11UL,15UL,19UL,17UL,129UL,311UL,343UL,15UL,0UL};
        const unsigned long dim227Linitializers[] = {
            1UL,1UL,1UL,15UL,31UL,59UL,63UL,39UL,347UL,359UL,105UL,0UL};
        const unsigned long dim228Linitializers[] = {
            1UL,1UL,1UL,15UL,5UL,43UL,87UL,241UL,109UL,61UL,685UL,0UL};
        const unsigned long dim229Linitializers[] = {
            1UL,1UL,7UL,7UL,9UL,39UL,121UL,127UL,369UL,579UL,853UL,0UL};
        const unsigned long dim230Linitializers[] = {
            1UL,1UL,1UL,1UL,17UL,15UL,15UL,95UL,325UL,627UL,299UL,0UL};
        const unsigned long dim231Linitializers[] = {
            1UL,1UL,3UL,13UL,31UL,53UL,85UL,111UL,289UL,811UL,1635UL,0UL};
        const unsigned long dim232Linitializers[] = {
            1UL,3UL,7UL,1UL,19UL,29UL,75UL,185UL,153UL,573UL,653UL,0UL};
        const unsigned long dim233Linitializers[] = {
            1UL,3UL,7UL,1UL,29UL,31UL,55UL,91UL,249UL,247UL,1015UL,0UL};
        const unsigned long dim234Linitializers[] = {
            1UL,3UL,5UL,7UL,1UL,49UL,113UL,139UL,257UL,127UL,307UL,0UL};
        const unsigned long dim235Linitializers[] = {
            1UL,3UL,5UL,9UL,15UL,15UL,123UL,105UL,105UL,225UL,1893UL,0UL};
        const unsigned long dim236Linitializers[] = {
            1UL,3UL,3UL,1UL,15UL,5UL,105UL,249UL,73UL,709UL,1557UL,0UL};
        const unsigned long dim237Linitializers[] = {
            1UL,1UL,1UL,9UL,17UL,31UL,113UL,73UL,65UL,701UL,1439UL,0UL};
        const unsigned long dim238Linitializers[] = {
            1UL,3UL,5UL,15UL,13UL,21UL,117UL,131UL,243UL,859UL,323UL,0UL};
        const unsigned long dim239Linitializers[] = {
            1UL,1UL,1UL,9UL,19UL,15UL,69UL,149UL,89UL,681UL,515UL,0UL};
        const unsigned long dim240Linitializers[] = {
            1UL,1UL,1UL,5UL,29UL,13UL,21UL,97UL,301UL,27UL,967UL,0UL};
        const unsigned long dim241Linitializers[] = {
            1UL,1UL,3UL,3UL,15UL,45UL,107UL,227UL,495UL,769UL,1935UL,0UL};
        const unsigned long dim242Linitializers[] = {
            1UL,1UL,1UL,11UL,5UL,27UL,41UL,173UL,261UL,703UL,1349UL,0UL};
        const unsigned long dim243Linitializers[] = {
            1UL,3UL,3UL,3UL,11UL,35UL,97UL,43UL,501UL,563UL,1331UL,0UL};
        const unsigned long dim244Linitializers[] = {
            1UL,1UL,1UL,7UL,1UL,17UL,87UL,17UL,429UL,245UL,1941UL,0UL};
        const unsigned long dim245Linitializers[] = {
            1UL,1UL,7UL,15UL,29UL,13UL,1UL,175UL,425UL,233UL,797UL,0UL};
        const unsigned long dim246Linitializers[] = {
            1UL,1UL,3UL,11UL,21UL,57UL,49UL,49UL,163UL,685UL,701UL,0UL};
        const unsigned long dim247Linitializers[] = {
            1UL,3UL,3UL,7UL,11UL,45UL,107UL,111UL,379UL,703UL,1403UL,0UL};
        const unsigned long dim248Linitializers[] = {
            1UL,1UL,7UL,3UL,21UL,7UL,117UL,49UL,469UL,37UL,775UL,0UL};
        const unsigned long dim249Linitializers[] = {
            1UL,1UL,5UL,15UL,31UL,63UL,101UL,77UL,507UL,489UL,1955UL,0UL};
        const unsigned long dim250Linitializers[] = {
            1UL,3UL,3UL,11UL,19UL,21UL,101UL,255UL,203UL,673UL,665UL,0UL};
        const unsigned long dim251Linitializers[] = {
            1UL,3UL,3UL,15UL,17UL,47UL,125UL,187UL,271UL,899UL,2003UL,0UL};
        const unsigned long dim252Linitializers[] = {
            1UL,1UL,7UL,7UL,1UL,35UL,13UL,235UL,5UL,337UL,905UL,0UL};
        const unsigned long dim253Linitializers[] = {
            1UL,3UL,1UL,15UL,1UL,43UL,1UL,27UL,37UL,695UL,1429UL,0UL};
        const unsigned long dim254Linitializers[] = {
            1UL,3UL,1UL,11UL,21UL,27UL,93UL,161UL,299UL,665UL,495UL,0UL};
        const unsigned long dim255Linitializers[] = {
            1UL,3UL,3UL,15UL,3UL,1UL,81UL,111UL,105UL,547UL,897UL,0UL};
        const unsigned long dim256Linitializers[] = {
            1UL,3UL,5UL,1UL,3UL,53UL,97UL,253UL,401UL,827UL,1467UL,0UL};
        const unsigned long dim257Linitializers[] = {
            1UL,1UL,1UL,5UL,19UL,59UL,105UL,125UL,271UL,351UL,719UL,0UL};
        const unsigned long dim258Linitializers[] = {
            1UL,3UL,5UL,13UL,7UL,11UL,91UL,41UL,441UL,759UL,1827UL,0UL};
        const unsigned long dim259Linitializers[] = {
            1UL,3UL,7UL,11UL,29UL,61UL,61UL,23UL,307UL,863UL,363UL,0UL};
        const unsigned long dim260Linitializers[] = {
            1UL,1UL,7UL,1UL,15UL,35UL,29UL,133UL,415UL,473UL,1737UL,0UL};
        const unsigned long dim261Linitializers[] = {
            1UL,1UL,1UL,13UL,7UL,33UL,35UL,225UL,117UL,681UL,1545UL,0UL};
        const unsigned long dim262Linitializers[] = {
            1UL,1UL,1UL,3UL,5UL,41UL,83UL,247UL,13UL,373UL,1091UL,0UL};
        const unsigned long dim263Linitializers[] = {
            1UL,3UL,1UL,13UL,25UL,61UL,71UL,217UL,233UL,313UL,547UL,0UL};
        const unsigned long dim264Linitializers[] = {
            1UL,3UL,1UL,7UL,3UL,29UL,3UL,49UL,93UL,465UL,15UL,0UL};
        const unsigned long dim265Linitializers[] = {
            1UL,1UL,1UL,9UL,17UL,61UL,99UL,163UL,129UL,485UL,1087UL,0UL};
        const unsigned long dim266Linitializers[] = {
            1UL,1UL,1UL,9UL,9UL,33UL,31UL,163UL,145UL,649UL,253UL,0UL};
        const unsigned long dim267Linitializers[] = {
            1UL,1UL,1UL,1UL,17UL,63UL,43UL,235UL,287UL,111UL,567UL,0UL};
        const unsigned long dim268Linitializers[] = {
            1UL,3UL,5UL,13UL,29UL,7UL,11UL,69UL,153UL,127UL,449UL,0UL};
        const unsigned long dim269Linitializers[] = {
            1UL,1UL,5UL,9UL,11UL,21UL,15UL,189UL,431UL,493UL,1219UL,0UL};
        const unsigned long dim270Linitializers[] = {
            1UL,1UL,1UL,15UL,19UL,5UL,47UL,91UL,399UL,293UL,1743UL,0UL};
        const unsigned long dim271Linitializers[] = {
            1UL,3UL,3UL,11UL,29UL,53UL,53UL,225UL,409UL,303UL,333UL,0UL};
        const unsigned long dim272Linitializers[] = {
            1UL,1UL,1UL,15UL,31UL,31UL,21UL,81UL,147UL,287UL,1753UL,0UL};
        const unsigned long dim273Linitializers[] = {
            1UL,3UL,5UL,5UL,5UL,63UL,35UL,125UL,41UL,687UL,1793UL,0UL};
        const unsigned long dim274Linitializers[] = {
            1UL,1UL,1UL,9UL,19UL,59UL,107UL,219UL,455UL,971UL,297UL,0UL};
        const unsigned long dim275Linitializers[] = {
            1UL,1UL,3UL,5UL,3UL,51UL,121UL,31UL,245UL,105UL,1311UL,0UL};
        const unsigned long dim276Linitializers[] = {
            1UL,3UL,1UL,5UL,5UL,57UL,75UL,107UL,161UL,431UL,1693UL,0UL};
        const unsigned long dim277Linitializers[] = {
            1UL,3UL,1UL,3UL,19UL,53UL,27UL,31UL,191UL,565UL,1015UL,0UL};
        const unsigned long dim278Linitializers[] = {
            1UL,3UL,5UL,13UL,9UL,41UL,35UL,249UL,287UL,49UL,123UL,0UL};
        const unsigned long dim279Linitializers[] = {
            1UL,1UL,5UL,7UL,27UL,17UL,21UL,3UL,151UL,885UL,1165UL,0UL};
        const unsigned long dim280Linitializers[] = {
            1UL,1UL,7UL,1UL,15UL,17UL,65UL,139UL,427UL,339UL,1171UL,0UL};
        const unsigned long dim281Linitializers[] = {
            1UL,1UL,1UL,5UL,23UL,5UL,9UL,89UL,321UL,907UL,391UL,0UL};
        const unsigned long dim282Linitializers[] = {
            1UL,1UL,7UL,9UL,15UL,1UL,77UL,71UL,87UL,701UL,917UL,0UL};
        const unsigned long dim283Linitializers[] = {
            1UL,1UL,7UL,1UL,17UL,37UL,115UL,127UL,469UL,779UL,1543UL,0UL};
        const unsigned long dim284Linitializers[] = {
            1UL,3UL,7UL,3UL,5UL,61UL,15UL,37UL,301UL,951UL,1437UL,0UL};
        const unsigned long dim285Linitializers[] = {
            1UL,1UL,1UL,13UL,9UL,51UL,127UL,145UL,229UL,55UL,1567UL,0UL};
        const unsigned long dim286Linitializers[] = {
            1UL,3UL,7UL,15UL,19UL,47UL,53UL,153UL,295UL,47UL,1337UL,0UL};
        const unsigned long dim287Linitializers[] = {
            1UL,3UL,3UL,5UL,11UL,31UL,29UL,133UL,327UL,287UL,507UL,0UL};
        const unsigned long dim288Linitializers[] = {
            1UL,1UL,7UL,7UL,25UL,31UL,37UL,199UL,25UL,927UL,1317UL,0UL};
        const unsigned long dim289Linitializers[] = {
            1UL,1UL,7UL,9UL,3UL,39UL,127UL,167UL,345UL,467UL,759UL,0UL};
        const unsigned long dim290Linitializers[] = {
            1UL,1UL,1UL,1UL,31UL,21UL,15UL,101UL,293UL,787UL,1025UL,0UL};
        const unsigned long dim291Linitializers[] = {
            1UL,1UL,5UL,3UL,11UL,41UL,105UL,109UL,149UL,837UL,1813UL,0UL};
        const unsigned long dim292Linitializers[] = {
            1UL,1UL,3UL,5UL,29UL,13UL,19UL,97UL,309UL,901UL,753UL,0UL};
        const unsigned long dim293Linitializers[] = {
            1UL,1UL,7UL,1UL,19UL,17UL,31UL,39UL,173UL,361UL,1177UL,0UL};
        const unsigned long dim294Linitializers[] = {
            1UL,3UL,3UL,3UL,3UL,41UL,81UL,7UL,341UL,491UL,43UL,0UL};
        const unsigned long dim295Linitializers[] = {
            1UL,1UL,7UL,7UL,31UL,35UL,29UL,77UL,11UL,335UL,1275UL,0UL};
        const unsigned long dim296Linitializers[] = {
            1UL,3UL,3UL,15UL,17UL,45UL,19UL,63UL,151UL,849UL,129UL,0UL};
        const unsigned long dim297Linitializers[] = {
            1UL,1UL,7UL,5UL,7UL,13UL,47UL,73UL,79UL,31UL,499UL,0UL};
        const unsigned long dim298Linitializers[] = {
            1UL,3UL,1UL,11UL,1UL,41UL,59UL,151UL,247UL,115UL,1295UL,0UL};
        const unsigned long dim299Linitializers[] = {
            1UL,1UL,1UL,9UL,31UL,37UL,73UL,23UL,295UL,483UL,179UL,0UL};
        const unsigned long dim300Linitializers[] = {
            1UL,3UL,1UL,15UL,13UL,63UL,81UL,27UL,169UL,825UL,2037UL,0UL};
        const unsigned long dim301Linitializers[] = {
            1UL,3UL,5UL,15UL,7UL,11UL,73UL,1UL,451UL,101UL,2039UL,0UL};
        const unsigned long dim302Linitializers[] = {
            1UL,3UL,5UL,3UL,13UL,53UL,31UL,137UL,173UL,319UL,1521UL,0UL};
        const unsigned long dim303Linitializers[] = {
            1UL,3UL,1UL,3UL,29UL,1UL,73UL,227UL,377UL,337UL,1189UL,0UL};
        const unsigned long dim304Linitializers[] = {
            1UL,3UL,3UL,13UL,27UL,9UL,31UL,101UL,229UL,165UL,1983UL,0UL};
        const unsigned long dim305Linitializers[] = {
            1UL,3UL,1UL,13UL,13UL,19UL,19UL,111UL,319UL,421UL,223UL,0UL};
        const unsigned long dim306Linitializers[] = {
            1UL,1UL,7UL,15UL,25UL,37UL,61UL,55UL,359UL,255UL,1955UL,0UL};
        const unsigned long dim307Linitializers[] = {
            1UL,1UL,5UL,13UL,17UL,43UL,49UL,215UL,383UL,915UL,51UL,0UL};
        const unsigned long dim308Linitializers[] = {
            1UL,1UL,3UL,1UL,3UL,7UL,13UL,119UL,155UL,585UL,967UL,0UL};
        const unsigned long dim309Linitializers[] = {
            1UL,3UL,1UL,13UL,1UL,63UL,125UL,21UL,103UL,287UL,457UL,0UL};
        const unsigned long dim310Linitializers[] = {
            1UL,1UL,7UL,1UL,31UL,17UL,125UL,137UL,345UL,379UL,1925UL,0UL};
        const unsigned long dim311Linitializers[] = {
            1UL,1UL,3UL,5UL,5UL,25UL,119UL,153UL,455UL,271UL,2023UL,0UL};
        const unsigned long dim312Linitializers[] = {
            1UL,1UL,7UL,9UL,9UL,37UL,115UL,47UL,5UL,255UL,917UL,0UL};
        const unsigned long dim313Linitializers[] = {
            1UL,3UL,5UL,3UL,31UL,21UL,75UL,203UL,489UL,593UL,1UL,0UL};
        const unsigned long dim314Linitializers[] = {
            1UL,3UL,7UL,15UL,19UL,63UL,123UL,153UL,135UL,977UL,1875UL,0UL};
        const unsigned long dim315Linitializers[] = {
            1UL,1UL,1UL,1UL,5UL,59UL,31UL,25UL,127UL,209UL,745UL,0UL};
        const unsigned long dim316Linitializers[] = {
            1UL,1UL,1UL,1UL,19UL,45UL,67UL,159UL,301UL,199UL,535UL,0UL};
        const unsigned long dim317Linitializers[] = {
            1UL,1UL,7UL,1UL,31UL,17UL,19UL,225UL,369UL,125UL,421UL,0UL};
        const unsigned long dim318Linitializers[] = {
            1UL,3UL,3UL,11UL,7UL,59UL,115UL,197UL,459UL,469UL,1055UL,0UL};
        const unsigned long dim319Linitializers[] = {
            1UL,3UL,1UL,3UL,27UL,45UL,35UL,131UL,349UL,101UL,411UL,0UL};
        const unsigned long dim320Linitializers[] = {
            1UL,3UL,7UL,11UL,9UL,3UL,67UL,145UL,299UL,253UL,1339UL,0UL};
        const unsigned long dim321Linitializers[] = {
            1UL,3UL,3UL,11UL,9UL,37UL,123UL,229UL,273UL,269UL,515UL,0UL};
        const unsigned long dim322Linitializers[] = {
            1UL,3UL,7UL,15UL,11UL,25UL,75UL,5UL,367UL,217UL,951UL,0UL};
        const unsigned long dim323Linitializers[] = {
            1UL,1UL,3UL,7UL,9UL,23UL,63UL,237UL,385UL,159UL,1273UL,0UL};
        const unsigned long dim324Linitializers[] = {
            1UL,1UL,5UL,11UL,23UL,5UL,55UL,193UL,109UL,865UL,663UL,0UL};
        const unsigned long dim325Linitializers[] = {
            1UL,1UL,7UL,15UL,1UL,57UL,17UL,141UL,51UL,217UL,1259UL,0UL};
        const unsigned long dim326Linitializers[] = {
            1UL,1UL,3UL,3UL,15UL,7UL,89UL,233UL,71UL,329UL,203UL,0UL};
        const unsigned long dim327Linitializers[] = {
            1UL,3UL,7UL,11UL,11UL,1UL,19UL,155UL,89UL,437UL,573UL,0UL};
        const unsigned long dim328Linitializers[] = {
            1UL,3UL,1UL,9UL,27UL,61UL,47UL,109UL,161UL,913UL,1681UL,0UL};
        const unsigned long dim329Linitializers[] = {
            1UL,1UL,7UL,15UL,1UL,33UL,19UL,15UL,23UL,913UL,989UL,0UL};
        const unsigned long dim330Linitializers[] = {
            1UL,3UL,1UL,1UL,25UL,39UL,119UL,193UL,13UL,571UL,157UL,0UL};
        const unsigned long dim331Linitializers[] = {
            1UL,1UL,7UL,13UL,9UL,55UL,59UL,147UL,361UL,935UL,515UL,0UL};
        const unsigned long dim332Linitializers[] = {
            1UL,1UL,1UL,9UL,7UL,59UL,67UL,117UL,71UL,855UL,1493UL,0UL};
        const unsigned long dim333Linitializers[] = {
            1UL,3UL,1UL,3UL,13UL,19UL,57UL,141UL,305UL,275UL,1079UL,0UL};
        const unsigned long dim334Linitializers[] = {
            1UL,1UL,1UL,9UL,17UL,61UL,33UL,7UL,43UL,931UL,781UL,0UL};
        const unsigned long dim335Linitializers[] = {
            1UL,1UL,3UL,1UL,11UL,17UL,21UL,97UL,295UL,277UL,1721UL,0UL};
        const unsigned long dim336Linitializers[] = {
            1UL,3UL,1UL,13UL,15UL,43UL,11UL,241UL,147UL,391UL,1641UL,0UL};
        const unsigned long dim337Linitializers[] = {
            1UL,1UL,1UL,1UL,1UL,19UL,37UL,21UL,255UL,263UL,1571UL,0UL};
        const unsigned long dim338Linitializers[] = {
            1UL,1UL,3UL,3UL,23UL,59UL,89UL,17UL,475UL,303UL,757UL,543UL,0UL};
        const unsigned long dim339Linitializers[] = {
           1UL,3UL,3UL,9UL,11UL,55UL,35UL,159UL,139UL,203UL,1531UL,1825UL,0UL};
        const unsigned long dim340Linitializers[] = {
            1UL,1UL,5UL,3UL,17UL,53UL,51UL,241UL,269UL,949UL,1373UL,325UL,0UL};
        const unsigned long dim341Linitializers[] = {
            1UL,3UL,7UL,7UL,5UL,29UL,91UL,149UL,239UL,193UL,1951UL,2675UL,0UL};
        const unsigned long dim342Linitializers[] = {
            1UL,3UL,5UL,1UL,27UL,33UL,69UL,11UL,51UL,371UL,833UL,2685UL,0UL};
        const unsigned long dim343Linitializers[] = {
            1UL,1UL,1UL,15UL,1UL,17UL,35UL,57UL,171UL,1007UL,449UL,367UL,0UL};
        const unsigned long dim344Linitializers[] = {
            1UL,1UL,1UL,7UL,25UL,61UL,73UL,219UL,379UL,53UL,589UL,4065UL,0UL};
        const unsigned long dim345Linitializers[] = {
            1UL,3UL,5UL,13UL,21UL,29UL,45UL,19UL,163UL,169UL,147UL,597UL,0UL};
        const unsigned long dim346Linitializers[] = {
            1UL,1UL,5UL,11UL,21UL,27UL,7UL,17UL,237UL,591UL,255UL,1235UL,0UL};
        const unsigned long dim347Linitializers[] = {
            1UL,1UL,7UL,7UL,17UL,41UL,69UL,237UL,397UL,173UL,1229UL,2341UL,0UL};
        const unsigned long dim348Linitializers[] = {
            1UL,1UL,3UL,1UL,1UL,33UL,125UL,47UL,11UL,783UL,1323UL,2469UL,0UL};
        const unsigned long dim349Linitializers[] = {
            1UL,3UL,1UL,11UL,3UL,39UL,35UL,133UL,153UL,55UL,1171UL,3165UL,0UL};
        const unsigned long dim350Linitializers[] = {
           1UL,1UL,5UL,11UL,27UL,23UL,103UL,245UL,375UL,753UL,477UL,2165UL,0UL};
        const unsigned long dim351Linitializers[] = {
          1UL,3UL,1UL,15UL,15UL,49UL,127UL,223UL,387UL,771UL,1719UL,1465UL,0UL};
        const unsigned long dim352Linitializers[] = {
            1UL,1UL,1UL,9UL,11UL,9UL,17UL,185UL,239UL,899UL,1273UL,3961UL,0UL};
        const unsigned long dim353Linitializers[] = {
            1UL,1UL,3UL,13UL,11UL,51UL,73UL,81UL,389UL,647UL,1767UL,1215UL,0UL};
        const unsigned long dim354Linitializers[] = {
            1UL,3UL,5UL,15UL,19UL,9UL,69UL,35UL,349UL,977UL,1603UL,1435UL,0UL};
        const unsigned long dim355Linitializers[] = {
            1UL,1UL,1UL,1UL,19UL,59UL,123UL,37UL,41UL,961UL,181UL,1275UL,0UL};
        const unsigned long dim356Linitializers[] = {
            1UL,1UL,1UL,1UL,31UL,29UL,37UL,71UL,205UL,947UL,115UL,3017UL,0UL};
        const unsigned long dim357Linitializers[] = {
            1UL,1UL,7UL,15UL,5UL,37UL,101UL,169UL,221UL,245UL,687UL,195UL,0UL};
        const unsigned long dim358Linitializers[] = {
            1UL,1UL,1UL,1UL,19UL,9UL,125UL,157UL,119UL,283UL,1721UL,743UL,0UL};
        const unsigned long dim359Linitializers[] = {
            1UL,1UL,7UL,3UL,1UL,7UL,61UL,71UL,119UL,257UL,1227UL,2893UL,0UL};
        const unsigned long dim360Linitializers[] = {
            1UL,3UL,3UL,3UL,25UL,41UL,25UL,225UL,31UL,57UL,925UL,2139UL,0UL};


        const unsigned long * const Linitializers[359] = {
            dim02SLinitializers,
            dim03SLinitializers,
            dim04SLinitializers,
            dim05SLinitializers,
            dim06SLinitializers,
            dim07SLinitializers,
            dim08SLinitializers,
            dim09SLinitializers,
            dim10SLinitializers,
            dim11SLinitializers,
            dim12SLinitializers,
            dim13SLinitializers,
            dim14SLinitializers,
            dim15SLinitializers,
            dim16SLinitializers,
            dim17SLinitializers,
            dim18SLinitializers,
            dim19SLinitializers,
            dim20SLinitializers,
            dim21SLinitializers,
            dim22SLinitializers,
            dim23SLinitializers,
            dim24SLinitializers,
            dim25SLinitializers,
            dim26SLinitializers,
            dim27SLinitializers,
            dim28SLinitializers,
            dim29SLinitializers,
            dim30SLinitializers,
            dim31SLinitializers,
            dim32SLinitializers,
            dim33SLinitializers,
            dim34SLinitializers,
            dim35SLinitializers,
            dim36SLinitializers,
            dim37SLinitializers,
            dim38SLinitializers,
            dim39SLinitializers,
            dim40SLinitializers,
            dim041Linitializers,
            dim042Linitializers,
            dim043Linitializers,
            dim044Linitializers,
            dim045Linitializers,
            dim046Linitializers,
            dim047Linitializers,
            dim048Linitializers,
            dim049Linitializers,
            dim050Linitializers,
            dim051Linitializers,
            dim052Linitializers,
            dim053Linitializers,
            dim054Linitializers,
            dim055Linitializers,
            dim056Linitializers,
            dim057Linitializers,
            dim058Linitializers,
            dim059Linitializers,
            dim060Linitializers,
            dim061Linitializers,
            dim062Linitializers,
            dim063Linitializers,
            dim064Linitializers,
            dim065Linitializers,
            dim066Linitializers,
            dim067Linitializers,
            dim068Linitializers,
            dim069Linitializers,
            dim070Linitializers,
            dim071Linitializers,
            dim072Linitializers,
            dim073Linitializers,
            dim074Linitializers,
            dim075Linitializers,
            dim076Linitializers,
            dim077Linitializers,
            dim078Linitializers,
            dim079Linitializers,
            dim080Linitializers,
            dim081Linitializers,
            dim082Linitializers,
            dim083Linitializers,
            dim084Linitializers,
            dim085Linitializers,
            dim086Linitializers,
            dim087Linitializers,
            dim088Linitializers,
            dim089Linitializers,
            dim090Linitializers,
            dim091Linitializers,
            dim092Linitializers,
            dim093Linitializers,
            dim094Linitializers,
            dim095Linitializers,
            dim096Linitializers,
            dim097Linitializers,
            dim098Linitializers,
            dim099Linitializers,
            dim100Linitializers,
            dim101Linitializers,
            dim102Linitializers,
            dim103Linitializers,
            dim104Linitializers,
            dim105Linitializers,
            dim106Linitializers,
            dim107Linitializers,
            dim108Linitializers,
            dim109Linitializers,
            dim110Linitializers,
            dim111Linitializers,
            dim112Linitializers,
            dim113Linitializers,
            dim114Linitializers,
            dim115Linitializers,
            dim116Linitializers,
            dim117Linitializers,
            dim118Linitializers,
            dim119Linitializers,
            dim120Linitializers,
            dim121Linitializers,
            dim122Linitializers,
            dim123Linitializers,
            dim124Linitializers,
            dim125Linitializers,
            dim126Linitializers,
            dim127Linitializers,
            dim128Linitializers,
            dim129Linitializers,
            dim130Linitializers,
            dim131Linitializers,
            dim132Linitializers,
            dim133Linitializers,
            dim134Linitializers,
            dim135Linitializers,
            dim136Linitializers,
            dim137Linitializers,
            dim138Linitializers,
            dim139Linitializers,
            dim140Linitializers,
            dim141Linitializers,
            dim142Linitializers,
            dim143Linitializers,
            dim144Linitializers,
            dim145Linitializers,
            dim146Linitializers,
            dim147Linitializers,
            dim148Linitializers,
            dim149Linitializers,
            dim150Linitializers,
            dim151Linitializers,
            dim152Linitializers,
            dim153Linitializers,
            dim154Linitializers,
            dim155Linitializers,
            dim156Linitializers,
            dim157Linitializers,
            dim158Linitializers,
            dim159Linitializers,
            dim160Linitializers,
            dim161Linitializers,
            dim162Linitializers,
            dim163Linitializers,
            dim164Linitializers,
            dim165Linitializers,
            dim166Linitializers,
            dim167Linitializers,
            dim168Linitializers,
            dim169Linitializers,
            dim170Linitializers,
            dim171Linitializers,
            dim172Linitializers,
            dim173Linitializers,
            dim174Linitializers,
            dim175Linitializers,
            dim176Linitializers,
            dim177Linitializers,
            dim178Linitializers,
            dim179Linitializers,
            dim180Linitializers,
            dim181Linitializers,
            dim182Linitializers,
            dim183Linitializers,
            dim184Linitializers,
            dim185Linitializers,
            dim186Linitializers,
            dim187Linitializers,
            dim188Linitializers,
            dim189Linitializers,
            dim190Linitializers,
            dim191Linitializers,
            dim192Linitializers,
            dim193Linitializers,
            dim194Linitializers,
            dim195Linitializers,
            dim196Linitializers,
            dim197Linitializers,
            dim198Linitializers,
            dim199Linitializers,
            dim200Linitializers,
            dim201Linitializers,
            dim202Linitializers,
            dim203Linitializers,
            dim204Linitializers,
            dim205Linitializers,
            dim206Linitializers,
            dim207Linitializers,
            dim208Linitializers,
            dim209Linitializers,
            dim210Linitializers,
            dim211Linitializers,
            dim212Linitializers,
            dim213Linitializers,
            dim214Linitializers,
            dim215Linitializers,
            dim216Linitializers,
            dim217Linitializers,
            dim218Linitializers,
            dim219Linitializers,
            dim220Linitializers,
            dim221Linitializers,
            dim222Linitializers,
            dim223Linitializers,
            dim224Linitializers,
            dim225Linitializers,
            dim226Linitializers,
            dim227Linitializers,
            dim228Linitializers,
            dim229Linitializers,
            dim230Linitializers,
            dim231Linitializers,
            dim232Linitializers,
            dim233Linitializers,
            dim234Linitializers,
            dim235Linitializers,
            dim236Linitializers,
            dim237Linitializers,
            dim238Linitializers,
            dim239Linitializers,
            dim240Linitializers,
            dim241Linitializers,
            dim242Linitializers,
            dim243Linitializers,
            dim244Linitializers,
            dim245Linitializers,
            dim246Linitializers,
            dim247Linitializers,
            dim248Linitializers,
            dim249Linitializers,
            dim250Linitializers,
            dim251Linitializers,
            dim252Linitializers,
            dim253Linitializers,
            dim254Linitializers,
            dim255Linitializers,
            dim256Linitializers,
            dim257Linitializers,
            dim258Linitializers,
            dim259Linitializers,
            dim260Linitializers,
            dim261Linitializers,
            dim262Linitializers,
            dim263Linitializers,
            dim264Linitializers,
            dim265Linitializers,
            dim266Linitializers,
            dim267Linitializers,
            dim268Linitializers,
            dim269Linitializers,
            dim270Linitializers,
            dim271Linitializers,
            dim272Linitializers,
            dim273Linitializers,
            dim274Linitializers,
            dim275Linitializers,
            dim276Linitializers,
            dim277Linitializers,
            dim278Linitializers,
            dim279Linitializers,
            dim280Linitializers,
            dim281Linitializers,
            dim282Linitializers,
            dim283Linitializers,
            dim284Linitializers,
            dim285Linitializers,
            dim286Linitializers,
            dim287Linitializers,
            dim288Linitializers,
            dim289Linitializers,
            dim290Linitializers,
            dim291Linitializers,
            dim292Linitializers,
            dim293Linitializers,
            dim294Linitializers,
            dim295Linitializers,
            dim296Linitializers,
            dim297Linitializers,
            dim298Linitializers,
            dim299Linitializers,
            dim300Linitializers,
            dim301Linitializers,
            dim302Linitializers,
            dim303Linitializers,
            dim304Linitializers,
            dim305Linitializers,
            dim306Linitializers,
            dim307Linitializers,
            dim308Linitializers,
            dim309Linitializers,
            dim310Linitializers,
            dim311Linitializers,
            dim312Linitializers,
            dim313Linitializers,
            dim314Linitializers,
            dim315Linitializers,
            dim316Linitializers,
            dim317Linitializers,
            dim318Linitializers,
            dim319Linitializers,
            dim320Linitializers,
            dim321Linitializers,
            dim322Linitializers,
            dim323Linitializers,
            dim324Linitializers,
            dim325Linitializers,
            dim326Linitializers,
            dim327Linitializers,
            dim328Linitializers,
            dim329Linitializers,
            dim330Linitializers,
            dim331Linitializers,
            dim332Linitializers,
            dim333Linitializers,
            dim334Linitializers,
            dim335Linitializers,
            dim336Linitializers,
            dim337Linitializers,
            dim338Linitializers,
            dim339Linitializers,
            dim340Linitializers,
            dim341Linitializers,
            dim342Linitializers,
            dim343Linitializers,
            dim344Linitializers,
            dim345Linitializers,
            dim346Linitializers,
            dim347Linitializers,
            dim348Linitializers,
            dim349Linitializers,
            dim350Linitializers,
            dim351Linitializers,
            dim352Linitializers,
            dim353Linitializers,
            dim354Linitializers,
            dim355Linitializers,
            dim356Linitializers,
            dim357Linitializers,
            dim358Linitializers,
            dim359Linitializers,
            dim360Linitializers
        };

    }

    const int SobolRsg::bits_ = 8*sizeof(unsigned long);
    // 1/(2^bits_) (written as (1/2)/(2^(bits_-1)) to avoid long overflow)
    const double SobolRsg::normalizationFactor_ =
        0.5/(1UL<<(SobolRsg::bits_-1));

    SobolRsg::SobolRsg(Size dimensionality, unsigned long seed,
                       DirectionIntegers directionIntegers)
    : dimensionality_(dimensionality), sequenceCounter_(0), firstDraw_(true),
      sequence_(Array(dimensionality), 1.0),
      integerSequence_(dimensionality, 0),
      directionIntegers_(dimensionality,std::vector<unsigned long>(bits_))
    {
        QL_REQUIRE(dimensionality<=PPMT_MAX_DIM,
                   "dimensionality "
                   + SizeFormatter::toString(dimensionality) +
                   " exceeds the number of available "
                   "primitive polynomials modulo two (" +
                   IntegerFormatter::toString(PPMT_MAX_DIM) +")");

        // initializes coefficient array of the k-th primitive polynomial
        // and degree of the k-th primitive polynomial
        std::vector<unsigned int> degree(dimensionality_);
        std::vector<long> ppmt(dimensionality_);
        // degree 0 is not used
        ppmt[0]=0;
        degree[0]=0;
        Size k, index;
        unsigned int currentDegree;
        for (k=1,index=0,currentDegree=1; k<dimensionality_; k++,index++){
            ppmt[k] = PrimitivePolynomials[currentDegree-1][index];
            if (ppmt[k]==-1) {
                ++currentDegree;
                index=0;
                ppmt[k] = PrimitivePolynomials[currentDegree-1][index];
            }
            degree[k] = currentDegree;
        }

        // initializes bits_ direction integers for each dimension
        // and store them into directionIntegers_[dimensionality_][bits_]
        //
        // In each dimension k with its associated primitive polynomial,
        // the first degree_[k] direction integers can be chosen freely
        // provided that only the l leftmost bits can be non-zero, and
        // that the l-th leftmost bit must be set

        // degenerate (no free direction integers) first dimension
        int j;
        for (j=0; j<bits_; j++)
            directionIntegers_[0][j] = (1UL<<(bits_-j-1));


        Size maxTabulated;
        // dimensions from 2 (k=1) to maxTabulated (k=maxTabulated-1) included
        // are initialized from tabulated coefficients
        switch (directionIntegers) {
            case Unit:
                maxTabulated=dimensionality_;
                for (k=1; k<maxTabulated; k++) {
                    for (Size l=1; l<=degree[k]; l++) {
                        directionIntegers_[k][l-1] = 1UL;
                        directionIntegers_[k][l-1] <<= (bits_-l);
                    }
                }
                break;
            case Jaeckel:
                // maxTabulated=32
                maxTabulated = sizeof(initializers)/sizeof(unsigned long *)+1;
                for (k=1; k<QL_MIN(dimensionality_, maxTabulated); k++) {
                    j = 0;
                    // 0UL marks coefficients' end for a given dimension
                    while (initializers[k-1][j] != 0UL) {
                        directionIntegers_[k][j] = initializers[k-1][j];
                        directionIntegers_[k][j] <<= (bits_-j-1);
                        j++;
                    }
                }
                break;
            case SobolLevitan:
                // maxTabulated=40
                maxTabulated = sizeof(SLinitializers)/sizeof(unsigned long *)+1;
                for (k=1; k<QL_MIN(dimensionality_, maxTabulated); k++) {
                    j = 0;
                    // 0UL marks coefficients' end for a given dimension
                    while (SLinitializers[k-1][j] != 0UL) {
                        directionIntegers_[k][j] = SLinitializers[k-1][j];
                        directionIntegers_[k][j] <<= (bits_-j-1);
                        j++;
                    }
                }
                break;
            case SobolLevitanLemieux:
                // maxTabulated=360
                maxTabulated = sizeof(Linitializers)/sizeof(unsigned long *)+1;
                for (k=1; k<QL_MIN(dimensionality_, maxTabulated); k++) {
                    j = 0;
                    // 0UL marks coefficients' end for a given dimension
                    while (Linitializers[k-1][j] != 0UL) {
                        directionIntegers_[k][j] = Linitializers[k-1][j];
                        directionIntegers_[k][j] <<= (bits_-j-1);
                        j++;
                    }
                }
                break;
            default:
                break;
        }

        // random initialization for higher dimensions
        if (dimensionality_>maxTabulated) {
            MersenneTwisterUniformRng uniformRng(seed);
            for (k=maxTabulated; k<dimensionality_; k++) {
                for (Size l=1; l<=degree[k]; l++) {
                    do {
                        // u is in (0,1)
                        double u = uniformRng.next().value;
                        // the direction integer has at most the
                        // rightmost l bits non-zero
                        directionIntegers_[k][l-1] =
                            (unsigned long)(u*(1UL<<l));
                    } while (!(directionIntegers_[k][l-1] & 1UL));
                    // iterate until the direction integer is odd
                    // that is it has the rightmost bit set

                    // shifting bits_-l bits to the left
                    // we are guaranteed that the l-th leftmost bit
                    // is set, and only the first l leftmost bit
                    // can be non-zero
                    directionIntegers_[k][l-1] <<= (bits_-l);
                }
            }
        }

        // computation of directionIntegers_[k][l] for l>=degree_[k]
        // by recurrence relation
        for (k=1; k<dimensionality_; k++) {
            unsigned int gk = degree[k];
            for (int l=gk; l<bits_; l++) {
                // eq. 8.19 "Monte Carlo Methods in Finance" by P. Jäckel
                unsigned long n = (directionIntegers_[k][l-gk]>>gk);
                // a[k][j] are the coefficients of the monomials in ppmt[k]
                // The highest order coefficient a[k][0] is not actually
                // used in the recurrence relation, and the lowest order
                // coefficient a[k][gk] is always set: this is the reason
                // why the highest and lowest coefficient of
                // the polynomial ppmt[k] are not included in its encoding,
                // provided that its degree is known.
                // That is: a[k][j] = ppmt[k] >> (gk-j-1)
                for (Size j=1; j<gk; j++) {
                    // XORed with a selection of (unshifted) direction
                    // integers controlled by which of the a[k][j] are set
                    if ((ppmt[k] >> (gk-j-1)) & 1UL)
                        n ^= directionIntegers_[k][l-j];
                }
                // a[k][gk] is always set, so directionIntegers_[k][l-gk]
                // will always enter
                n ^= directionIntegers_[k][l-gk];
                directionIntegers_[k][l]=n;
            }
        }

        // in case one needs to check the directionIntegers used
        /* bool printDirectionIntegers = false;
           if (printDirectionIntegers) {
               std::ofstream outStream("directionIntegers.txt");
               for (k=0; k<QL_MIN(32UL,dimensionality_); k++) {
                   outStream << std::endl << k+1       << "\t"
                                          << degree[k] << "\t"
                                          << ppmt[k]   << "\t";
                   for (j=0; j<10; j++) {
                       outStream << IntegerFormatter::toPowerOfTwo(
                           directionIntegers_[k][j], 3) << "\t";
                   }
               }
               outStream.close();
           }
        */

        // initialize the Sobol integer/double vectors
        for (k=0; k<dimensionality_; k++) {
            integerSequence_[k]=directionIntegers_[k][0];
            // first draw
            sequence_.value[k] = integerSequence_[k]*normalizationFactor_;
        }
    }


    const SobolRsg::sample_type& SobolRsg::nextSequence() const {
        if (firstDraw_) {
            // it was precomputed in the constructor
            firstDraw_ = false;
            return sequence_;
        }
        // increment the counter
        sequenceCounter_++;
        // did we overflow?
        QL_REQUIRE(sequenceCounter_ != 0, "period exceeded");

        // instead of using the counter n as new unique generating integer
        // for the n-th draw use the Gray code G(n) as proposed
        // by Antonov and Saleev
        unsigned long n = sequenceCounter_;
        // Find rightmost zero bit of n
        int j = 0;
        while (n & 1) { n >>= 1; j++; }
        for (Size k=0; k<dimensionality_; k++) {
            // XOR the appropriate direction number into each component of
            // the integer sequence to obtain a new Sobol integer for that
            // component
            integerSequence_[k] ^= directionIntegers_[k][j];
            // normalize to get a double in (0,1)
            sequence_.value[k] = integerSequence_[k]*normalizationFactor_;
        }
        return sequence_;
    }

}

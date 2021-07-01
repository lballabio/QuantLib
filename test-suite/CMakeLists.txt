# cpp files, this list is maintained manually

set(QuantLib-Test_SRC
    americanoption.cpp
    amortizingbond.cpp
    andreasenhugevolatilityinterpl.cpp
    array.cpp
    asianoptions.cpp
    assetswap.cpp
    autocovariances.cpp
    barrieroption.cpp
    basismodels.cpp
    basketoption.cpp
    batesmodel.cpp
    bermudanswaption.cpp
    binaryoption.cpp
    blackdeltacalculator.cpp
    blackformula.cpp
    bonds.cpp
    brownianbridge.cpp
    businessdayconventions.cpp
    calendars.cpp
    callablebonds.cpp
    capfloor.cpp
    capflooredcoupon.cpp
    cashflows.cpp
    catbonds.cpp
    cdo.cpp
    cdsoption.cpp
    chooseroption.cpp
    cliquetoption.cpp
    cms.cpp
    cmsspread.cpp
    commodityunitofmeasure.cpp
    compiledboostversion.cpp
    compoundoption.cpp
    convertiblebonds.cpp
    covariance.cpp
    creditdefaultswap.cpp
    creditriskplus.cpp
    crosscurrencyratehelpers.cpp
    currency.cpp
    curvestates.cpp
    dates.cpp
    daycounters.cpp
    defaultprobabilitycurves.cpp
    digitalcoupon.cpp
    digitaloption.cpp
    distributions.cpp
    dividendoption.cpp
    doublebarrieroption.cpp
    doublebinaryoption.cpp
    europeanoption.cpp
    everestoption.cpp
    exchangerate.cpp
    extendedtrees.cpp
    extensibleoptions.cpp
    fastfouriertransform.cpp
    fdcev.cpp
    fdheston.cpp
    fdcir.cpp
    fdmlinearop.cpp
    fdsabr.cpp
    fittedbonddiscountcurve.cpp
    forwardoption.cpp
    forwardrateagreement.cpp
    functions.cpp
    garch.cpp
    gaussianquadratures.cpp
    gjrgarchmodel.cpp
    gsr.cpp
    hestonmodel.cpp
    hestonslvmodel.cpp
    himalayaoption.cpp
    hybridhestonhullwhiteprocess.cpp
    indexes.cpp
    inflation.cpp
    inflationcapfloor.cpp
    inflationcapflooredcoupon.cpp
    inflationcpibond.cpp
    inflationcpicapfloor.cpp
    inflationcpiswap.cpp
    inflationvolatility.cpp
	inflationzciisinterpolation.cpp
    instruments.cpp
    integrals.cpp
    interestrates.cpp
    interpolations.cpp
    jumpdiffusion.cpp
    lazyobject.cpp
    libormarketmodel.cpp
    libormarketmodelprocess.cpp
    linearleastsquaresregression.cpp
    lookbackoptions.cpp
    lowdiscrepancysequences.cpp
    main.cpp
    margrabeoption.cpp
    marketmodel.cpp
    marketmodel_cms.cpp
    marketmodel_smm.cpp
    marketmodel_smmcapletalphacalibration.cpp
    marketmodel_smmcapletcalibration.cpp
    marketmodel_smmcaplethomocalibration.cpp
    markovfunctional.cpp
    matrices.cpp
    mclongstaffschwartzengine.cpp
    mersennetwister.cpp
    money.cpp
    noarbsabr.cpp
    normalclvmodel.cpp
    nthorderderivativeop.cpp
    nthtodefault.cpp
    numericaldifferentiation.cpp
    observable.cpp
    ode.cpp
    operators.cpp
    optimizers.cpp
    optionletstripper.cpp
    overnightindexedswap.cpp
    pagodaoption.cpp
    partialtimebarrieroption.cpp
    pathgenerator.cpp
    period.cpp
    piecewiseyieldcurve.cpp
    piecewisezerospreadedtermstructure.cpp
    quantlibtestsuite.cpp
    quantooption.cpp
    quotes.cpp
    rangeaccrual.cpp
    riskneutraldensitycalculator.cpp
    riskstats.cpp
    rngtraits.cpp
    rounding.cpp
    sampledcurve.cpp
    schedule.cpp
    settings.cpp
    shortratemodels.cpp
    sofrfutures.cpp
    solvers.cpp
    spreadoption.cpp
    squarerootclvmodel.cpp
    stats.cpp
    subperiodcoupons.cpp
    swap.cpp
    swapforwardmappings.cpp
    swaption.cpp
    swaptionvolatilitycube.cpp
    swaptionvolatilitymatrix.cpp
    swingoption.cpp
    termstructures.cpp
    timegrid.cpp
    timeseries.cpp
    tqreigendecomposition.cpp
    tracing.cpp
    transformedgrid.cpp
    twoassetbarrieroption.cpp
    twoassetcorrelationoption.cpp
    ultimateforwardtermstructure.cpp
    utilities.cpp
    variancegamma.cpp
    varianceoption.cpp
    varianceswaps.cpp
    volatilitymodels.cpp
    vpp.cpp
    zabr.cpp
    zerocouponswap.cpp
)

# hpp files, this list is maintained manually

set(QuantLib-Test_HDR
    americanoption.hpp
    amortizingbond.hpp
    andreasenhugevolatilityinterpl.hpp
    array.hpp
    asianoptions.hpp
    assetswap.hpp
    autocovariances.hpp
    barrieroption.hpp
    basismodels.hpp
    basketoption.hpp
    batesmodel.hpp
    bermudanswaption.hpp
    binaryoption.hpp
    blackdeltacalculator.hpp
    blackformula.hpp
    bonds.hpp
    brownianbridge.hpp
    businessdayconventions.hpp
    calendars.hpp
    callablebonds.hpp
    capfloor.hpp
    capflooredcoupon.hpp
    cashflows.hpp
    catbonds.hpp
    cdo.hpp
    cdsoption.hpp
    chooseroption.hpp
    cliquetoption.hpp
    cms.hpp
    cmsspread.hpp
    commodityunitofmeasure.hpp
    compiledboostversion.hpp
    compoundoption.hpp
    convertiblebonds.hpp
    covariance.hpp
    creditdefaultswap.hpp
    creditriskplus.hpp
    crosscurrencyratehelpers.hpp
    currency.hpp
    curvestates.hpp
    dates.hpp
    daycounters.hpp
    defaultprobabilitycurves.hpp
    digitalcoupon.hpp
    digitaloption.hpp
    distributions.hpp
    dividendoption.hpp
    doublebarrieroption.hpp
    doublebinaryoption.hpp
    europeanoption.hpp
    everestoption.hpp
    exchangerate.hpp
    extendedtrees.hpp
    extensibleoptions.hpp
    fastfouriertransform.hpp
    fdcev.hpp
    fdcir.hpp
    fdheston.hpp
    fdmlinearop.hpp
    fdsabr.hpp
    fittedbonddiscountcurve.hpp
    forwardoption.hpp
    forwardrateagreement.hpp
    functions.hpp
    garch.hpp
    gaussianquadratures.hpp
    gjrgarchmodel.hpp
    gsr.hpp
    hestonmodel.hpp
    hestonslvmodel.hpp
    himalayaoption.hpp
    hybridhestonhullwhiteprocess.hpp
    indexes.hpp
    inflation.hpp
    inflationcapfloor.hpp
    inflationcapflooredcoupon.hpp
    inflationcpibond.hpp
    inflationcpicapfloor.hpp
    inflationcpiswap.hpp
    inflationvolatility.hpp
	inflationzciisinterpolation.hpp
    instruments.hpp
    integrals.hpp
    interestrates.hpp
    interpolations.hpp
    jumpdiffusion.hpp
    lazyobject.hpp
    libormarketmodel.hpp
    libormarketmodelprocess.hpp
    linearleastsquaresregression.hpp
    lookbackoptions.hpp
    lowdiscrepancysequences.hpp
    margrabeoption.hpp
    marketmodel.hpp
    marketmodel_cms.hpp
    marketmodel_smm.hpp
    marketmodel_smmcapletalphacalibration.hpp
    marketmodel_smmcapletcalibration.hpp
    marketmodel_smmcaplethomocalibration.hpp
    markovfunctional.hpp
    matrices.hpp
    mclongstaffschwartzengine.hpp
    mersennetwister.hpp
    money.hpp
    noarbsabr.hpp
    normalclvmodel.hpp
    nthorderderivativeop.hpp
    nthtodefault.hpp
    numericaldifferentiation.hpp
    observable.hpp
    ode.hpp
    operators.hpp
    optimizers.hpp
    optionletstripper.hpp
    overnightindexedswap.hpp
    pagodaoption.hpp
    paralleltestrunner.hpp
    partialtimebarrieroption.hpp
    pathgenerator.hpp
    period.hpp
    piecewiseyieldcurve.hpp
    piecewisezerospreadedtermstructure.hpp
    quantooption.hpp
    quotes.hpp
    rangeaccrual.hpp
    riskneutraldensitycalculator.hpp
    riskstats.hpp
    rngtraits.hpp
    rounding.hpp
    sampledcurve.hpp
    schedule.hpp
    settings.hpp
    shortratemodels.hpp
    sofrfutures.hpp
    solvers.hpp
    speedlevel.hpp
    spreadoption.hpp
    squarerootclvmodel.hpp
    stats.hpp
    subperiodcoupons.hpp
    swap.hpp
    swapforwardmappings.hpp
    swaption.hpp
    swaptionvolatilitycube.hpp
    swaptionvolatilitymatrix.hpp
    swaptionvolstructuresutilities.hpp
    swingoption.hpp
    termstructures.hpp
    timegrid.hpp
    timeseries.hpp
    tqreigendecomposition.hpp
    tracing.hpp
    transformedgrid.hpp
    twoassetbarrieroption.hpp
    twoassetcorrelationoption.hpp
    ultimateforwardtermstructure.hpp
    utilities.hpp
    variancegamma.hpp
    varianceoption.hpp
    varianceswaps.hpp
    volatilitymodels.hpp
    vpp.hpp
    zabr.hpp
    zerocouponswap.hpp
)

set(BENCHMARK_FILES 
    main.cpp 
    quantlibbenchmark.cpp
    
    americanoption.cpp                  americanoption.hpp
    asianoptions.cpp                    asianoptions.hpp
    barrieroption.cpp                   barrieroption.hpp
    basketoption.cpp                    basketoption.hpp
    batesmodel.cpp                      batesmodel.hpp
    convertiblebonds.cpp                convertiblebonds.hpp
    digitaloption.cpp                   digitaloption.hpp
    dividendoption.cpp                  dividendoption.hpp
    europeanoption.cpp                  europeanoption.hpp
    fdheston.cpp                        fdheston.hpp
    hestonmodel.cpp                     hestonmodel.hpp
    interpolations.cpp                  interpolations.hpp
    jumpdiffusion.cpp                   jumpdiffusion.hpp
    lowdiscrepancysequences.cpp         lowdiscrepancysequences.hpp
    marketmodel_cms.cpp                 marketmodel_cms.hpp
    marketmodel_smm.cpp                 marketmodel_smm.hpp
    quantooption.cpp                    quantooption.hpp
    riskstats.cpp                       riskstats.hpp
    shortratemodels.cpp                 shortratemodels.hpp

    utilities.cpp                       utilities.hpp
                                        swaptionvolstructuresutilities.hpp
)

if (USE_BOOST_DYNAMIC_LIBRARIES)
    add_definitions(-DBOOST_TEST_DYN_LINK)
endif()

find_package (Boost REQUIRED COMPONENTS unit_test_framework)

set (TEST quantlib-test-suite)
add_executable (${TEST} ${QuantLib-Test_SRC} ${QuantLib-Test_HDR})
target_link_libraries (${TEST} ${QL_LINK_LIBRARY} ${Boost_LIBRARIES})
set_property(TARGET ${TEST} PROPERTY PROJECT_LABEL "testsuite")

set (BENCHMARK quantlib-benchmark)
add_executable (${BENCHMARK} ${BENCHMARK_FILES})
target_link_libraries (${BENCHMARK} ${QL_LINK_LIBRARY} ${Boost_LIBRARIES})
set_property(TARGET ${BENCHMARK} PROPERTY PROJECT_LABEL "benchmark")

add_test (${TEST} ${TEST})

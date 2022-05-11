set(QL_TEST_SOURCES
    americanoption.cpp
    amortizingbond.cpp
    andreasenhugevolatilityinterpl.cpp
    array.cpp
    asianoptions.cpp
    assetswap.cpp
    autocovariances.cpp
    barrieroption.cpp
    basismodels.cpp
    basisswapratehelpers.cpp
    basketoption.cpp
    batesmodel.cpp
    bermudanswaption.cpp
    binaryoption.cpp
    blackdeltacalculator.cpp
    blackformula.cpp
    bondforward.cpp
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
    overnightindexedcoupon.cpp
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
    svivolatility.cpp
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

set(QL_TEST_HEADERS
    americanoption.hpp
    amortizingbond.hpp
    andreasenhugevolatilityinterpl.hpp
    array.hpp
    asianoptions.hpp
    assetswap.hpp
    autocovariances.hpp
    barrieroption.hpp
    basismodels.hpp
    basisswapratehelpers.hpp
    basketoption.hpp
    batesmodel.hpp
    bermudanswaption.hpp
    binaryoption.hpp
    blackdeltacalculator.hpp
    blackformula.hpp
    bondforward.hpp
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
    overnightindexedcoupon.hpp
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
    svivolatility.hpp
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

set(QL_BENCHMARK_SOURCES
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

if (QL_BUILD_TEST_SUITE OR QL_BUILD_BENCHMARK)
    add_library(ql_unit_test_main STATIC main.cpp)
    target_include_directories(ql_unit_test_main PRIVATE
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS})
    if (NOT Boost_USE_STATIC_LIBS)
        target_compile_definitions(ql_unit_test_main PRIVATE BOOST_TEST_DYN_LINK)
    endif()
endif()

if (QL_BUILD_TEST_SUITE)
    add_executable(ql_test_suite ${QL_TEST_SOURCES} ${QL_TEST_HEADERS})
    set_target_properties(ql_test_suite PROPERTIES OUTPUT_NAME "quantlib-test-suite")
    if (NOT Boost_USE_STATIC_LIBS)
        target_compile_definitions(ql_test_suite PRIVATE BOOST_ALL_DYN_LINK BOOST_TEST_DYN_LINK)
    endif()
    if (QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER OR QL_ENABLE_SESSIONS OR
            QL_ENABLE_SINGLETON_THREAD_SAFE_INIT OR QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN)
        set(QL_TEST_SUITE_LIBRARIES Boost::thread ${RT_LIBRARY})
    endif()
    target_link_libraries(ql_test_suite PRIVATE
        ql_library
        ql_unit_test_main
        ${QL_TEST_SUITE_LIBRARIES})
    if (QL_INSTALL_TEST_SUITE)
        install(TARGETS ql_test_suite RUNTIME DESTINATION ${QL_INSTALL_BINDIR})
    endif()
    add_test(NAME quantlib_test_suite COMMAND ql_test_suite --log_level=message)
endif()

IF (QL_BUILD_BENCHMARK)
    add_executable(ql_benchmark ${QL_BENCHMARK_SOURCES})
    set_target_properties(ql_benchmark PROPERTIES OUTPUT_NAME "quantlib-benchmark")
    if (NOT Boost_USE_STATIC_LIBS)
        target_compile_definitions(ql_benchmark PRIVATE BOOST_ALL_DYN_LINK BOOST_TEST_DYN_LINK)
    endif()
    if (QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER OR QL_ENABLE_SESSIONS OR
            QL_ENABLE_SINGLETON_THREAD_SAFE_INIT OR QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN)
        set(QL_BENCHMARK_LIBRARIES Boost::thread ${RT_LIBRARY})
    endif()
    target_link_libraries(ql_benchmark PRIVATE
        ql_library
        ql_unit_test_main
        ${QL_BENCHMARK_LIBRARIES})
    if (QL_INSTALL_BENCHMARK)
        install(TARGETS ql_benchmark RUNTIME DESTINATION ${QL_INSTALL_BINDIR})
    endif()
endif()

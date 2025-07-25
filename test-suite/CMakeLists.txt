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
    cms_normal.cpp
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
    equitycashflow.cpp
    equityindex.cpp
    equitytotalreturnswap.cpp
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
    multipleresetscoupons.cpp
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
    preconditions.cpp
    prices.cpp
    quantlibglobalfixture.cpp
    quantooption.cpp
    quotes.cpp
    rangeaccrual.cpp
    riskneutraldensitycalculator.cpp
    riskstats.cpp
    rngtraits.cpp
    rounding.cpp
    schedule.cpp
    settings.cpp
    shortratemodels.cpp
    sofrfutures.cpp
    softbarrieroption.cpp
    solvers.cpp
    spreadoption.cpp
    squarerootclvmodel.cpp
    stats.cpp
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
    xoshiro256starstar.cpp
    zabr.cpp
    zerocouponswap.cpp
    zigguratgaussian.cpp
)

set(QL_TEST_HEADERS
    paralleltestrunner.hpp
    preconditions.hpp
    quantlibglobalfixture.hpp
    swaptionvolstructuresutilities.hpp
    toplevelfixture.hpp
    utilities.hpp
)


if (QL_BUILD_TEST_SUITE)
    add_library(ql_test OBJECT ${QL_TEST_SOURCES} ${QL_TEST_HEADERS})
    target_include_directories(ql_test PRIVATE
    	${CMAKE_CURRENT_SOURCE_DIR}  # test-suite/
    	${QuantLib_SOURCE_DIR}/ql    # core QL headers
    )

    
    if (NOT Boost_USE_STATIC_LIBS)
        target_compile_definitions(ql_test PUBLIC BOOST_ALL_DYN_LINK)
    endif()
    if(MSVC AND CMAKE_UNITY_BUILD)
        # for Unity builds, we need to add /bigobj
        target_compile_options(ql_test PUBLIC "/bigobj")
    endif()
    target_link_libraries(ql_test PUBLIC ql_library ${QL_THREAD_LIBRARIES})


    add_executable(ql_test_suite quantlibtestsuite.cpp)
    set_source_files_properties(quantlibtestsuite.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION true)
    set_target_properties(ql_test_suite PROPERTIES OUTPUT_NAME "quantlib-test-suite")
    target_link_libraries(ql_test_suite PRIVATE ql_test) 

    if (QL_INSTALL_TEST_SUITE)
        install(TARGETS ql_test_suite RUNTIME DESTINATION ${QL_INSTALL_BINDIR})
    endif()
    add_test(NAME quantlib_test_suite COMMAND ql_test_suite --log_level=message)


    add_executable(ql_benchmark quantlibbenchmark.cpp)
    set_target_properties(ql_benchmark PROPERTIES OUTPUT_NAME "quantlib-benchmark")
    target_link_libraries(ql_benchmark PRIVATE ql_test)
    if (QL_INSTALL_BENCHMARK)
        install(TARGETS ql_benchmark RUNTIME DESTINATION ${QL_INSTALL_BINDIR})
    endif()
endif()


install(FILES
  all.hpp
  americanpayoffatexpiry.hpp
  americanpayoffathit.hpp
  blackcalculator.hpp
  blackformula.hpp
  blackscholescalculator.hpp
  genericmodelengine.hpp
  greeks.hpp
  latticeshortratemodelengine.hpp
  mclongstaffschwartzengine.hpp
  mcsimulation.hpp
  DESTINATION include/ql/pricingengines)

add_subdirectory(asian)
add_subdirectory(barrier)
add_subdirectory(basket)
add_subdirectory(bond)
add_subdirectory(capfloor)
add_subdirectory(cliquet)
add_subdirectory(credit)
add_subdirectory(forward)
add_subdirectory(inflation)
add_subdirectory(lookback)
add_subdirectory(quanto)
add_subdirectory(swap)
add_subdirectory(swaption)
add_subdirectory(vanilla)

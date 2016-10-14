install(FILES
    all.hpp
    onefactormodel.hpp
    twofactormodel.hpp
    DESTINATION include/ql/models/shortrate)

add_subdirectory(calibrationhelpers)
add_subdirectory(onefactormodels)
add_subdirectory(twofactormodels)

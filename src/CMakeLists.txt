include_directories (.)
add_subdirectory(cxl)
add_subdirectory(algo)
add_subdirectory(generator)
add_subdirectory(utils)
add_subdirectory(utils/prototypes)

#TODO: arrange these files into a design
add_executable (CXLStressTester
AddressList.cpp
hammer.cpp
Manager.cpp
Target.cpp
Test.cpp
# Enable all this if we want monolotic app
algo/AlgoManager.cpp
algo/IAlgorithm.cpp
algo/MulWrStream.cpp
cxl/Cxl.cpp
generator/ITrafficGenerator.cpp
generator/CpuTrafficGenerator.cpp
generator/DeviceTrafficGenerator.cpp
utils/Logger.cpp
utils/Parser.cpp
utils/prototypes/Singleton.cpp
)


#monolitic app
target_link_libraries(CXLStressTester numa pci)

install(TARGETS CXLStressTester
        CONFIGURATIONS Linux
        RUNTIME DESTINATION ./
)


install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/scripts/utils/
        DESTINATION ./utils/
        CONFIGURATIONS Linux
        FILES_MATCHING 
        PATTERN "*enable*.sh"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/scripts/randomizer/
        DESTINATION ./utils/randomizer
        CONFIGURATIONS Linux
        PATTERN "*"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/hammers/
        DESTINATION ./hammers/
        CONFIGURATIONS Linux
        FILES_MATCHING 
        PATTERN "*.hammer"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)



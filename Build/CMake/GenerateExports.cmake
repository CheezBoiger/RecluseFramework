# Cmake generation for auto exports and imports

# Internal function to call for ExportThisLibrary.
# Need to define RECLUSE_FRAMEWORK_PATH in order to create the exports file.
function(ExportLibraryInternal TARGET_LIBRARY BUILD_PATH)
	set(RECLUSE_GENERATED_LIBRARY_DIRECTORY ${BUILD_PATH}/Include)
	set(EXPORT_LIBRARY_DIRECTORY ${RECLUSE_GENERATED_LIBRARY_DIRECTORY}/${TARGET_LIBRARY})
	set(EXPORT_LIBRARY_FILE ${EXPORT_LIBRARY_DIRECTORY}/${TARGET_LIBRARY}_exports.hpp)
	set(RECLUSE_EXPORT_NAME ${TARGET_LIBRARY})
	#if (NOT IS_DIRECTORY ${EXPORT_LIBRARY_DIRECTORY})
	#	file(MAKE_DIRECTORY ${EXPORT_LIBRARY_DIRECTORY})
	#endif()
	set(EXPORT_DLL_DEFINITION "${TARGET_LIBRARY}_DLL_EXPORT")
	add_definitions(-D"${EXPORT_DLL_DEFINITION}"=1)
	target_include_directories(${TARGET_LIBRARY} PUBLIC ${EXPORT_LIBRARY_DIRECTORY})
	configure_file(${RECLUSE_FRAMEWORK_PATH}/Build/CMake/exports.hpp.in ${EXPORT_LIBRARY_FILE})
endfunction()


# ExportThisLibrary is intended to be used for projects that want to export their api to other projects/libraries/applications.
# Need to define RECLUSE_FRAMEWORK_PATH in order to create the exports file.
function(ExportThisLibrary TARGET_LIBRARY BUILD_PATH)
	ExportLibraryInternal(${TARGET_LIBRARY} ${BUILD_PATH})
	#add_custom_command(TARGET ${TARGET_LIBRARY} PRE_BUILD COMMAND ExportLibraryInternal ARGS ${TARGET_LIBRARY})
endfunction()


# ImportThisLibrary is intended to be used for projects that want to import an api from another library/project to the current 
# target library that they are building/using.
function(ImportThisLibrary TARGET_LIBRARY IMPORT_LIBRARY BUILD_PATH)
	set(RECLUSE_GENERATED_LIBRARY_DIRECTORY ${BUILD_PATH})
	set(IMPORT_LIBRARY_DIRECTORY ${RECLUSE_GENERATED_LIBRARY_DIRECTORY}/${IMPORT_LIBRARY})
	target_include_directories(${TARGET_LIBRARY} PUBLIC ${IMPORT_LIBRARY_DIRECTORY})
endfunction()
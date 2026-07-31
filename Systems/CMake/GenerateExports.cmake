# Cmake generation for auto exports and imports

# Internal function to call for ExportThisLibrary.
function(ExportLibraryInternal TARGET_LIBRARY)
	set(RECLUSE_GENERATED_LIBRARY_DIRECTORY ${CMAKE_SOURCE_DIR}/Recluse/Build/Libraries)
	set(EXPORT_LIBRARY_DIRECTORY ${RECLUSE_GENERATED_LIBRARY_DIRECTORY}/${TARGET_LIBRARY})
	set(EXPORT_LIBRARY_FILE ${EXPORT_LIBRARY_DIRECTORY}/${TARGET_LIBRARY}_exports.hpp)
	set(RECLUSE_EXPORT_NAME ${TARGET_LIBRARY})
	if (NOT IS_DIRECTORY ${EXPORT_LIBRARY_DIRECTORY})
		file(MAKE_DIRECTORY ${EXPORT_LIBRARY_DIRECTORY})
	endif()
	set(EXPORT_DLL_DEFINITION "${TARGET_LIBRARY}_DLL_EXPORT")
	add_definitions(-D"${EXPORT_DLL_DEFINITION}"=1)
	target_include_directories(${TARGET_LIBRARY} PUBLIC ${EXPORT_LIBRARY_DIRECTORY})
	configure_file(${CMAKE_SOURCE_DIR}/Systems/CMake/exports.hpp.in ${EXPORT_LIBRARY_FILE})
endfunction()


# ExportThisLibrary is intended to be used for projects that want to export their api to other projects/libraries/applications.
function(ExportThisLibrary TARGET_LIBRARY)
	ExportLibraryInternal(${TARGET_LIBRARY})
	#add_custom_command(TARGET ${TARGET_LIBRARY} PRE_BUILD COMMAND ExportLibraryInternal ARGS ${TARGET_LIBRARY})
endfunction()


# ImportThisLibrary is intended to be used for projects that want to import an api from another library/project to the current 
# target library that they are building/using.
function(ImportThisLibrary TARGET_LIBRARY IMPORT_LIBRARY)
	set(RECLUSE_GENERATED_LIBRARY_DIRECTORY ${CMAKE_SOURCE_DIR}/Recluse/Build/Libraries)
	set(IMPORT_LIBRARY_DIRECTORY ${RECLUSE_GENERATED_LIBRARY_DIRECTORY}/${IMPORT_LIBRARY})
	target_include_directories(${TARGET_LIBRARY} PUBLIC ${IMPORT_LIBRARY_DIRECTORY})
endfunction()
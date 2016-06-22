
find_program (JAVA java)
find_program (IMAGEMAGICK_CONVERT convert)
find_program (SOUND_EXCHANGE sox)

set (LMSASM_DIR ${CMAKE_SOURCE_DIR}/lmssrc/adk/lmsasm)
set (RAW2RSF ${CMAKE_SOURCE_DIR}/lmssrc/raw2rsf.sh)

macro (add_lms_app TARGET)
    set (options ARCHIVE)
    set (one_value_args PATH ICON INSTALL_TO)
    set (multi_value_args PROGRAMS PROGRAMS_TASM GRAPHICS SOUNDS)
    cmake_parse_arguments (LMSSRC
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )
    if (LMSSRC_ICON)
        list (APPEND LMSSRC_GRAPHICS icon)
        set (icon_SOURCE ${LMSSRC_ICON})
    endif ()

    configure_file (${CMAKE_SOURCE_DIR}/lms2012/bytecodes.h
        ${LMSASM_DIR}/bytecodes.h COPYONLY)
    configure_file (${CMAKE_SOURCE_DIR}/lms2012/bytecodes.c
        ${LMSASM_DIR}/bytecodes.c COPYONLY)

    foreach (program ${LMSSRC_PROGRAMS})
        set (lms ${CMAKE_CURRENT_SOURCE_DIR}/${program}.lms)
        set (rbf ${CMAKE_CURRENT_BINARY_DIR}/${program}.rbf)

        add_custom_command (OUTPUT ${rbf}
            COMMAND ${JAVA}
                -jar ${LMSASM_DIR}/assembler.jar
                ${CMAKE_CURRENT_SOURCE_DIR}/${program}
            COMMAND mv
                ${CMAKE_CURRENT_SOURCE_DIR}/${program}.rbf
                ${rbf}
            DEPENDS
                ${lms}
                ${LMSASM_DIR}/bytecodes.c
                ${LMSASM_DIR}/bytecodes.h
            WORKING_DIRECTORY
                ${LMSASM_DIR}
        )
        list (APPEND LMSSRC_RBF_LIST ${rbf})
    endforeach ()

    foreach (graphic ${LMSSRC_GRAPHICS})
        # some generated files do not have the same name as the source file
        set (source ${${graphic}_SOURCE})
        if (NOT source)
            set (source ${graphic})
        endif ()

        set (bmp ${CMAKE_CURRENT_SOURCE_DIR}/${source}.bmp)
        set (rgf ${CMAKE_CURRENT_BINARY_DIR}/${graphic}.rgf)

        add_custom_command (OUTPUT ${rgf}
            COMMAND ${IMAGEMAGICK_CONVERT} ${bmp} ${rgf}
            DEPENDS ${bmp}
        )
        list (APPEND LMSSRC_RGF_LIST ${rgf})
    endforeach ()

    foreach (sound ${LMSSRC_SOUNDS})
        set (wav ${CMAKE_CURRENT_SOURCE_DIR}/${sound}.wav)
        set (raw ${CMAKE_CURRENT_BINARY_DIR}/${sound}.raw)
        set (rsf ${CMAKE_CURRENT_BINARY_DIR}/${sound}.rsf)

        add_custom_command (OUTPUT ${raw}
            COMMAND ${SOUND_EXCHANGE}
                ${wav}
                --bits 8
                --channels 1
                --encoding unsigned-integer
                --rate 8000
                ${raw}
            DEPENDS
                ${wav}
        )
        add_custom_command (OUTPUT ${rsf}
            COMMAND ${RAW2RSF}
                ${raw}
                ${rsf}
            DEPENDS
                ${raw}
                ${RAW2RSF}
        )
        list (APPEND LMSSRC_RSF_LIST ${rsf})
    endforeach ()

    add_custom_target (${TARGET} ALL
        DEPENDS
            ${LMSSRC_RBF_LIST}
            ${LMSSRC_RGF_LIST}
            ${LMSSRC_RSF_LIST}
    )

endmacro ()

add_subdirectory (Bluetooth)
add_subdirectory ("Brick Datalog")
add_subdirectory ("Brick Info")
add_subdirectory ("Brick Program")
add_subdirectory (Debug)
add_subdirectory (Demo)
add_subdirectory ("IR Control")
add_subdirectory ("Motor Control")
add_subdirectory ("Port View")
add_subdirectory (SelfTest)
add_subdirectory (Sleep)
add_subdirectory (TEST)
add_subdirectory (Test)
add_subdirectory (Volume)
add_subdirectory (WiFi)
add_subdirectory (tst)
add_subdirectory (ui)
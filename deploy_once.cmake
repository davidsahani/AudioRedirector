# deploy_once.cmake

set(MARKER_FILE "${TARGET_BIN_DIR}_deployed")

if(EXISTS "${MARKER_FILE}")
    return()  # Already deployed, skip this step
endif()

message(STATUS "[deploy_once] Running windeployqt...")

# Run windeployqt.exe
execute_process(
    COMMAND "${QT_BIN_DIR}/windeployqt.exe"
        --no-translations
        --no-system-d3d-compiler
        --no-system-dxc-compiler
        --no-compiler-runtime
        --no-opengl-sw
        --no-ffmpeg
        --exclude-plugins qgif,qtuiotouchplugin,qnetworklistmanager,qmodernwindowsstyle,qcertonlybackend,qschannelbackend
        --skip-plugin-types generic,networkinformation,styles,tls
        "${TARGET_FILE}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    RESULT_VARIABLE _deploy_result
)

if(NOT _deploy_result EQUAL 0)
    message(FATAL_ERROR "[deploy_once] windeployqt failed with exit code ${_deploy_result}")
else()
    message(STATUS "[deploy_once] windeployqt Done.")
endif()

string(TIMESTAMP NOW "%Y-%m-%d %H:%M:%S")
file(WRITE "${MARKER_FILE}" "Deployed at ${NOW}")

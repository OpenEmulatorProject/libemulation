add_library(emulation-hal
  ${LIBEMULATION_HAL_DIR}/HIDJoystick.cpp
  ${LIBEMULATION_HAL_DIR}/OEMatrix3.cpp
  ${LIBEMULATION_HAL_DIR}/OEVector.cpp
  ${LIBEMULATION_HAL_DIR}/OpenGLCanvas.cpp
  ${LIBEMULATION_HAL_DIR}/PAAudio.cpp)

set(LIBEMULATION_HAL_INCLUDE_DIRS
  ${LIBEMULATION_HAL_DIR}
)

set(LIBEMULATION_HAL emulation-hal)
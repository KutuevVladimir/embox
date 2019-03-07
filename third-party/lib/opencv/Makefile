PKG_VER  := 4.0.1
PKG_SOURCES := https://github.com/opencv/opencv/archive/$(PKG_VER).zip
PKG_MD5     := eb367fc73de66e6d8808e8712138c6b4

include $(EXTBLD_LIB)

EMBOX_TARGET_CC =$(shell which $(EMBOX_CROSS_COMPILE)gcc)
EMBOX_TARGET_CXX=$(shell which $(EMBOX_CROSS_COMPILE)g++)

ifeq (,$(EMBOX_TARGET_CC))
$(error Cannot find C compiler: $(EMBOX_CROSS_COMPILE)gcc)
endif
ifeq (,$(EMBOX_TARGET_CXX))
$(error Cannot find C++ compiler: $(EMBOX_CROSS_COMPILE)g++)
endif

AT=

OPENCV_DIR   = $(BUILD_DIR)
OPENCV_FLAGS = $(OPENCV_DIR)/flags

EMBOX_IMPORTED_CPPFLAGS += -I../../stlport
EMBOX_IMPORTED_CPPFLAGS += -save-temps=obj
EMBOX_IMPORTED_CPPFLAGS += -include $(abspath .)/stlport_embox_compat.h
EMBOX_IMPORTED_CPPFLAGS += -DUSE_SPRINTF_INSTEAD
EMBOX_IMPORTED_CPPFLAGS += $(filter -I%,$(EMBOX_CPPFLAGS))
EMBOX_IMPORTED_CPPFLAGS += $(filter -nostdinc,$(EMBOX_CPPFLAGS))
EMBOX_IMPORTED_CPPFLAGS += $(filter -D%,$(EMBOX_CPPFLAGS))
EMBOX_IMPORTED_CPPFLAGS += $(filter -U%,$(EMBOX_CPPFLAGS))
#EMBOX_IMPORTED_CPPFLAGS := $(filter-out -D"__impl_x%,$(EMBOX_IMPORTED_CPPFLAGS))

EMBOX_IMPORTED_CFLAGS   += $(EMBOX_IMPORTED_CPPFLAGS)
EMBOX_IMPORTED_CFLAGS   += $(filter -fno-common,$(EMBOX_CFLAGS))
EMBOX_IMPORTED_CFLAGS   += $(filter -march%,$(EMBOX_CFLAGS))
EMBOX_IMPORTED_CFLAGS   += $(filter -m%,$(EMBOX_CFLAGS))
EMBOX_IMPORTED_CFLAGS   += $(filter -fno-stack-protector,$(EMBOX_CFLAGS))
EMBOX_IMPORTED_CFLAGS   += $(filter -g,$(EMBOX_CFLAGS))
EMBOX_IMPORTED_CFLAGS   += $(filter -mno-unaligned-access,$(EMBOX_CFLAGS))
EMBOX_IMPORTED_CFLAGS   += -I$(abspath .)/include

EMBOX_IMPORTED_CXXFLAGS += $(EMBOX_IMPORTED_CPPFLAGS)
EMBOX_IMPORTED_CXXFLAGS += $(subst .,$(ROOT_DIR),$(filter -I%,$(EMBOX_CXXFLAGS)))
EMBOX_IMPORTED_CXXFLAGS += $(filter -fno-common,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += $(filter -march%,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += $(filter -m32,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += $(filter -fno-stack-protector,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += $(filter -g,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += $(filter -fno-rtti,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += $(filter -fno-exceptions,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += $(filter -fno-threadsafe-statics,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += $(filter -mno-unaligned-access,$(EMBOX_CXXFLAGS))
EMBOX_IMPORTED_CXXFLAGS += -I$(abspath .)/include

EMBOX_IMPORTED_MAKEFLAGS += $(filter -j,$(EMBOX_MAKEFLAGS))
ifneq ($(filter -j,$(EMBOX_MAKEFLAGS)),)
EMBOX_IMPORTED_MAKEFLAGS += $(shell nproc)
endif
# no wonder the following doesn't work
# EMBOX_IMPORTED_MAKEFLAGS += $(filter --jobserver-fds=%,$(EMBOX_MAKEFLAGS))

ifneq (,$(filter -fno-rtti,$(EMBOX_CXXFLAGS)))
EMBOX_RTTI_FLAG          = --without-rtti
else
EMBOX_RTTI_FLAG          =
endif

EMBOX_DERIVED_CFLAGS    += $(EMBOX_IMPORTED_CFLAGS)
EMBOX_DERIVED_CXXFLAGS  += $(EMBOX_IMPORTED_CXXFLAGS)
EMBOX_DERIVED_MAKEFLAGS += $(EMBOX_IMPORTED_MAKEFLAGS)
EMBOX_DERIVED_RTTI_FLAG  = $(EMBOX_RTTI_FLAG)
EMBOX_DERIVED_CC         = $(EMBOX_TARGET_CC)
EMBOX_DERIVED_CXX        = $(EMBOX_TARGET_CXX)

INCLUDE_FLAGS := -I$(SRC_DIR)/include -I$(SRC_DIR)/compat/cxx/include -nostdinc
INCLUDE_FLAGS += -I$(SRC_DIR)/compat/libc/include -I$(SRC_DIR)/compat/posix/include
INCLUDE_FLAGS += -I$(SRC_DIR)/arch/$(ARCH)/include
INCLUDE_FLAGS += -I$(SRC_DIR)/arch/$(ARCH)/include
INCLUDE_FLAGS += -I$(EXTERNAL_BUILD_DIR)/third_party/gcc/core/gcc-4.8.1/libstdc++-v3/include/std/
INCLUDE_FLAGS += -I$(EXTERNAL_BUILD_DIR)/third_party/gcc/core/install/include/
INCLUDE_FLAGS += -I$(EXTERNAL_BUILD_DIR)/third_party/gcc/core/gcc-4.8.1/libstdc++-v3/include/
INCLUDE_FLAGS += -I$(EXTERNAL_BUILD_DIR)/third_party/gcc/core/gcc-4.8.1/libstdc++-v3/include/c_std/
INCLUDE_FLAGS += -I$(EXTERNAL_BUILD_DIR)/third_party/gcc/core/gcc-4.8.1/libstdc++-v3/include/c_global/
INCLUDE_FLAGS += -I$(EXTERNAL_BUILD_DIR)/third_party/gcc/core/gcc-4.8.1/libstdc++-v3/config/cpu/generic/opt/
INCLUDE_FLAGS += -I$(EXTERNAL_BUILD_DIR)/third_party/gcc/core/build/arm-none-eabi/libstdc++-v3/include/arm-none-eabi/

OPENCV_CXX_FLAGS := $(INCLUDE_FLAGS)

OPENCV_FLAGS = \
		-DBUILD_SHARED_LIBS=OFF -DBUILD_opencv_python_bindings_generator=OFF -DBUILD_JAVA=OFF -DBUILD_TESTS=OFF -DBUILD_opencv_apps=OFF BUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_ml=OFF -DWITH_FFMPEG=OFF -DWITH_EIGEN=OFF -DBUILD_IPP_IW=OFF -DBUILD_ITT=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_PROTOBUF=OFF \
		-DBUILD_opencv_calib3d=OFF \
		-DBUILD_opencv_dnn=OFF \
		-DBUILD_opencv_features2d=OFF \
		-DBUILD_opencv_flann=OFF \
		-DBUILD_opencv_gapi=OFF \
		-DBUILD_opencv_highgui=OFF \
		-DBUILD_opencv_imgcodecs=OFF \
		-DBUILD_opencv_imgproc=OFF \
		-DBUILD_opencv_java_bindings_generator=OFF \
		-DBUILD_opencv_objdetect=OFF \
		-DBUILD_opencv_photo=OFF \
		-DBUILD_opencv_stitching=OFF \
		-DBUILD_opencv_ts=OFF \
		-DBUILD_opencv_video=OFF \
		-DBUILD_opencv_videoio=OFF \
		-DCPACK_BINARY_STGZ=OFF \
		-DCPACK_BINARY_TGZ=OFF \
		-DCPACK_BINARY_TZ=OFF \
		-DCPACK_SOURCE_TBZ2=OFF \
		-DCPACK_SOURCE_TGZ=OFF \
		-DCPACK_SOURCE_TXZ=OFF \
		-DCPACK_SOURCE_TZ=OFF \
		-DCV_ENABLE_INTRINSICS=OFF \
		-DCV_TRACE=OFF \
		-DENABLE_CCACHE=OFF \
		-DENABLE_OMIT_FRAME_POINTER=OFF \
		-DENABLE_PIC=OFF \
		-DENABLE_PRECOMPILED_HEADERS=OFF \
		-DOPENCL_FOUND=OFF \
		-DOPENCV_DNN_OPENCL=OFF \
		-DOPENCV_GENERATE_SETUPVARS=OFF \
		-DVIDEOIO_ENABLE_PLUGINS=OFF \
		-DVIDEOIO_ENABLE_STRICT_PLUGIN_CHECK=OFF \
		-DWITH_1394=OFF \
		-DWITH_CAROTENE=OFF \
		-DWITH_ADE=OFF \
		-DWITH_GSTREAMER=OFF \
		-DWITH_GTK=OFF \
		-DWITH_IMGCODEC_HDR=OFF \
		-DWITH_IMGCODEC_PFM=OFF \
		-DWITH_IMGCODEC_PXM=OFF \
		-DWITH_IMGCODEC_SUNRASTER=OFF \
		-DWITH_IPP=OFF \
		-DWITH_ITT=OFF \
		-DWITH_JASPER=OFF \
		-DWITH_JPEG=OFF \
		-DWITH_LAPACK=OFF \
		-DWITH_OPENCL=OFF \
		-DWITH_OPENCLAMDBLAS=OFF \
		-DWITH_OPENCLAMDFFT=OFF \
		-DWITH_OPENEXR=OFF \
		-DWITH_PNG=OFF \
		-DWITH_PROTOBUF=OFF \
		-DWITH_PTHREADS_PF=OFF \
		-DWITH_QUIRC=OFF \
		-DWITH_TIFF=OFF \
		-DWITH_V4L=OFF \
		-DWITH_VTK=OFF \
		-DWITH_WEBP=OFF \
		-DCMAKE_SYSTEM_PROCESSOR:STRING=$(AUTOCONF_TARGET_TRIPLET) \
		-DCMAKE_C_COMPILER:PATH=$(EMBOX_GCC) \
		-DCMAKE_CXX_COMPILER:PATH=$(EMBOX_GXX) \
		-DCMAKE_AR:STRING=ar \
		-DCMAKE_SYSTEM_NAME:STRING=Generic \
		-DCMAKE_BUILD_TYPE:STRING=Debug \
		-DCMAKE_VERBOSE_MAKEFILE:BOOL=on \
		-DCMAKE_BUILD_TYPE:STRING=Debug \
		-DC_HAS_MFPU_NEON=0

all:

.PHONY: all

$(CONFIGURE) :
	cd $(BUILD_DIR); cp $(THIRDPARTY_DIR)/lib/opencv/CMakeLists.txt $(BUILD_DIR)/opencv-4.0.1/

$(BUILD) :
	cd $(BUILD_DIR) && mkdir build; cd build && cmake -DCMAKE_CXX_FLAGS="$(OPENCV_CXX_FLAGS)" $(OPENCV_FLAGS) ../opencv-4.0.1/ && make

$(INSTALL) :
	cd $(BUILD_DIR) && cp build/lib/libopencv_core.a install

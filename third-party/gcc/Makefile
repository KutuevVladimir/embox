
PKG_NAME := gcc
PKG_VER  := 8.3.0

PKG_SOURCES := http://ftp.gnu.org/gnu/gcc/$(PKG_NAME)-$(PKG_VER)/$(PKG_NAME)-$(PKG_VER).tar.gz
PKG_MD5     := 9972f8c24c02ebcb5a342c1b30de69ff
#PKG_PATCHES := pkg_patch.txt

include $(EXTBLD_LIB)

ifeq (arm,$(EMBOX_ARCH))
AUTOCONF_TARGET_TRIPLET := arm-none-eabi
else ifeq (sparc,$(EMBOX_ARCH))
AUTOCONF_TARGET_TRIPLET := sparc-elf
else ifeq (x86,$(EMBOX_ARCH))
AUTOCONF_TARGET_TRIPLET := i386-pc-elf
else
$(error Unsupported architecture: $(EMBOX_ARCH))
endif

target_libgcc_cppflags := -DLIBGCC2_HAS_XF_MODE=0 \
	-DDO_GLOBAL_CTORS_BODY -DDO_GLOBAL_DTORS_BODY \
	-DNAME__MAIN -DSYMBOL__MAIN=__embox_register_eh_frame_once
cppflags = -I$(ROOT_DIR)/third-party/gcc/include \
	$${EMBOX_IMPORTED_CPPFLAGS} \
	-I$(PKG_BUILD_DIR)/gcc/include -nostdinc #-I/usr/include -I/usr/lib/gcc/x86_64-pc-linux-gnu/8.2.1/include/ -I/usr/lib/gcc/arm-none-eabi/8.3.0/include/
PKG_BUILD_DIR  := $(BUILD_DIR)/build

$(CONFIGURE) :
	mkdir -p $(PKG_BUILD_DIR) && cd $(PKG_BUILD_DIR) && ( \
		. $(EMBOX_GCC_ENV) && $(PKG_SOURCE_DIR)/configure \
			--target=$(AUTOCONF_TARGET_TRIPLET) \
			--disable-multilib \
			--disable-shared \
			--disable-tls \
			--disable-nls \
			--disable-libssp \
			--disable-isl-version-check \
			--disable-cloog-version-check \
			--without-newlib \
			--disable-libgomp \
			--disable-libmudflap \
			--enable-languages=c,c++ \
	)
	touch $@

$(BUILD) :
	cd $(PKG_BUILD_DIR) && ( \
		. $(EMBOX_GCC_ENV) && $(MAKE) MAKEFLAGS='$${EMBOX_IMPORTED_MAKEFLAGS}' \
			CC_FOR_TARGET=$${EMBOX_CROSS_COMPILE}gcc \
			GCC_FOR_TARGET=$${EMBOX_CROSS_COMPILE}gcc \
			CXX_FOR_TARGET=$${EMBOX_CROSS_COMPILE}g++ \
			RAW_CXX_FOR_TARGET=$${EMBOX_CROSS_COMPILE}g++ \
			AR_FOR_TARGET=ar \
			RANLIB_FOR_TARGET=ranlib \
         	TARGET-zlib="LINKER=true CC=true AR=true RANLIB=touch all" \
         	all-gcc all-target-libgcc all-target-libstdc++-v3; \
	)
	touch $@

$(INSTALL) :
	mkdir -p $(PKG_INSTALL_DIR)/include $(PKG_INSTALL_DIR)/include/bits
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libgcc/unwind.h $(PKG_INSTALL_DIR)/include/
	cp $(PKG_SOURCE_DIR)/libstdc++-v3/libsupc++/cxxabi.h $(PKG_INSTALL_DIR)/include/
	cp $(PKG_SOURCE_DIR)/libstdc++-v3/libsupc++/exception $(PKG_INSTALL_DIR)/include/
	cp $(PKG_SOURCE_DIR)/libstdc++-v3/libsupc++/initializer_list $(PKG_INSTALL_DIR)/include/
	cp $(PKG_SOURCE_DIR)/libstdc++-v3/libsupc++/new $(PKG_INSTALL_DIR)/include/
	cp $(PKG_SOURCE_DIR)/libstdc++-v3/libsupc++/typeinfo $(PKG_INSTALL_DIR)/include/
	cp $(PKG_SOURCE_DIR)/libstdc++-v3/libsupc++/atomic_lockfree_defines.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/bits/cxxabi_forced.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/bits/exception_defines.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/bits/exception_ptr.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/bits/hash_bytes.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/bits/nested_exception.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/$(AUTOCONF_TARGET_TRIPLET)/bits/c++config.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/$(AUTOCONF_TARGET_TRIPLET)/bits/cpu_defines.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/$(AUTOCONF_TARGET_TRIPLET)/bits/cxxabi_tweaks.h $(PKG_INSTALL_DIR)/include/bits/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/include/$(AUTOCONF_TARGET_TRIPLET)/bits/os_defines.h $(PKG_INSTALL_DIR)/include/bits/
	mkdir -p $(PKG_INSTALL_DIR)/lib
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libgcc/libgcc.a $(PKG_INSTALL_DIR)/lib/
	cp $(PKG_BUILD_DIR)/$(AUTOCONF_TARGET_TRIPLET)/libstdc++-v3/libsupc++/.libs/libsupc++.a $(PKG_INSTALL_DIR)/lib
	touch $@

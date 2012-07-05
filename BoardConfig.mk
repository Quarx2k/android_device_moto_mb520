# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This file sets variables that control the way modules are built
# thorughout the system. It should not be used to conditionally
# disable makefiles (the proper mechanism to control what gets
# included in a build is to use PRODUCT_PACKAGES in a product
# definition file).
#

# WARNING: This line must come *before* including the proprietary
# variant, so that it gets overwritten by the parent (which goes
# against the traditional rules of inheritance).

BOARD_USES_GENERIC_AUDIO := false

TARGET_NO_RADIOIMAGE := true
TARGET_NO_BOOTLOADER := true
TARGET_NO_PREINSTALL := true
TARGET_BOOTLOADER_BOARD_NAME := kobe

# Board properties
TARGET_BOARD_PLATFORM := omap3
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_GLOBAL_CFLAGS += -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp
TARGET_GLOBAL_CPPFLAGS += -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp
TARGET_OMAP3 := true
COMMON_GLOBAL_CFLAGS += -DTARGET_OMAP3 -DOMAP_COMPAT -DBINDER_COMPAT
ARCH_ARM_HAVE_TLS_REGISTER := false

# Wifi related defines
BOARD_WLAN_DEVICE           := wl1271
WPA_SUPPLICANT_VERSION      := VER_0_6_X
BOARD_WPA_SUPPLICANT_DRIVER := CUSTOM
WIFI_DRIVER_MODULE_PATH     := "/system/lib/modules/tiwlan_drv.ko"
WIFI_DRIVER_MODULE_NAME     := tiwlan_drv
WIFI_DRIVER_FW_STA_PATH     := "/system/etc/wifi/fw_wlan1271.bin"
WIFI_FIRMWARE_LOADER        := wlan_loader
PRODUCT_WIRELESS_TOOLS      := true
BOARD_SOFTAP_DEVICE         := wl1271
AP_CONFIG_DRIVER_WILINK     := true
WIFI_DRIVER_FW_AP_PATH      := "/system/etc/wifi/fw_tiwlan_ap.bin"
WPA_SUPPL_APPROX_USE_RSSI   := true
WPA_SUPPL_WITH_SIGNAL_POLL  := true
# CM9
WIFI_DRIVER_LOADER_DELAY    := 200000
WIFI_AP_DRIVER_MODULE_PATH  := "/system/lib/modules/tiap_drv.ko"
WIFI_AP_DRIVER_MODULE_NAME  := tiap_drv
WIFI_AP_FIRMWARE_LOADER     := wlan_ap_loader
WIFI_AP_DRIVER_MODULE_ARG   := ""
BOARD_HOSTAPD_SERVICE_NAME  := hostap_netd
BOARD_HOSTAPD_NO_ENTROPY    := true
BOARD_HOSTAPD_DRIVER        := true
BOARD_HOSTAPD_DRIVER_NAME   := wilink
BOARD_HOSTAPD_TIAP_ROOT     := system/wlan/ti/WiLink_AP

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
TARGET_CUSTOM_BLUEDROID := ../../../device/moto/kobe/bluedroid.c

# Usb Specific
BOARD_CUSTOM_USB_CONTROLLER := ../../device/moto/kobe/UsbController.cpp
BOARD_USE_USB_MASS_STORAGE_SWITCH := true
BOARD_MASS_STORAGE_FILE_PATH := "/sys/devices/platform/usb_mass_storage/lun0/file"
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/platform/usb_mass_storage/lun0/file"
BOARD_MTP_DEVICE := "/dev/mtp"
BOARD_VOLD_EMMC_SHARES_DEV_MAJOR := true

# Build options
BOARD_BOOTIMAGE_MAX_SIZE := $(call image-size-from-data-size,0x00280000)
BOARD_RECOVERYIMAGE_MAX_SIZE := $(call image-size-from-data-size,0x00500000)
BOARD_SYSTEMIMAGE_MAX_SIZE := $(call image-size-from-data-size,0x07500000)
BOARD_USERDATAIMAGE_MAX_SIZE := $(call image-size-from-data-size,0x04ac0000)
BOARD_FLASH_BLOCK_SIZE := 131072
TARGET_CUSTOM_RELEASETOOL := ./device/moto/kobe/releasetools/squisher

# OMX Stuff
HARDWARE_OMX := true
TARGET_USE_OMX_RECOVERY := true
TARGET_USE_OMAP_COMPAT  := true
BUILD_WITH_TI_AUDIO := 1
BUILD_PV_VIDEO_ENCODERS := 1
# Required for videos (youtube)
BOARD_WITHOUT_PIXEL_FORMAT_YV12 := true
BOARD_CUSTOM_OMX_16BPP_YUV := 27

# Bootmenu
BOARD_USES_BOOTMENU := true
BOARD_WITH_CPCAP    := true
BOARD_MMC_DEVICE    := /dev/block/mmcblk1
BOARD_SDCARD_DEVICE_PRIMARY   := /dev/block/mmcblk0p1
BOARD_SDCARD_DEVICE_SECONDARY := /dev/block/mmcblk0
BOARD_SDEXT_DEVICE  := /dev/block/mmcblk0p2
BOARD_SYSTEM_DEVICE := /dev/block/mmcblk1p21
BOARD_DATA_DEVICE   := /dev/block/mmcblk1p25

# Recovery
TARGET_RECOVERY_PIXEL_FORMAT := "BGRA_8888"
BOARD_RECOVERY_IGNORE_BOOTABLES := true
BOARD_HAS_NO_MISC_PARTITION := true
BOARD_HAS_NO_SELECT_BUTTON := true
BOARD_HAS_SMALL_RECOVERY := true
BOARD_HAS_LARGE_FILESYSTEM := true
BOARD_NEVER_UMOUNT_SYSTEM := true
BOARD_CUSTOM_RECOVERY_KEYMAPPING:= ../../device/moto/kobe/recovery_keys.c
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext3
TARGET_USERIMAGES_USE_EXT4 := false
#TARGET_RECOVERY_UI_LIB := librecovery_ui_generic
#TARGET_RECOVERY_UPDATER_LIBS += librecovery_updater_generic
TARGET_RECOVERY_PRE_COMMAND := "/system/bootmenu/script/reboot_command.sh"
TARGET_RECOVERY_PRE_COMMAND_CLEAR_REASON := true

# FM Radio (not ready in ICS)
# BOARD_HAVE_FM_RADIO := true
# BOARD_FM_DEVICE := wl1271
# COMMON_GLOBAL_CFLAGS += -DHAVE_FM_RADIO

# Egl Specific
USE_OPENGL_RENDERER := true
BOARD_EGL_CFG := device/moto/kobe/egl.cfg
DEFAULT_FB_NUM := 0
BOARD_USE_YUV422I_DEFAULT_COLORFORMAT := true
BOARD_USES_OVERLAY := true
BOARD_WITHOUT_HW_COMPOSER := true

# Camera
USE_CAMERA_STUB := false
BOARD_OVERLAY_BASED_CAMERA_HAL := true

# Other..
BOARD_USE_LEGACY_TOUCHSCREEN := true
ENABLE_SENSORS_COMPAT := true
TARGET_PROXIMITY_SENSOR_LIMIT := 0x1F
BOARD_USES_AUDIO_LEGACY := true
TARGET_PROVIDES_LIBAUDIO := true
BOARD_USE_KINETO_COMPATIBILITY := true

# Beware: set only prebuilt OR source+config
TARGET_PREBUILT_KERNEL := $(ANDROID_BUILD_TOP)/device/moto/kobe/kernel

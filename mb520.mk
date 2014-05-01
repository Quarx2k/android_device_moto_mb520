#
# Copyright (C) 2011 The Android Open Source Project
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

#
# This is the product configuration for a generic Motorola Defy (jordan)
#
$(call inherit-product, device/moto/jordan-common/jordan.mk)

device_path = device/moto/mb520
DEVICE_PACKAGE_OVERLAYS += device/moto/mb520/overlay

## (3)  Finally, the least specific parts, i.e. the non-GSM-specific aspects
PRODUCT_PROPERTY_OVERRIDES += \
	ro.media.capture.maxres=3m \
	ro.media.capture.classification=classA \

PRODUCT_COPY_FILES += \
	${device_path}/media_profiles.xml:system/etc/media_profiles.xml \
	${device_path}/devtree:system/bootstrap/2nd-boot/devtree \

# Include non-opensource parts
$(call inherit-product, vendor/motorola/kobe/kobe-vendor.mk)


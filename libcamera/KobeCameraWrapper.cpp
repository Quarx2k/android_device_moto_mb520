/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "KobeCameraWrapper"

#include <cmath>
#include <dlfcn.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <camera/Camera.h>
#include "KobeCameraWrapper.h"

namespace android {

wp<KobeCameraWrapper> KobeCameraWrapper::singleton;

static sp<CameraHardwareInterface>
openMotoInterface(const char *libName, const char *funcName)
{
    sp<CameraHardwareInterface> interface;
    void *libHandle = ::dlopen(libName, RTLD_NOW);

    if (libHandle != NULL) {
        typedef sp<CameraHardwareInterface> (*OpenCamFunc)();
        OpenCamFunc func = (OpenCamFunc) ::dlsym(libHandle, funcName);
        if (func != NULL) {
            interface = func();
        } else {
            ALOGE("Could not find library entry point!");
        }
    } else {
        ALOGE("dlopen() error: %s\n", dlerror());
    }

    return interface;
}

sp<KobeCameraWrapper> KobeCameraWrapper::createInstance(int cameraId)
{
    ALOGV("%s :", __func__);
    if (singleton != NULL) {
        sp<KobeCameraWrapper> hardware = singleton.promote();
        if (hardware != NULL) {
            return hardware;
        }
    }

    CameraType type = CAM_SOC;
    sp<CameraHardwareInterface> motoInterface;
    sp<KobeCameraWrapper> hardware;
    motoInterface = openMotoInterface("libkobecamera.so", "_ZN7android16CameraHalSocImpl14createInstanceEv");
    type = CAM_SOC;
    if (motoInterface != NULL) {
        hardware = new KobeCameraWrapper(motoInterface, type);
        singleton = hardware;
    } else {
        ALOGE("Could not open hardware interface");
    }

    return hardware;
}

KobeCameraWrapper::KobeCameraWrapper(sp<CameraHardwareInterface>& motoInterface, CameraType type) :
    mMotoInterface(motoInterface),
    mCameraType(type),
    mNotifyCb(NULL),
    mDataCb(NULL),
    mDataCbTimestamp(NULL),
    mCbUserData(NULL)
{
}

KobeCameraWrapper::~KobeCameraWrapper()
{
}

sp<IMemoryHeap>
KobeCameraWrapper::getPreviewHeap() const
{
    return mMotoInterface->getPreviewHeap();
}

sp<IMemoryHeap>
KobeCameraWrapper::getRawHeap() const
{
    return mMotoInterface->getRawHeap();
}

void
KobeCameraWrapper::setCallbacks(notify_callback notify_cb,
                                  data_callback data_cb,
                                  data_callback_timestamp data_cb_timestamp,
                                  void* user)
{
    mNotifyCb = notify_cb;
    mDataCb = data_cb;
    mDataCbTimestamp = data_cb_timestamp;
    mCbUserData = user;

    if (mNotifyCb != NULL) {
        notify_cb = &KobeCameraWrapper::notifyCb;
    }
    if (mDataCb != NULL) {
        data_cb = &KobeCameraWrapper::dataCb;
    }
    if (mDataCbTimestamp != NULL) {
        data_cb_timestamp = &KobeCameraWrapper::dataCbTimestamp;
    }

    mMotoInterface->setCallbacks(notify_cb, data_cb, data_cb_timestamp, this);
}

void
KobeCameraWrapper::notifyCb(int32_t msgType, int32_t ext1, int32_t ext2, void* user)
{
    KobeCameraWrapper *_this = (KobeCameraWrapper *) user;
    user = _this->mCbUserData;


    _this->mNotifyCb(msgType, ext1, ext2, user);
}

void
KobeCameraWrapper::dataCb(int32_t msgType, const sp<IMemory>& dataPtr, void* user)
{
    KobeCameraWrapper *_this = (KobeCameraWrapper *) user;
    user = _this->mCbUserData;

    if (msgType == CAMERA_MSG_COMPRESSED_IMAGE) {
        _this->fixUpBrokenGpsLatitudeRef(dataPtr);
    }

    _this->mDataCb(msgType, dataPtr, user);
 }

void
KobeCameraWrapper::dataCbTimestamp(nsecs_t timestamp, int32_t msgType,
                                     const sp<IMemory>& dataPtr, void* user)
{
    KobeCameraWrapper *_this = (KobeCameraWrapper *) user;
    user = _this->mCbUserData;

    _this->mDataCbTimestamp(timestamp, msgType, dataPtr, user);
}

/*
 * Motorola's libcamera fails in writing the GPS latitude reference
 * tag properly. Instead of writing 'N' or 'S', it writes 'W' or 'E'.
 * Below is a very hackish workaround for that: We search for the GPS
 * latitude reference tag by pattern matching into the first couple of
 * data bytes. As the output format of Motorola's libcamera is static,
 * this should be fine until Motorola fixes their lib.
 */
void
KobeCameraWrapper::fixUpBrokenGpsLatitudeRef(const sp<IMemory>& dataPtr)
{
    ssize_t offset;
    size_t size;
    sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
    uint8_t *data = (uint8_t*)heap->base();

    if (data != NULL) {
        data += offset;

        /* scan first 512 bytes for GPS latitude ref marker */
        static const unsigned char sLatitudeRefMarker[] = {
            0x01, 0x00, /* GPS Latitude ref tag */
            0x02, 0x00, /* format: string */
            0x02, 0x00, 0x00, 0x00 /* 2 bytes long */
        };

        for (size_t i = 0; i < 512 && i < (size - 10); i++) {
            if (memcmp(data + i, sLatitudeRefMarker, sizeof(sLatitudeRefMarker)) == 0) {
                char *ref = (char *) (data + i + sizeof(sLatitudeRefMarker));
                if ((*ref == 'W' || *ref == 'E') && *(ref + 1) == '\0') {
                    ALOGI("Found broken GPS latitude ref marker, offset %d, item %c",
                         i + sizeof(sLatitudeRefMarker), *ref);
                    *ref = (*ref == 'W') ? 'N' : 'S';
                }
                break;
            }
        }
    }
}

void
KobeCameraWrapper::enableMsgType(int32_t msgType)
{
    mMotoInterface->enableMsgType(msgType);
}

void
KobeCameraWrapper::disableMsgType(int32_t msgType)
{
    mMotoInterface->disableMsgType(msgType);
}

bool
KobeCameraWrapper::msgTypeEnabled(int32_t msgType)
{
    return mMotoInterface->msgTypeEnabled(msgType);
}

status_t
KobeCameraWrapper::startPreview()
{
    return mMotoInterface->startPreview();
}

bool
KobeCameraWrapper::useOverlay()
{
    return mMotoInterface->useOverlay();
}

status_t
KobeCameraWrapper::setOverlay(const sp<Overlay> &overlay)
{
    return mMotoInterface->setOverlay(overlay);
}

void
KobeCameraWrapper::stopPreview()
{
    mMotoInterface->stopPreview();
}

bool
KobeCameraWrapper::previewEnabled()
{
    return mMotoInterface->previewEnabled();
}

status_t
KobeCameraWrapper::startRecording()
{
    return mMotoInterface->startRecording();
}

void
KobeCameraWrapper::stopRecording()
{
    mMotoInterface->stopRecording();
}

bool
KobeCameraWrapper::recordingEnabled()
{
    return mMotoInterface->recordingEnabled();
}

void
KobeCameraWrapper::releaseRecordingFrame(const sp<IMemory>& mem)
{
    return mMotoInterface->releaseRecordingFrame(mem);
}

status_t
KobeCameraWrapper::autoFocus()
{
    return mMotoInterface->autoFocus();
}

status_t
KobeCameraWrapper::cancelAutoFocus()
{
    return mMotoInterface->cancelAutoFocus();
}

status_t
KobeCameraWrapper::takePicture()
{
    return mMotoInterface->takePicture();
}

status_t
KobeCameraWrapper::cancelPicture()
{
    return mMotoInterface->cancelPicture();
}

status_t
KobeCameraWrapper::setParameters(const CameraParameters& params)
{
    CameraParameters pars(params.flatten());
    status_t retval;
    retval = mMotoInterface->setParameters(pars);
    return retval;
}

CameraParameters
KobeCameraWrapper::getParameters() const
{
    CameraParameters ret = mMotoInterface->getParameters();

    ret.set(CameraParameters::KEY_SMOOTH_ZOOM_SUPPORTED, "false");
    ret.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, "320x240,640x480,1280x960,1600x1200,2048x1536");

    /* cut down supported effects to values supported by framework */
    ret.set(CameraParameters::KEY_SUPPORTED_EFFECTS, "none,mono,sepia,negative,solarize,red-tint,green-tint,blue-tint");

    /* Motorola uses mot-exposure-offset instead of exposure-compensation
       for whatever reason -> adapt the values.
       The limits used here are taken from the lib, we surely also
       could parse it, but it's likely not worth the hassle */
    float exposure = ret.getFloat("mot-exposure-offset");
    int exposureParam = (int) round(exposure * 3);

    ret.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, exposureParam);
    ret.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, "9");
    ret.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, "-9");
    ret.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "0.3333333333333");

    ret.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT, CameraParameters::PIXEL_FORMAT_YUV422I);
    ret.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE,
            "(1000,30000),(1000,25000),(1000,20000),(1000,24000),(1000,15000),(1000,10000)");
    ret.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "1000, 30000");

    return ret;
}

status_t
KobeCameraWrapper::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    return mMotoInterface->sendCommand(cmd, arg1, arg2);
}

void
KobeCameraWrapper::release()
{
    mMotoInterface->release();
}

status_t
KobeCameraWrapper::dump(int fd, const Vector<String16>& args) const
{
    return mMotoInterface->dump(fd, args);
}

}; //namespace android

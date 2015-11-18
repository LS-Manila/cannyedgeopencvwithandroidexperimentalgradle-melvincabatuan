#include <jni.h>

#include <android/log.h>
#include <android/bitmap.h>

#include "opencv2/imgproc.hpp"

#define  LOG_TAG    "CannyEdge"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT void JNICALL
Java_ph_edu_dlsu_cannyedge_CameraActivity_process(JNIEnv *env, jobject instance,
                                                            jobject pTarget, jbyteArray pSource,
                                                            jint minThresh, jint maxThresh) {
    uint32_t t;
    cv::Mat srcBGR;
    cv::Mat edge;

    AndroidBitmapInfo bitmapInfo;
    uint32_t* bitmapContent;

    if(AndroidBitmap_getInfo(env, pTarget, &bitmapInfo) < 0) abort();
    if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
    if(AndroidBitmap_lockPixels(env, pTarget, (void**)&bitmapContent) < 0) abort();

    /// Access source array data... OK
    jbyte* source = (jbyte*)env->GetPrimitiveArrayCritical(pSource, 0);
    if (source == NULL) abort();

    /// cv::Mat for YUV420sp source and output BGRA
    cv::Mat srcGray(bitmapInfo.height, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
    //cv::Mat src(bitmapInfo.height + bitmapInfo.height/2, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
    cv::Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);


/***********************************************************************************************/
    /// Native Image Processing HERE...

    t = cv::getTickCount();

    if(srcBGR.empty())
        srcBGR = cv::Mat(bitmapInfo.height, bitmapInfo.width, CV_8UC3);

    if (edge.empty())
        edge = cv::Mat(srcGray.size(), CV_8UC1);

    // cvtColor(src, srcBGR, CV_YUV420sp2RGB);  // 3-6 ms

    cv::blur(srcGray, edge, cv::Size(3,3));

    cv::Canny(edge, edge, minThresh, maxThresh, 3);


    LOGI("Processing took %0.2f ms.", 1000*((float)cv::getTickCount() - t)/(float)cv::getTickFrequency());

    cvtColor(edge, mbgra, CV_GRAY2BGRA);
    //cvtColor(srcBGR, mbgra, CV_BGR2BGRA);

/************************************************************************************************/

    /// Release Java byte buffer and unlock backing bitmap
    //env-> ReleasePrimitiveArrayCritical(pSource,source,0);
    /*
     * If 0, then JNI should copy the modified array back into the initial Java
     * array and tell JNI to release its temporary memory buffer.
     *
     * */

    env-> ReleasePrimitiveArrayCritical(pSource, source, JNI_COMMIT);
    /*
 * If JNI_COMMIT, then JNI should copy the modified array back into the
 * initial array but without releasing the memory. That way, the client code
 * can transmit the result back to Java while still pursuing its work on the
 * memory buffer
 *
 * */

    /*
     * Get<Primitive>ArrayCritical() and Release<Primitive>ArrayCritical()
     * are similar to Get<Primitive>ArrayElements() and Release<Primitive>ArrayElements()
     * but are only available to provide a direct access to the target array
     * (instead of a copy). In exchange, the caller must not perform blocking
     * or JNI calls and should not hold the array for a long time
     *
     */


    if (AndroidBitmap_unlockPixels(env, pTarget) < 0) abort();
}


#ifdef __cplusplus
}
#endif
//
// Created by FT on 2018/4/26.
//

#include <utils.h>
#include <JUB_SDK.h>
#include <logUtils.h>
#include <sstream>
#include <iostream>


static JavaVM *jVM = NULL;
static jobject j_ObjScanCalbak = NULL;
static jobject j_ObjDiscCalbak = NULL;
// 初始化参数
jobject j_InitParam    = NULL;


void setScanCallbackObj(JUB_VOID_PTR javaVM, jobject callback) {
    jVM  = (JavaVM *)javaVM;
    if (NULL != j_ObjScanCalbak) {
        j_ObjScanCalbak = NULL;
    }
    j_ObjScanCalbak = callback;
}

void setDiscCallbackObj(JUB_VOID_PTR javaVM, jobject callback) {
    jVM  = (JavaVM *)javaVM;
    j_ObjDiscCalbak = callback;
}


void callback(unsigned int type) {
    LOG_ERR("callback type: %d", type);

    JNIEnv *env = NULL;
    if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_6)) != JNI_OK) {
        LOG_ERR("GetEnv failed");
        return;
    }

    jclass clazz = env->GetObjectClass(j_InitParam);
    if (NULL == clazz) {
        LOG_ERR("NULL == clazz");
        env->DeleteLocalRef(clazz);
        return;
    }

    jmethodID onCallbackID = env->GetMethodID(clazz, "funcCallBack", "(I)I");
    if (NULL == onCallbackID) {
        LOG_ERR("NULL == onCallbackID");
        env->DeleteLocalRef(clazz);
        clazz = NULL;
        return;
    }

    jint operationType = type;
    env->CallIntMethod(j_InitParam, onCallbackID, operationType);
    env->DeleteLocalRef(clazz);
}

void scanCallback(JUB_BYTE_PTR devname, JUB_BYTE_PTR uuid, JUB_ULONG devType) {
    LOG_ERR("scanCallback devname: %s  uuid: %s  devType: %lu", devname, uuid, devType);

    JNIEnv *env = NULL;
    if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_6)) != JNI_OK) {
        LOG_ERR("GetEnv failed");
        return;
    }

    jclass clazz = env->GetObjectClass(j_ObjScanCalbak);
    if (NULL == clazz) {
        LOG_ERR("NULL == clazz");
        env->DeleteLocalRef(clazz);
        return;
    }

    jmethodID onScanResultID = env->GetMethodID(clazz, "onScanResult", "(Ljava/lang/String;Ljava/lang/String;I)V");
    if (NULL == onScanResultID) {
        LOG_ERR("NULL == onScanResultID");
        env->DeleteLocalRef(clazz);
        clazz = NULL;
        return;
    }

    jstring devName = env->NewStringUTF((const char *) devname);
    jstring devUuid = env->NewStringUTF((const char *) uuid);

    env->CallVoidMethod(j_ObjScanCalbak, onScanResultID, devName, devUuid, devType);
    env->DeleteLocalRef(devName);
    env->DeleteLocalRef(devUuid);
    env->DeleteLocalRef(clazz);
}

void disconnectCallback(JUB_BYTE_PTR devname) {
    LOG_ERR("disconnectCallback name: %s", devname);

    JNIEnv *env = NULL;
    if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_6)) != JNI_OK) {
        LOG_ERR("GetEnv failed");
        return;
    }

    jclass clazz = env->GetObjectClass(j_ObjDiscCalbak);
    if (NULL == clazz) {
        LOG_ERR("NULL == clazz");
        return;
    }

    jmethodID onDisconnectID = env->GetMethodID(clazz, "onDisconnect", "(Ljava/lang/String;)V");
    if (NULL == onDisconnectID) {
        LOG_ERR("NULL == onDisconnectID");
        env->DeleteLocalRef(clazz);
        clazz = NULL;
        return;
    }

    jstring devName = env->NewStringUTF((const char *) devname);
    env->CallVoidMethod(j_ObjDiscCalbak, onDisconnectID, devName);
    env->DeleteLocalRef(devName);
    env->DeleteLocalRef(clazz);
}


JUB_RV jobjectToInitParam(JNIEnv *env, JUB_VOID_PTR javaVM, DEVICE_INIT_PARAM *param) {

    CHECK_NULL(env, JUBR_ARGUMENTS_BAD)
    CHECK_NULL(javaVM, JUBR_ARGUMENTS_BAD)

    param->param = javaVM;
    param->scanCallBack = reinterpret_cast<DEV_ScanCallBack>(scanCallback);
    param->discCallBack = disconnectCallback;
    param->callBack = reinterpret_cast<DEV_ReadCallBack>(callback);

    return JUBR_OK;
}

/**
 * 创建 java 结果对象
 * @param className
 * @param errorCode
 * @return
 */
jobject newJavaObject(JNIEnv *env, const char * className, int errorCode, jstring message) {
    if (nullptr == env) {
        return nullptr;
    }

    jclass clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOG_ERR(">>>> clazz == NULL");
        return nullptr;
    }
    LOG_ERR(">>>> FindClass success");

    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(ILjava/lang/String;)V");
    jobject newObject = env->NewObject(clazz, constructor, errorCode, message);
    return newObject;
}

std::string CharPtr2HexStr(std::vector<unsigned char> v) {

    std::stringstream ss;
    ss << std::hex;

    size_t len = v.size();
    for (size_t i(0); i < len; ++i) {
        if (0x0F >= (int)v[i]) {
            ss << 0;
        }
        ss << (int)v[i];
    }
    return ss.str();
}

std::string CharPtr2HexStr(unsigned char* data,size_t len){
    std::vector<unsigned char> v(data,data+len);
    return CharPtr2HexStr(v);
}

std::vector<unsigned char> HexStr2CharPtr(std::string str) {

    std::vector<unsigned char> v;

    std::string strHex;
    auto l = str.size();
    if (l%2) {
        strHex = "0";
    }
    else {
        strHex = "";
    }
    strHex += str;
    unsigned int c;
    std::string sub;
    for (int i = 0; i < l; i += 2) {
        sub = strHex.substr(i, 2);
        std::istringstream hex_chars_stream(sub.c_str());
        hex_chars_stream >> std::hex >> c;
        v.push_back(c);
    }

    return v;
}
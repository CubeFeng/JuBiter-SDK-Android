package com.jubiter.sdk;

/**
 * @author fengshuo
 * @date 2019/9/25
 * @time 15:51
 */
public interface ConnectionStateCallback {

    /**
     * 连接成功，返回设备句柄
     *
     * todo: 导出aar包后如何保留形参命名
     *
     * @param mac 设备MAC
     * @param handle 连接成功后的设备句柄
     */
    public void onConnected(String mac, int handle);

    /**
     * 连接断开回调
     *
     * @param mac 设备MAC
     */
    public void onDisconnected(String mac);

    /**
     * 错误码
     *
     * @param error
     */
    public void onError(int error);

}

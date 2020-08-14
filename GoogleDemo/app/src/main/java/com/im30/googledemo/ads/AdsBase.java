package com.im30.googledemo.ads;

import android.app.Activity;

public abstract class AdsBase implements AdsInterface {


    public enum AdsInfoType
    {
        LOG_Display(0),     //广告展示
        LOG_Click(1),       //广告点击
        LOG_Impression(2),  //广告Impression
        LOG_Dismiss(3),     //广告关闭
        LOG_Error(4),      //广告报错
        LOG_LOADED(5),      //加载完成
        LOG_LoadForReward(6), //c++ usage
        LOG_VIDEO_PLAY(7),  //视频播放
        LOG_VIDEO_Reward(8), //视频奖励生效
        LOG_VIDEO_LEFT_APPLICATION(9);   //视频千万其他位置

        private int value;
        AdsInfoType(int i) {
            value = i;
        }

        public int getValue()
        {
            return this.value;
        }

        public static AdsInfoType get(int ordinal){
            return values()[ordinal];
        }
    }


    private boolean m_showAfterLoad = false;
    private boolean m_isAdsReady = false;
    private boolean m_isAdsLoaded = false;


    public void showAds(boolean waitForLoad)
    {
        if(isAdsReady())
            start();
        else if(waitForLoad)
            m_showAfterLoad = true;
    }

    public void setAdsReady(boolean ready)
    {
        m_isAdsReady = ready;
        if(isAdsReady() && m_showAfterLoad)
            start();
    }

    public void setAdsLoaded(boolean loaded)
    {
        m_isAdsLoaded = loaded;
    }

    public boolean isAdsLoaded()
    {
        return m_isAdsLoaded;
    }

    public boolean isAdsReady()
    {
        return m_isAdsReady;
    }

    public void sendLogInfo(AdsInfoType type, String info)
    {
        AdsManager.getInstance().onAdsLogInfo(getType(), type, info);
    }

    public void onResume() {
    }

    public void onPause() {
    }

    public void onDestroy() {
    }
}

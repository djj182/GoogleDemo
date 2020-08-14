package com.im30.googledemo.ads;

public enum AdsType {
    FB_InterstitialAd(1<<0),       //facebook 插屏广告
    FB_FULLSREEN_REWARD(1<<1),     //facebook全屏强行视频广告
    ADS_FULLSREEN_REWARD(1<<2),     //Googld AdsMod
    UNITY_FULLSREEN_REWARD(1<<3),
    ADS_ALL(FB_InterstitialAd.getValue() | FB_FULLSREEN_REWARD.getValue()
            | ADS_FULLSREEN_REWARD.getValue() | UNITY_FULLSREEN_REWARD.getValue()),
    NONE(0);

    AdsType(int i) {
        this.value = i;
    }

    private int value;

    public int getValue()
    {
        return value;
    }
    public static AdsType get(int ordinal){
        for (int i = 0; i < values().length; i++)
        {
            if(values()[i].getValue() == ordinal)
                return values()[i];
        }
        return NONE;
    }
}

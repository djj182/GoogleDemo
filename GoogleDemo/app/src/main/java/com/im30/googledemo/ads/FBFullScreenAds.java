package com.im30.googledemo.ads;

import android.app.Activity;
import android.util.Log;

import com.facebook.ads.Ad;
import com.facebook.ads.AdError;
import com.facebook.ads.AdSettings;
import com.facebook.ads.AudienceNetworkAds;
import com.facebook.ads.RewardedVideoAd;;
import com.facebook.ads.RewardedVideoAdListener;

public class FBFullScreenAds extends AdsBase{
    private final String TAG = FBFullScreenAds.class.getSimpleName();
    private RewardedVideoAd rewardedVideoAd;
    private Activity m_activity;
    private int reloadCountMax = 10;
    private int reloadCount = 0;

    private final FBRewardListener mAdListener = new FBRewardListener();

    @Override
    public AdsType getType() {
        return AdsType.FB_FULLSREEN_REWARD;
    }

    @Override
    public void preLoad() {
        clear();

        rewardedVideoAd = new RewardedVideoAd(m_activity, "405856650234637_444666726353629");
        rewardedVideoAd.setAdListener(mAdListener);
        rewardedVideoAd.loadAd();
        return;
    }

    @Override
    public void forceLoad() {
        reloadCount = 0;
        preLoad();
    }


    @Override
    public void init(Activity activity) {
        m_activity = activity;
        AudienceNetworkAds.initialize(m_activity);
//        AdSettings.addTestDevice("7ac0f2c0-ba3b-4e18-bd20-ba985dbdef63");
    }



    @Override
    public void start() {
        if(rewardedVideoAd.isAdLoaded())
            rewardedVideoAd.show();
    }

    @Override
    public void clear() {
        setAdsLoaded(false);
        setAdsReady(false);
        if(rewardedVideoAd != null)
        {
            rewardedVideoAd.destroy();
            rewardedVideoAd = null;
        }
    }

    @Override
    public boolean isAdsReady()
    {
        return rewardedVideoAd != null && rewardedVideoAd.isAdLoaded();
    }

    private class FBRewardListener implements RewardedVideoAdListener
    {
        @Override
        public void onError(Ad ad, AdError error) {
            // Rewarded video ad failed to load
            sendLogInfo(AdsInfoType.LOG_Error, "FBFullScreenAds is LOG_Error: " + error.getErrorMessage());
            setAdsLoaded(true);
            clear();
            if(reloadCount < reloadCountMax)
            {
                preLoad();
                reloadCount++;
            }

        }

        @Override
        public void onAdLoaded(Ad ad) {
            // Rewarded video ad is loaded and ready to be displayed
            sendLogInfo(AdsInfoType.LOG_LOADED, "FBFullScreenAds is loaded and ready to be displayed");
            setAdsReady(true);

        }

        @Override
        public void onAdClicked(Ad ad) {
            // Rewarded video ad clicked
            sendLogInfo(AdsInfoType.LOG_Click, "FBFullScreenAds Rewarded video ad clicked!");
        }

        @Override
        public void onLoggingImpression(Ad ad) {
            // Rewarded Video ad impression - the event will fire when the
            // video starts playing
            sendLogInfo(AdsInfoType.LOG_Impression, "FBFullScreenAds Rewarded video ad impression logged!");
        }

        @Override
        public void onRewardedVideoCompleted() {
            // Rewarded Video View Complete - the video has been played to the end.
            // You can use this event to initialize your reward
            Log.d(TAG, "Rewarded video completed!");

            // Call method to give reward
            // giveReward();
            sendLogInfo(AdsInfoType.LOG_VIDEO_Reward, "FBFullScreenAds isReward ");
        }

        @Override
        public void onRewardedVideoClosed() {
            // The Rewarded Video ad was closed - this can occur during the video
            // by closing the app, or closing the end card.
            Log.d(TAG, "Rewarded video ad closed!");
            sendLogInfo(AdsInfoType.LOG_Dismiss, "FBFullScreenAds is Close");
            preLoad();
        }
    }


}

package com.im30.googledemo.ads;

import android.app.Activity;

import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.initialization.InitializationStatus;
import com.google.android.gms.ads.initialization.OnInitializationCompleteListener;
import com.google.android.gms.ads.reward.RewardItem;
import com.google.android.gms.ads.reward.RewardedVideoAd;
import com.google.android.gms.ads.reward.RewardedVideoAdListener;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdCallback;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;
import com.im30.googledemo.utils.Utils;

import androidx.annotation.NonNull;

public class AdModFullScreenAds extends AdsBase {

    Activity m_activity;
    private static RewardedVideoAd mRewardedVideoAd;
    private static RewardedAd rewardedAd;
    private int reloadCountMax = 10;
    private int reloadCount = 0;

    private static String RewardADIdDebug = "ca-app-pub-3940256099942544/5224354917";
    private static String RewardADId = "ca-app-pub-1832760595508219/2314388799";
    @Override
    public void init(Activity activity) {
        m_activity = activity;
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                MobileAds.initialize(m_activity, new OnInitializationCompleteListener() {
                    @Override
                    public void onInitializationComplete(InitializationStatus initializationStatus) {
                        rewardedAd = createAndLoadRewardedAd(Utils.isApkDebugAble(m_activity) ? RewardADIdDebug:RewardADId);
                    }
                });
                clear();
                // Use an activity context to get the rewarded video instance.
                // 老版本ads 代码
//        if(mRewardedVideoAd == null)
//            mRewardedVideoAd = MobileAds.getRewardedVideoAdInstance(m_activity);
//        m_activity.runOnUiThread(new Runnable() {
//            @Override
//            public void run() {
//                mRewardedVideoAd.setRewardedVideoAdListener(AdModFullScreenAds.this);
//            }
//        });

            }
        });
    }

    @Override
    public void start() {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (isAdsReady())
                {
                    rewardedAd.show(m_activity,adCallback);
                    setAdsReady(false);
                }

            }
        });
    }

    @Override
    public void clear() {
    }

    @Override
    public AdsType getType() {
        return AdsType.ADS_FULLSREEN_REWARD;
    }


    @Override
    public void preLoad() {
        setAdsReady(false);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                rewardedAd = createAndLoadRewardedAd(Utils.isApkDebugAble(m_activity) ? RewardADIdDebug:RewardADId);
            }
        });
    }


    @Override
    public void forceLoad()
    {
        reloadCount = 0;
        preLoad();
    }

    public RewardedAd createAndLoadRewardedAd(String adUnitId) {
        RewardedAd rewardedAd = new RewardedAd(m_activity, adUnitId);
        RewardedAdLoadCallback adLoadCallback = new RewardedAdLoadCallback() {
            @Override
            public void onRewardedAdLoaded() {
                // Ad successfully loaded.
                setAdsReady(true);
                sendLogInfo(AdsInfoType.LOG_LOADED, "AdsModReward is loaded and ready to be displayed!");
            }

            @Override
            public void onRewardedAdFailedToLoad(LoadAdError error) {
                // Ad failed to load.
                setAdsLoaded(true);
                // Gets the domain from which the error came.
                String errorDomain = error.getDomain();
                // Gets the error code. See
                // https://developers.google.com/android/reference/com/google/android/gms/ads/AdRequest#constant-summary
                // for a list of possible codes.
                int errorCode = error.getCode();
                // Gets an error message.
                // For example "Account not approved yet". See
                // https://support.google.com/admob/answer/9905175 for explanations of
                // common errors.
                String errorMessage = error.getMessage();
                // Gets the cause of the error, if available.
                AdError cause = error.getCause();
                sendLogInfo(AdsInfoType.LOG_Error, String.format("AdsModReward Video Error errorDomain: %s errorMessage:%s errorCode:%d" ,errorDomain,errorMessage, errorCode));
                if(reloadCount < reloadCountMax)
                {
                    preLoad();
                    reloadCount++;
                    sendLogInfo(AdsInfoType.LOG_Error, "AdsModReward Video ReloadCount: " + reloadCount);
                }
            }
        };

        if (rewardedAd != null && !rewardedAd.isLoaded()) {
            AdRequest.Builder request = new AdRequest.Builder();
//                    request.addTestDevice("A57720F3DA7086C42B9E652191E920E4");
//                    request.addTestDevice("A57720F3DA7086C42B9E652191E920E4")
            rewardedAd.loadAd(request.build(), adLoadCallback);
        }

        return rewardedAd;
    }

    private RewardedAdCallback adCallback = new RewardedAdCallback() {
        @Override
        public void onRewardedAdOpened() {
            // Ad opened.
            sendLogInfo(AdsInfoType.LOG_Display, "AdsModReward OPened!");
        }

        @Override
        public void onRewardedAdClosed() {
            // Ad closed.
            sendLogInfo(AdsInfoType.LOG_Dismiss, "AdsModReward Video Closed!");
            preLoad();
        }

        @Override
        public void onUserEarnedReward(@NonNull com.google.android.gms.ads.rewarded.RewardItem rewardItem) {
            // User earned reward.
            sendLogInfo(AdsInfoType.LOG_VIDEO_Reward, "AdsModReward Video Rewardeds!");
        }

        @Override
        public void onRewardedAdFailedToShow(AdError error) {
            // Ad failed to display.
            // Gets the domain from which the error came.
            String errorDomain = error.getDomain();
            // Gets the error code. See
            // https://developers.google.com/android/reference/com/google/android/gms/ads/AdRequest#constant-summary
            // for a list of possible codes.
            int errorCode = error.getCode();
            // Gets an error message.
            // For example "Account not approved yet". See
            // https://support.google.com/admob/answer/9905175 for explanations of
            // common errors.
            String errorMessage = error.getMessage();
            // Gets the cause of the error, if available.
            AdError cause = error.getCause();
            sendLogInfo(AdsInfoType.LOG_Error, String.format("AdsModReward Video Error errorDomain: %s errorMessage:%s errorCode:%d" ,errorDomain,errorMessage, errorCode));
            sendLogInfo(AdsInfoType.LOG_VIDEO_PLAY, "AdsModReward Video Start!");
        }
    };
}

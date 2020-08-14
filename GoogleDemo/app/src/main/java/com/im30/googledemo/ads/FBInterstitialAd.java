package com.im30.googledemo.ads;

import android.app.Activity;
import android.util.Log;

import com.facebook.ads.Ad;
import com.facebook.ads.AdError;
import com.facebook.ads.AdSettings;
import com.facebook.ads.InterstitialAd;
import com.facebook.ads.InterstitialAdListener;

import static com.im30.googledemo.ads.AdsType.FB_InterstitialAd;


public class FBInterstitialAd extends AdsBase {

    private final String TAG = FBInterstitialAd.class.getSimpleName();

    private Activity m_activity;
    private InterstitialAd interstitialAd;
    @Override
    public void init(Activity activity) {
        m_activity = activity;
//        AdSettings.addTestDevice("67a55b58-15a3-4d6a-a823-50614eda83ec");
        clear();
        interstitialAd = new InterstitialAd(m_activity, "405856650234637_405859606901008");
        // Set listeners for the Interstitial Ad
        interstitialAd.setAdListener(new InterstitialAdListener() {
            @Override
            public void onInterstitialDisplayed(Ad ad) {
                // Interstitial ad displayed callback
                sendLogInfo(AdsInfoType.LOG_Display, "Interstitial ad displayed.");
            }

            @Override
            public void onInterstitialDismissed(Ad ad) {
                sendLogInfo(AdsInfoType.LOG_Dismiss, "Interstitial ad dismissed.");
                clear();
            }

            @Override
            public void onError(Ad ad, AdError adError) {
                Log.e(TAG, "Interstitial ad failed to load: " + adError.getErrorMessage());
                sendLogInfo(AdsInfoType.LOG_Error, "Interstitial ad failed to load: " + adError.getErrorMessage());
            }

            @Override
            public void onAdLoaded(Ad ad) {
                setAdsReady(true);
                sendLogInfo(AdsInfoType.LOG_LOADED, "Interstitial ad is loaded and ready to be displayed!");
            }

            @Override
            public void onAdClicked(Ad ad) {
                // Ad clicked callback
                sendLogInfo(AdsInfoType.LOG_Click, "Interstitial ad clicked!");
            }

            @Override
            public void onLoggingImpression(Ad ad) {
                // Ad impression logged callback
                sendLogInfo(AdsInfoType.LOG_Impression, "Interstitial ad impression logged!");
            }
        });
    }


    @Override
    public void preLoad() {
        if(interstitialAd.isAdLoaded())
        {
            return;
        }
        interstitialAd.loadAd();
        return;
    }

    @Override
    public void forceLoad() {

    }

    @Override
    public void start() {

//        if(interstitialAd.isAdInvalidated())
//        {
//            clear();
//            return;
//        }
//        if(!interstitialAd.isAdLoaded())
//        {
//            return;
//        }
        interstitialAd.show();
        // For auto play video ads, it's recommended to load the ad
        // at least 30 seconds before it is shown

    }

    @Override
    public void clear() {
        if(interstitialAd != null)

        {
            interstitialAd.destroy();
            interstitialAd = null;
        }
    }

    @Override
    public AdsType getType() {
        return FB_InterstitialAd;
    }

}

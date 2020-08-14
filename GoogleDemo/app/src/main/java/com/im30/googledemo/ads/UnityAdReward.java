package com.im30.googledemo.ads;

import android.app.Activity;
import com.unity3d.ads.IUnityAdsListener;
import com.unity3d.ads.UnityAds;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import static com.im30.googledemo.ads.AdsType.UNITY_FULLSREEN_REWARD;

public class UnityAdReward extends AdsBase {
    private String UnityGameId = "3215141";
    private boolean testMode = false;
//    private String m_readyPlaceId = "";
    private final UnityAdsListener myAdsListener = new UnityAdsListener ();
    private Activity m_activity;

    private HashMap<String, String> m_readyPlace = new HashMap<>();
    @Override
    public void init(Activity activity) {
        // Initialize the SDK:
        m_activity = activity;
        UnityAds.initialize (activity, UnityGameId, myAdsListener, testMode);
    }

    @Override
    public void start() {
        for (String key : m_readyPlace.keySet()) {
            if (UnityAds.isReady (key)) {
                UnityAds.show (m_activity, key);
                break;
            }
        }

    }

    @Override
    public void clear() {

    }

    @Override
    public AdsType getType() {
        return UNITY_FULLSREEN_REWARD;
    }

    @Override
    public void preLoad() {
        return;
    }

    @Override
    public boolean isAdsReady()
    {
        return m_readyPlace.size() > 0;
    }

    @Override
    public void forceLoad()
    {
        m_readyPlace.clear();
        UnityAds.removeListener(myAdsListener);
        UnityAds.initialize (m_activity, UnityGameId, myAdsListener, testMode);
    }

    private void processReadyPlaceMentId(String placementId)
    {
        m_readyPlace.put(placementId, placementId);
    }

    //UnityAdsListener
    private class UnityAdsListener implements IUnityAdsListener {

        public void onUnityAdsReady (String placementId) {
            // Implement functionality for an ad being ready to show.
            setAdsReady(true);
            processReadyPlaceMentId(placementId);
            sendLogInfo(AdsInfoType.LOG_LOADED, "AdsUnityReward is loaded and ready to be displayed!：" + placementId);

        }

        @Override
        public void onUnityAdsStart (String placementId) {
            m_readyPlace.remove(placementId);
            // Implement functionality for a user starting to watch an ad.
            sendLogInfo(AdsInfoType.LOG_VIDEO_PLAY, "AdsUnityReward Video Start! " + placementId);

        }

        @Override
        public void onUnityAdsFinish (String placementId, UnityAds.FinishState finishState) {
            // Implement conditional logic for each ad completion status:
            if (finishState == UnityAds.FinishState.COMPLETED) {
                sendLogInfo(AdsInfoType.LOG_VIDEO_Reward, "AdsUnityReward Video Rewardeds!");
                // Reward the user for watching the ad to completion.
            } else if (finishState == UnityAds.FinishState.SKIPPED) {
                // Do not reward the user for skipping the ad.
                sendLogInfo(AdsInfoType.LOG_Dismiss, "AdsUnityReward Video Play Skip: ");
            } else if (finishState == UnityAds.FinishState.ERROR) {
                // Log an error.
                sendLogInfo(AdsInfoType.LOG_Error, "AdsUnityReward Video Play Error: ");
            }
        }

        @Override
        public void onUnityAdsError (UnityAds.UnityAdsError error, String message) {
            // Implement functionality for a Unity Ads service error occurring.
            setAdsLoaded(true);
            sendLogInfo(AdsInfoType.LOG_Error, "AdsUnityReward Video Error Code: " + error + " msg: " + message);

        }
    }
}

package com.im30.googledemo.ads;


import android.app.Activity;
import android.util.Log;
import android.util.Pair;

import com.facebook.ads.AdSettings;
import com.facebook.ads.AudienceNetworkAds;
import com.google.android.gms.ads.MobileAds;


import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

import static com.facebook.ads.AdSettings.IntegrationErrorMode.INTEGRATION_ERROR_CALLBACK_MODE;
import static com.im30.googledemo.ads.AdsType.ADS_FULLSREEN_REWARD;
import static com.im30.googledemo.ads.AdsType.UNITY_FULLSREEN_REWARD;

public class AdsManager {

    /**
     * 开启广告
     *
     * @param type
     */
    private final String TAG = AdsManager.class.getSimpleName();
    private static AdsManager _instance = null;


    public static AdsManager getInstance() {
        if (_instance == null)
            _instance = new AdsManager();
        return _instance;
    }

    private List<AdsInterface> m_adsLoadList = new ArrayList<AdsInterface>();
    private List<Pair<AdsType, Class<? extends AdsInterface>>> m_AdsClass = new ArrayList<Pair<AdsType, Class<? extends AdsInterface>>>();
    private Activity m_activity;
    private boolean inited = false;
    private boolean loaded = false;

    public void init(Activity activity) {
        m_activity = activity;
        inited = true;

        m_AdsClass.add(new Pair<AdsType, Class<? extends AdsInterface>>(ADS_FULLSREEN_REWARD, AdModFullScreenAds.class));
//        m_AdsClass.add(new Pair<AdsType, Class<? extends AdsInterface>>(AdsType.FB_FULLSREEN_REWARD, FBFullScreenAds.class));
//        m_AdsClass.add(new Pair<AdsType, Class<? extends AdsInterface>>(UNITY_FULLSREEN_REWARD, UnityAdReward.class));
//        AudienceNetworkAds.initialize(m_activity);
//        preLoadAds();

    }

    private void preLoadAds(int types)  {
        if(loaded)
            return;
        loaded = true;
        Iterator<Pair<AdsType, Class<? extends AdsInterface>>> it = m_AdsClass.iterator();
        while (it.hasNext()) {
            Pair<AdsType, Class<? extends AdsInterface>> entry = it.next();
            if((entry.first.getValue() & types) > 0)
                preLoadClass(entry.second);
        }

        //处理报错
    }

    private void preLoadClass(Class<? extends  AdsInterface> adsClass)
    {
        try
        {
            AdsInterface adsObject = adsClass.newInstance();
            adsObject.init(m_activity);
            adsObject.preLoad();
            m_adsLoadList.add(adsObject);
            Log.d(TAG, "Ads Start Load " + adsClass.getName());
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

    }

    public boolean openAds(int type) {
        return openAds(AdsType.get(type));
//        return openAds(UNITY_FULLSREEN_REWARD);
    }

    public boolean openAds(AdsType type) {
        if (!inited)
            return false;
        Iterator<AdsInterface> i = m_adsLoadList.iterator();
        boolean adsFire = false;
        while (i.hasNext()) {
            AdsInterface ads = i.next();
            if (ads.getType() == type || (ads.getType().getValue() & type.getValue()) > 0) {
                Log.d(TAG, "Ads " + type + "Open With " + ads.isAdsReady() + " " + ads.isAdsLoaded());
                if(ads.isAdsReady())
                {
                    ads.showAds(false);
                    adsFire = true;
                    break;
                }
                else
                {
                    if(ads.isAdsLoaded())
                    {
                        ads.forceLoad();
                    }
                }

            }
        }

        return adsFire;
    }

    public boolean checkAdsReady(int type)
    {
        return checkAdsReady(AdsType.get(type));
    }
    public boolean checkAdsReady(AdsType type)
    {
        if (!inited)
            return false;
        Iterator<AdsInterface> i = m_adsLoadList.iterator();
        while (i.hasNext()) {
            AdsInterface ads = i.next();
            if ((ads.getType() == type || (ads.getType().getValue() & type.getValue()) > 0) && ads.isAdsReady()) {
                return true;
            }
        }
        return false;
    }

    //c++ directCall

    public static void static_openAds(int type) {
        AdsManager.getInstance().openAds(type);
    }

    public static boolean static_adsReady(int type) {
        return AdsManager.getInstance().checkAdsReady(type);
    }

    public static void static_load(int types)
    {
        AdsManager.getInstance().preLoadAds(types);
    }

    // callback
    public void onAdsLogInfo(final AdsType type, final AdsBase.AdsInfoType infoType, final String info) {
        Log.d(TAG, "Ads " + type + " Send Log : " + infoType + " msg: " + info);
//        IF.getInstance().runOnGLThread(new Runnable() {
//            @Override
//            public void run() {
//                onAdsShowRespone(type.getValue(), infoType.getValue(), info);
//            }
//        });

    }
    private static native void onAdsShowRespone(int type, int status, String info);


    //liftcycle

    public void onResume() {
        Iterator<AdsInterface> i = m_adsLoadList.iterator();
        while (i.hasNext()) {
            AdsInterface ads = i.next();
            ads.onResume();
        }
    }

    public void onPause() {
        Iterator<AdsInterface> i = m_adsLoadList.iterator();
        while (i.hasNext()) {
            AdsInterface ads = i.next();
            ads.onPause();
        }
    }

    public void onDestroy() {
        Iterator<AdsInterface> i = m_adsLoadList.iterator();
        while (i.hasNext()) {
            AdsInterface ads = i.next();
            ads.onDestroy();
        }
    }

}

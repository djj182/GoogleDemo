package com.im30.googledemo.ads;

import android.app.Activity;

public interface AdsInterface {
    public void init(Activity activity);

    public void start();

    public boolean isAdsReady();

    public boolean isAdsLoaded();

    public void clear();

    public AdsType getType();

    /**
     *
     * @return true should preload, false no need to preload
     */
    public void preLoad();

    // reload even there is an error;
    public void forceLoad();

    public void showAds(boolean waitForLoad);

    public void setAdsReady(boolean ready);

    public void setAdsLoaded(boolean loaded);

    public void onResume();

    public void onPause();

    public void onDestroy();

}


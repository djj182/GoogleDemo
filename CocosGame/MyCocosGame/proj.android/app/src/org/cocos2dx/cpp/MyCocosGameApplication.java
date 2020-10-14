package org.cocos2dx.cpp;

import android.content.Context;
import android.os.Handler;

import androidx.multidex.MultiDexApplication;

public class MyCocosGameApplication extends MultiDexApplication {
    public static volatile Context applicationContext;
    public static volatile Handler applicationHandler;

    @Override
    public void onCreate() {
        super.onCreate();
        initApplication();
    }

    private void initApplication() {
        applicationContext = getApplicationContext();
        applicationHandler = new Handler(applicationContext.getMainLooper());
    }
}
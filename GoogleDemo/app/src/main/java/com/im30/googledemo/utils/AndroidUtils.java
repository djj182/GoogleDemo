package com.im30.googledemo.utils;

import android.content.Context;

import com.im30.googledemo.GoogleDemoApplication;

public class AndroidUtils {

    public static Context mainApplication;

    public static void postDelay(Runnable runnable) {
        GoogleDemoApplication.applicationHandler.post(runnable);
    }

    public static void postDelay(Runnable runnable,long delay) {
        GoogleDemoApplication.applicationHandler.postDelayed(runnable,delay);
    }
}

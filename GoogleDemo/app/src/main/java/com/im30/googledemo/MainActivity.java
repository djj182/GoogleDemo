package com.im30.googledemo;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.app.ActivityOptions;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ResolveInfo;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.webkit.WebChromeClient;
import android.widget.FrameLayout;
import android.widget.Toast;

import com.google.firebase.FirebaseApp;
import com.google.firebase.analytics.FirebaseAnalytics;
import com.google.firebase.iid.FirebaseInstanceId;

import java.lang.reflect.Method;
import java.util.Map;
import java.util.Properties;

public class MainActivity extends AppCompatActivity {

    static Context context = null;
    FrameLayout frameLayout = null;
    FrameLayout frameLayout1 = null;
    FirebaseAnalytics m_FirebaseAnalytics;
    FirebaseApp mFirebaseApp;
    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        context = this;
        setContentView(R.layout.activity_main);

        // Firebase 相关初始化
        mFirebaseApp = FirebaseApp.initializeApp(this);
        m_FirebaseAnalytics = FirebaseAnalytics.getInstance(this);
        m_FirebaseAnalytics.logEvent(FirebaseAnalytics.Event.APP_OPEN,null);
    }

    public void pushClick(View view){
        m_FirebaseAnalytics.logEvent(FirebaseAnalytics.Event.SHARE,null);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    public static Point getRealScreenSize() {
        Point size = new Point();
        try {
            WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
                windowManager.getDefaultDisplay().getRealSize(size);
            } else {
                try {
                    Method mGetRawW = Display.class.getMethod("getRawWidth");
                    Method mGetRawH = Display.class.getMethod("getRawHeight");
                    size.set((Integer) mGetRawW.invoke(windowManager.getDefaultDisplay()), (Integer) mGetRawH.invoke(windowManager.getDefaultDisplay()));
                } catch (Exception e) {
                    size.set(windowManager.getDefaultDisplay().getWidth(), windowManager.getDefaultDisplay().getHeight());
                }
            }
        } catch (Exception e) {
        }
        return size;
    }
}

package com.im30.googledemo;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.FileProvider;

import android.Manifest;
import android.app.AlertDialog;
import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Point;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.StrictMode;
import android.provider.MediaStore;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.Toast;

import com.google.firebase.FirebaseApp;
import com.google.firebase.analytics.FirebaseAnalytics;
import com.google.firebase.crashlytics.FirebaseCrashlytics;
import com.im30.googledemo.thirdSDK.MiitHelper;

import java.io.File;
import java.lang.reflect.Method;

/**
 * @author dongjunjie
 * @description 初始化第三方sdk,初始化共有接口和数据
 */
public class BaseActivity extends AppCompatActivity {

    private static Context context = null;
    private static String TAG = BaseActivity.class.getSimpleName();

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        context = this;

        // Firebase 相关初始化
        iniFirebase();

        // OAID
        initMiit();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    /******************************** OAID SDK **********************************/
    private MiitHelper.AppIdsUpdater listener = new MiitHelper.AppIdsUpdater() {
        @Override
        public void OnIdsAvalid(@NonNull String ids) {

        }
    };

    private MiitHelper miitHelper = null;

    /**
     * OAID 初始化
     */
    public void initMiit() {
        miitHelper = new MiitHelper(listener);
        miitHelper.getDeviceIds(this);
    }

    /******************************** Firebase **********************************/
    protected FirebaseAnalytics m_FirebaseAnalytics = null;
    protected FirebaseApp mFirebaseApp = null;

    /**
     * Firebase 相关初始化
     */
    public void iniFirebase() {
        try {
            mFirebaseApp = FirebaseApp.initializeApp(this);
            m_FirebaseAnalytics = FirebaseAnalytics.getInstance(this);
            FirebaseCrashlytics.getInstance().setUserId("12998649000199");
            FirebaseCrashlytics.getInstance().setCrashlyticsCollectionEnabled(true);

            // 第一次打开app打点
            m_FirebaseAnalytics.logEvent(FirebaseAnalytics.Event.APP_OPEN,null);
        } catch (Exception e) {
            Log.d(TAG, "FirebaseApp init error");
        }
    }
    /******************************************************************/
}

package com.im30;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;

import com.im30.googledemo.ui.activitys.CppDemoActivity;
import com.im30.googledemo.ui.home.HomePageActivity;
import com.im30.googledemo.R;
import com.im30.googledemo.utils.AndroidUtils;

public class SplashAdsActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash_ads);
    }

    @Override
    protected void onResume() {
        super.onResume();
        AndroidUtils.postDelay(new Runnable() {
            @Override
            public void run() {
                jumpMainActivity();
            }
        },3000);
    }

    private void jumpMainActivity() {
        startActivity(new Intent(this, CppDemoActivity.class));
    }


}
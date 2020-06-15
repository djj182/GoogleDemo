package com.im30.googledemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        boolean test = false;
        String hello = String.format("%s_%d","asdas",test ? 1:0);
        int a =0;
    }
}

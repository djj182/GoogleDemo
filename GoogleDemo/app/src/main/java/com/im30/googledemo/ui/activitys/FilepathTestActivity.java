package com.im30.googledemo.ui.activitys;

import androidx.annotation.IntegerRes;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ScrollView;

import com.im30.googledemo.R;

public class FilepathTestActivity extends AppCompatActivity {

    private ScrollView fileTestViews;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_filepath_test);
        fileTestViews = (ScrollView)findViewById(R.id.fileTestViews);
        String [] buttons = this.getResources().getStringArray(R.array.file_buttons);
        for (String text :buttons) {
            Button button = new Button(this);
            button.setText(text);
            fileTestViews.addView(button);
        }
        fileTestViews.setOnClickListener(new ScrollView.OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });
    }
}
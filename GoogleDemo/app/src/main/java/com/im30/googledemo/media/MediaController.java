package com.im30.googledemo.media;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.StrictMode;
import android.provider.MediaStore;
import android.widget.Toast;

import com.im30.googledemo.BaseActivity;
import com.im30.googledemo.BuildConfig;

import java.io.File;
import java.lang.reflect.Method;

import androidx.core.app.ActivityCompat;
import androidx.core.content.FileProvider;

class MediaController {
    public static MediaController _instance = null;

    private static BaseActivity activity = null;
    public MediaController getInstance() {
        synchronized (_instance) {
            if (_instance == null) {
                _instance = new MediaController();
            }
        }

        return _instance;
    }

    /**
     * 上传图片是打开摄像机拍照
     * @param uid
     * @param idx
     */
    public void showPicturePicker1(int uid, int idx){
        if (activity == null) {
            return;
        }

        //try fix file uri error
        try {
            if(Build.VERSION.SDK_INT>=24){
                try{
                    Method m = StrictMode.class.getMethod("disableDeathOnFileUriExposure");
                    m.invoke(null);
                }catch(Exception e){
                    e.printStackTrace();
                }
            }

            if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                    (ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                            != PackageManager.PERMISSION_GRANTED ||
                            ActivityCompat.checkSelfPermission(activity, Manifest.permission.CAMERA)
                                    != PackageManager.PERMISSION_GRANTED)) {
                //申请WRITE_EXTERNAL_STORAGE权限
                ActivityCompat.requestPermissions(activity, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA},
                        1001);
            } else {
                if(checkCameraHardware()) {
                    //System.out.println("$$$showPicturePicker1,start take picture$$$");
                    Intent openCameraIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);

                    //指定照片保存路径（SD卡），image.jpg为一个临时文件，每次拍照后这个图片都会被替换
                    //把返回数据存入Intent
                    if (Build.VERSION.SDK_INT >= 24) {
                        Uri imageUri = FileProvider.getUriForFile(activity, BuildConfig.APPLICATION_ID + ".provider",
                                new File(activity.getFilesDir(),"image.jpg"));
                        openCameraIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                        openCameraIntent.putExtra(MediaStore.EXTRA_OUTPUT, imageUri);
                        activity.startActivityForResult(openCameraIntent, 2000);
                    } else {
                        Uri imageUri = Uri.fromFile(new File(Environment.getExternalStorageDirectory(),"image.jpg"));
                        openCameraIntent.putExtra(MediaStore.EXTRA_OUTPUT, imageUri);
                        activity.startActivityForResult(openCameraIntent, 2000);
                    }
                }
                else
                {
                    showPicturePicker2(uid, idx);
                }
            }
        } catch (final Exception e) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(activity,e.toString() + "\n"+e.getMessage(),Toast.LENGTH_LONG).show();
                    new AlertDialog.Builder(activity).setTitle(e.getMessage()).setMessage(e.toString()).create().show();
                }
            });
        }
    }

    /**
     * 上传头像时打开相册
     * @param uid
     * @param idx
     */
    public void showPicturePicker2(int uid, int idx){
        if (activity == null) {
            return;
        }

        if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                        != PackageManager.PERMISSION_GRANTED) {
            //申请WRITE_EXTERNAL_STORAGE权限
            ActivityCompat.requestPermissions(activity, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    1001);
        }
        else{
            Intent openAlbumIntent;
            if(android.os.Build.VERSION.SDK_INT>=android.os.Build.VERSION_CODES.KITKAT){
                //			Log.d("showPicturePicker2", "SDK_INT0:"+android.os.Build.VERSION.SDK_INT);
                openAlbumIntent = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            }else{
                //			Log.d("showPicturePicker2", "SDK_INT1:"+android.os.Build.VERSION.SDK_INT);
                openAlbumIntent = new Intent(Intent.ACTION_GET_CONTENT);
            }
            openAlbumIntent.setType("image/*");
            activity.startActivityForResult(openAlbumIntent, 2000);
        }

    }

    /**
     * 检测设备是否存在Camera硬件
     * @return
     */
    private boolean checkCameraHardware() {
        if (activity == null) {
            return false;
        }

        if (activity.getApplicationContext().getPackageManager().hasSystemFeature(
                PackageManager.FEATURE_CAMERA)) {
            // 存在
            return true;
        } else {
            // 不存在
            return false;
        }
    }

}

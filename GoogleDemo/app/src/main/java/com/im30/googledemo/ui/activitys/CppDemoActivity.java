package com.im30.googledemo.ui.activitys;

import android.Manifest;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.Camera;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Parcelable;
import android.os.StrictMode;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.im30.googledemo.R;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.FileProvider;

public class CppDemoActivity extends AppCompatActivity {

    public final static int HANDLER_SHOW_DIALOG = 1;
    public final static int HANDLER_SHOW_EDITBOX_DIALOG = 2;
    public final static int HANDLER_SHOW_TOAST = 7101;
    public final static int HADNLER_RESET_FRONTIMAGE = 9999;


    public static final int TAKE_PICTURE = 7200;
    public static final int CHOOSE_PICTURE = 7201;
    public static final int CLIP_TAKEN_PICTURE = 7202;
    public static final int CLIP_CHOOSED_PICTURE = 7203;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cpp_demo);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void openFile(String path);

    static CppDemoActivity instance = null;
    public static CppDemoActivity getInstance()
    {
        return instance;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.e("debug", "call IF.onActivityResult");
        // //Google play GameServices
        // if(mGameServiceHelper != null)
        // 	mGameServiceHelper.onActivityResult(requestCode, resultCode, data);
        // photo taker
        onPhotoActivityResult(requestCode, resultCode, data);
    }

    private final static int PLAY_SERVICES_RESOLUTION_REQUEST = 9000;

    /**
     * Check the device to make sure it has the Google Play Services APK. If
     * it doesn't, display a dialog that allows users to download the APK from
     * the Google Play Store or enable it in the device's system settings.
     */
//     private boolean checkPlayServices() {
//         int resultCode = GooglePlayServicesUtil.isGooglePlayServicesAvailable(this);
//         if (resultCode != ConnectionResult.SUCCESS) {
// //            if (GooglePlayServicesUtil.isUserRecoverableError(resultCode)) {
// //                GooglePlayServicesUtil.getErrorDialog(resultCode, this,
// //                        PLAY_SERVICES_RESOLUTION_REQUEST).show();
// //            } else {
//                 Log.i("Google Play Services", "This device is not supported.");
// ////                finish();
// //            }
//             return false;
//         }
//         return true;
//     }

    public static String intentToString(Intent intent) {
        if (intent == null) {
            return null;
        }

        return intent.toString() + " " + bundleToString(intent.getExtras());
    }

    public static String bundleToString(Bundle bundle) {
        StringBuilder out = new StringBuilder("Bundle[");

        if (bundle == null) {
            out.append("null");
        } else {
            boolean first = true;
            for (String key : bundle.keySet()) {
                if (!first) {
                    out.append(", ");
                }

                out.append(key).append('=');

                Object value = bundle.get(key);

                if (value instanceof int[]) {
                    out.append(Arrays.toString((int[]) value));
                } else if (value instanceof byte[]) {
                    out.append(Arrays.toString((byte[]) value));
                } else if (value instanceof boolean[]) {
                    out.append(Arrays.toString((boolean[]) value));
                } else if (value instanceof short[]) {
                    out.append(Arrays.toString((short[]) value));
                } else if (value instanceof long[]) {
                    out.append(Arrays.toString((long[]) value));
                } else if (value instanceof float[]) {
                    out.append(Arrays.toString((float[]) value));
                } else if (value instanceof double[]) {
                    out.append(Arrays.toString((double[]) value));
                } else if (value instanceof String[]) {
                    out.append(Arrays.toString((String[]) value));
                } else if (value instanceof CharSequence[]) {
                    out.append(Arrays.toString((CharSequence[]) value));
                } else if (value instanceof Parcelable[]) {
                    out.append(Arrays.toString((Parcelable[]) value));
                } else if (value instanceof Bundle) {
                    out.append(bundleToString((Bundle) value));
                } else {
                    out.append(value);
                }

                first = false;
            }
        }

        out.append("]");
        return out.toString();
    }

    /// photo taker
    protected void onPhotoActivityResult(int requestCode, int resultCode, Intent data) {
//  		Log.d("onPhotoActivityResult", "resultCode: "+resultCode+",requestCode:"+requestCode);
        if (resultCode == RESULT_OK) {
            switch (requestCode) {
                case TAKE_PICTURE:
                    //System.out.println("$$$TAKE_PICTURE$$$");

                    // 设置文件保存路径这里放在跟目录下
                    Uri imageUri = null;
                    if (Build.VERSION.SDK_INT >= 24) {
                        imageUri = FileProvider.getUriForFile(this,getPackageName().concat(".provider"),
                                new File(this.getFilesDir(),"image.jpg"));
                    } else {
                        imageUri = Uri.fromFile(new File(Environment.getExternalStorageDirectory(),"image.jpg"));
                    }
                    startPhotoZoom(imageUri, CLIP_TAKEN_PICTURE);
                    break;
                case CHOOSE_PICTURE:
                    if(data != null){

                        //System.out.println("$$$CHOOSE_PICTURE$$$");
                        //System.out.println(intentToString(data));

                        //照片的原始资源地址
                        Uri originalUri = data.getData();
//					Log.d("onPhotoActivityResult", "Uri: "+originalUri);
                        if(originalUri != null){
                            startPhotoZoom(originalUri, CLIP_CHOOSED_PICTURE);
                        }
                    }
                    break;
                case CLIP_TAKEN_PICTURE:
                case CLIP_CHOOSED_PICTURE:
                    if(data != null)
                    {
                        //System.out.println("$$$CLIP_TAKEN_PICTURE$$$");
                        //System.out.println(intentToString(data));
                        saveImg(data);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    private static int gameUid = -1;
    private static int index = -1;
    private static int whichButtonRequestPermission	= 0;
    private static final int showPicturePicker1Button	= 1;
    private static final int showPicturePicker2Button	= 2;
    private static int temp_uid = 0;
    private static int temp_idx = 0;
    private static final int	WRITE_EXTERNAL_STORAGE_REQUEST_CODE	= 1;
    private static final int 	GET_ACCOUNTS_REQUEST_CODE = 2;
    private static final int	CAMERA_REQUEST_CODE	= 3;
    private static final int	CAMERA_AR_REQUEST_CODE	= 4;
    private static final int	WR_EXTERNAL_STORAGE_REQUEST_CODE	= 5;
    private static final int ACC_EXTERNAL_STORAGE_REQUEST_CODE = 6;
    private static final int ACC_SETTING_REQUEST_CODE = 6;

    private static boolean temp_b_get_accounts_permis = false;
    private static boolean temp_b_get_mycard_permis = false;
    private static int REQUEST_MYCARD_MULTIPLE_PERMISSIONS = 998;

    public static boolean isCameraCanUse() {
        boolean canUse = true;
        Camera mCamera = null;
        try {
            mCamera = Camera.open();
            // setParameters 是针对魅族MX5 做的。MX5 通过Camera.open() 拿到的Camera
            // 对象不为null
            Camera.Parameters mParameters = mCamera.getParameters();
            mCamera.setParameters(mParameters);
        } catch (Exception e) {
            canUse = false;
        }
        if (mCamera != null) {
            try {
                mCamera.release();
            }
            catch(Exception e) {

            }
        }
        return canUse;
    }

    public boolean isFlyme() {
        if (Build.BRAND.contains("Meizu")) {
            return true;
        } else {
            return false;
        }
    }

    public int getARCameraPermissionWithoutRequest() {
        if( isFlyme() ) {
            if(isCameraCanUse()){
                return 1;
            }else {
                return 2;
            }
        }
        if( !checkCameraHardware()) {
            return 2;
        }
        else if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                        == PackageManager.PERMISSION_DENIED)) {
            return 2;
        }else if (android.os.Build.VERSION.SDK_INT < 23) {
            if(isCameraCanUse()){
                return 1;
            }else {
                return 2;
            }
        }
        return 1;
    }

    public int getARCameraPermission(){
        if( isFlyme() ) {
            if(isCameraCanUse()){
                return 1;
            }else {
                return 2;
            }
        }
        if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                        != PackageManager.PERMISSION_GRANTED)) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA},
                    CAMERA_AR_REQUEST_CODE);
            return 4;
        }else if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                        == PackageManager.PERMISSION_GRANTED)) {
            return 1;
        }else if (android.os.Build.VERSION.SDK_INT < 23 && checkCameraHardware()) {
            if(isCameraCanUse()){
                return 1;
            }else {
                return 2;
            }
        }
        return 2;
    }

    public void showCamera(View view){
        showPicturePicker1(0,1);
    }
    public void showAlbum(View view){
        showPicturePicker2(0,0);
    }
    // 点击“拍照”按钮时调用
    public void showPicturePicker1(int uid, int idx){
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
                    (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                            != PackageManager.PERMISSION_GRANTED ||
                            ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                                    != PackageManager.PERMISSION_GRANTED)) {
                //申请WRITE_EXTERNAL_STORAGE权限
                temp_uid = uid;
                temp_idx = idx;
                whichButtonRequestPermission = showPicturePicker1Button;
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA},
                        WRITE_EXTERNAL_STORAGE_REQUEST_CODE);
            }
            else{
                if(checkCameraHardware()) {
                    //System.out.println("$$$showPicturePicker1,start take picture$$$");
                    gameUid = uid;
                    index = idx;
                    Intent openCameraIntent = null;

//                    if (checkCameraFront()) {
//                        openCameraIntent = new Intent(MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA);
//                        openCameraIntent.putExtra("android.intent.extra.USE_FRONT_CAMERA", true);
//                        openCameraIntent.putExtra("android.intent.extras.LENS_FACING_FRONT", 1);
//                        openCameraIntent.putExtra("android.intent.extras.CAMERA_FACING", android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT);
//                    } else {
                    openCameraIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                    openCameraIntent.putExtra("android.intent.extras.CAMERA_FACING", Camera.CameraInfo.CAMERA_FACING_BACK);
//                    }

                    Uri imageUri = null;
                    if (Build.VERSION.SDK_INT >= 24) {
                        imageUri = FileProvider.getUriForFile(this,getPackageName().concat(".provider"),
                                new File(this.getFilesDir(),"image.jpg"));
                        openCameraIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                    } else {
                        imageUri = Uri.fromFile(new File(Environment.getExternalStorageDirectory(),"image.jpg"));
                    }
                    openCameraIntent.putExtra(MediaStore.EXTRA_OUTPUT, imageUri);

                    if (openCameraIntent.resolveActivity(this.getPackageManager()) != null) {
                        startActivityForResult(openCameraIntent, TAKE_PICTURE);
                    }

                }
                else
                {
                    showPicturePicker2(uid, idx);
                }
            }
        } catch (final Exception e) {
            e.printStackTrace();
        }

    }
    // 点击“相册”按钮时调用
    public void showPicturePicker2(int uid, int idx){
        if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                        != PackageManager.PERMISSION_GRANTED) {
            //申请WRITE_EXTERNAL_STORAGE权限
            temp_uid = uid;
            temp_idx = idx;
            whichButtonRequestPermission = showPicturePicker2Button;
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    WRITE_EXTERNAL_STORAGE_REQUEST_CODE);
        }
        else{
            gameUid = uid;
            index = idx;
            Intent openAlbumIntent;
            if(android.os.Build.VERSION.SDK_INT>=android.os.Build.VERSION_CODES.KITKAT){
                //			Log.d("showPicturePicker2", "SDK_INT0:"+android.os.Build.VERSION.SDK_INT);
                openAlbumIntent = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            }else{
                //			Log.d("showPicturePicker2", "SDK_INT1:"+android.os.Build.VERSION.SDK_INT);
                openAlbumIntent = new Intent(Intent.ACTION_GET_CONTENT);
            }
            openAlbumIntent.setType("image/*");
            startActivityForResult(openAlbumIntent, CHOOSE_PICTURE);
        }

    }
    int temp_camera_index = 0;
    public void showARCameraView(int index) {
        if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                        != PackageManager.PERMISSION_GRANTED) {
//			//申请WRITE_EXTERNAL_STORAGE权限
//			temp_uid = uid;
//			temp_idx = idx;
//			whichButtonRequestPermission = showPicturePicker1Button;
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA},
                    CAMERA_REQUEST_CODE);
            temp_camera_index = index;
        }
        else{
            if(checkCameraHardware()) {
//				//System.out.println("$$$showPicturePicker1,start take picture$$$");
//				gameUid = uid;
//				index = idx;
//				Intent openCameraIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
//				Uri imageUri = Uri.fromFile(new File(Environment.getExternalStorageDirectory(),"image.jpg"));
//				//指定照片保存路径（SD卡），image.jpg为一个临时文件，每次拍照后这个图片都会被替换
//				//把返回数据存入Intent
//				openCameraIntent.putExtra(MediaStore.EXTRA_OUTPUT, imageUri);
//				startActivityForResult(openCameraIntent, Cocos2dxHandler.TAKE_PICTURE);
//				mCameraHelper.initCamera();
            }
            else
            {
//				showPicturePicker2(uid, idx);

            }
        }
    }

    public static final String IMAGE_UNSPECIFIED = "image/*";
    // 保存截图文件的路径
    private static final String IMAGE_FILE_LOCATION = Environment.getExternalStorageDirectory() + "/screenshot.jpg";
    private Uri uritempFile = Uri.fromFile(new File(IMAGE_FILE_LOCATION));
    // 图片的剪裁
    public void startPhotoZoom(Uri uri, int resValue) {
        try
        {
            if (Build.VERSION.SDK_INT > Build.VERSION_CODES.N) {
                Log.d("ABTEST",uri.toString());
                String path = getPath(this,uri);
                if (path != null ) {
                    File file = new File(path);
                    uri = FileProvider.getUriForFile(this,this.getPackageName().concat(".provider"), file);
                }
            }

            Intent intent = new Intent("com.android.camera.action.CROP");
            intent.setDataAndType(uri, IMAGE_UNSPECIFIED);
            intent.putExtra("crop", "true");
            // aspectX aspectY 是宽高的比例
            intent.putExtra("aspectX", 1);
            intent.putExtra("aspectY", 1);
            // outputX outputY 是裁剪图片宽高
            intent.putExtra("outputX", 360);
            intent.putExtra("outputY", 360);
            //intent.putExtra("return-data", true);
            if (Build.VERSION.SDK_INT >= 24) {
                intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                intent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            }
            intent.putExtra("return-data", false);
            intent.putExtra(MediaStore.EXTRA_OUTPUT, uritempFile);
            intent.putExtra("outputFormat", Bitmap.CompressFormat.JPEG.toString());


            startActivityForResult(intent, resValue);
        }
        catch(Exception e){
            //ActivityNotFoundException
            e.printStackTrace();
        }
    }
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        // super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == WRITE_EXTERNAL_STORAGE_REQUEST_CODE) {
            if (grantResults != null && grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                switch (whichButtonRequestPermission){
                    case showPicturePicker1Button:
                        if( grantResults.length > 1 && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                            showPicturePicker1(temp_uid,temp_idx);
                        }

                        break;

                    case showPicturePicker2Button:
                        showPicturePicker2(temp_uid,temp_idx);
                        break;
                }
            } else if (requestCode == GET_ACCOUNTS_REQUEST_CODE) {
                temp_b_get_accounts_permis = true;
            } else if (requestCode == REQUEST_MYCARD_MULTIPLE_PERMISSIONS) {
                temp_b_get_mycard_permis = true;
                Log.d("REQUEST_MYCARD", "REQUEST_MYCARD_MULTIPLE_PERMISSIONS");
            }
            else {
                // Permission Denied

            }
        }

    }
//	public boolean isHaveGetAccountsPermission(){
//		if ((ActivityCompat.checkSelfPermission(this, Manifest.permission.GET_ACCOUNTS)
//        	== PackageManager.PERMISSION_GRANTED)) {
//			temp_b_get_accounts_permis = true;
//			// return temp_b_get_accounts_permis;
//		}
//		else if ((android.os.Build.VERSION.SDK_INT >= 23) &&
//			ActivityCompat.checkSelfPermission(this, Manifest.permission.GET_ACCOUNTS)
//        	!= PackageManager.PERMISSION_GRANTED) {
//		    //申请GET_ACCOUNTS权限
//		    whichButtonRequestPermission = showPicturePicker1Button;
//		    ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.GET_ACCOUNTS},
//		            GET_ACCOUNTS_REQUEST_CODE);
//		    temp_b_get_accounts_permis = false;
//		    // return temp_b_get_accounts_permis;
//		}
//		return temp_b_get_accounts_permis;
//	}

    /**
     * 是否含有Mycard支付所需的权限, 没有则申请权限
     * @return
     */
    public static boolean isMycardPermissionAvailable(){
        Log.d("REQUEST_MYCARD", "isMycardPermissionAvailable");
        // READ_PHONE_STATE
        boolean isReadPhoneState = ActivityCompat.checkSelfPermission(CppDemoActivity.getInstance(), Manifest.permission.READ_PHONE_STATE)
                == PackageManager.PERMISSION_GRANTED;
        // CAMERA
        boolean isCamera = ActivityCompat.checkSelfPermission(CppDemoActivity.getInstance(), Manifest.permission.CAMERA)
                == PackageManager.PERMISSION_GRANTED;

        // WRITE_EXTERNAL_STORAGE
        boolean isWriteExternalStorage = ActivityCompat.checkSelfPermission(CppDemoActivity.getInstance(), Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED;

        if (isReadPhoneState && isWriteExternalStorage && isCamera) {
            temp_b_get_mycard_permis = true;
        }
        else if ((android.os.Build.VERSION.SDK_INT >= 23) && !(isReadPhoneState && isCamera && isWriteExternalStorage)) {
            requestMycardPermissions();
            temp_b_get_mycard_permis = false;
        }
        return temp_b_get_mycard_permis;
    }

    private static void requestMycardPermissions() {

        if(android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {

            final List<String> permissionsList = new ArrayList<String>();
            addPermission(permissionsList, Manifest.permission.WRITE_EXTERNAL_STORAGE);
            addPermission(permissionsList, Manifest.permission.CAMERA);
            addPermission(permissionsList, Manifest.permission.READ_PHONE_STATE);

            if (permissionsList.size() > 0) {
                CppDemoActivity.getInstance().requestPermissions(permissionsList.toArray(new String[permissionsList.size()]),
                        REQUEST_MYCARD_MULTIPLE_PERMISSIONS);
                return;
            }
        }

    }

    @TargetApi(Build.VERSION_CODES.M)
    private static boolean addPermission(List<String> permissionsList, String permission) {
        if (CppDemoActivity.getInstance().checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
            permissionsList.add(permission);
            // Check for Rationale Option
            if (!CppDemoActivity.getInstance().shouldShowRequestPermissionRationale(permission))
                return false;
        }
        return true;
    }

    // 保存图片
    public void saveImg(Intent data){
		/*Bundle extras = data.getExtras();
		if (extras != null) {
    	    Bitmap bmp = data.getParcelableExtra("data");
			ByteArrayOutputStream stream = new ByteArrayOutputStream();
			bmp.compress(Bitmap.CompressFormat.JPEG, 75, stream);//

			// save to local
			final String absolutePath = savePhotoToSDCard(bmp, Environment.getExternalStorageDirectory().getAbsolutePath(), String.valueOf(gameUid +"_" + index));
			File dir = new File(absolutePath);
			if(dir.exists()){
				//FBUtil.nativeSendHeadImgUrl(absolutePath);
				CppDemoActivity.getInstance().runOnGLThread(new Runnable() {

					public void run() {
						FBUtil.nativeSendHeadImgUrl(absolutePath);
					}
				});
			}
		} else {
			// open from uri
			Uri imgUri = data.getData();

			try {

				InputStream is = getContentResolver().openInputStream(imgUri);
				Bitmap bmp = BitmapFactory.decodeStream(is);
				ByteArrayOutputStream bytes = new ByteArrayOutputStream();
	    		bmp.compress(Bitmap.CompressFormat.JPEG, 75, bytes);
	    		is.close();

				// save to local
				final String absolutePath = savePhotoToSDCard(bmp, Environment.getExternalStorageDirectory().getAbsolutePath(), String.valueOf(gameUid +"_" + index));
				File dir = new File(absolutePath);
				if(dir.exists()){
					//FBUtil.nativeSendHeadImgUrl(absolutePath);
					CppDemoActivity.getInstance().runOnGLThread(new Runnable() {

						public void run() {
							FBUtil.nativeSendHeadImgUrl(absolutePath);
						}
					});
				}

			} catch (Exception e) {
				e.printStackTrace();
			}
		}*/

        //System.out.println("uritempFile: "+uritempFile);
        if(uritempFile != null) {
            try {
                InputStream is = getContentResolver().openInputStream(uritempFile);
                Bitmap bmp = BitmapFactory.decodeStream(is);
                ByteArrayOutputStream stream = new ByteArrayOutputStream();
                bmp.compress(Bitmap.CompressFormat.JPEG, 75, stream);
                is.close();
                // save to local
                final String absolutePath = savePhotoToSDCard(bmp, Environment.getExternalStorageDirectory().getAbsolutePath(), String.valueOf(gameUid +"_" + index));
                File dir = new File(absolutePath);
                if(dir.exists()){
                    //FBUtil.nativeSendHeadImgUrl(absolutePath);
                    new Thread(new Runnable() {

                        public void run() {
                            openFile(absolutePath);
                        }
                    }).start();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /** 检测设备是否存在Camera硬件 */
    private boolean checkCameraHardware() {
        if (getApplicationContext().getPackageManager().hasSystemFeature(
                PackageManager.FEATURE_CAMERA)) {
            // 存在
            return true;
        } else {
            // 不存在
            return false;
        }
    }

    /** 检测设备是否存在前置摄像头 */
    private boolean checkCameraFront() {
        if (getApplicationContext().getPackageManager().hasSystemFeature(
                PackageManager.FEATURE_CAMERA_FRONT)) {
            // 存在
            return true;
        } else {
            // 不存在
            return false;
        }
    }

    /**
     * Save image to the SD card
     * @param photoBitmap
     * @param photoName
     * @param path
     */
    public String savePhotoToSDCard(Bitmap photoBitmap,String path,String photoName){
        String absolutePath = "";
        if (checkSDCardAvailable()) {
            File dir = new File(path);
            if (!dir.exists()){
                dir.mkdirs();
            }

            File photoFile = new File(path , photoName + ".jpg");
            absolutePath = photoFile.getAbsolutePath();


            FileOutputStream fileOutputStream = null;
            try {
                fileOutputStream = new FileOutputStream(photoFile);
                if (photoBitmap != null) {
                    if (photoBitmap.compress(Bitmap.CompressFormat.JPEG, 100, fileOutputStream)) {
                        fileOutputStream.flush();
                    }
                }
            } catch (FileNotFoundException e) {
                photoFile.delete();
                e.printStackTrace();
            } catch (IOException e) {
                photoFile.delete();
                e.printStackTrace();
            } finally{
                try {
                    fileOutputStream.close();
                } catch (Throwable e) {
                    //e.printStackTrace();
                }
            }
        }

        return absolutePath;
    }

    /**
     * Check the SD card
     * @return
     */
    public boolean checkSDCardAvailable(){
        return android.os.Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED);
    }

    public static boolean isExternalStoragePermissionAvailable()
    {
        int permission = ActivityCompat.checkSelfPermission(CppDemoActivity.getInstance(), Manifest.permission.WRITE_EXTERNAL_STORAGE);
        return permission == PackageManager.PERMISSION_GRANTED;
    }

    public static void gotoAppSettingView()
    {
        Intent intent=new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
        Uri uri=Uri.fromParts("package", CppDemoActivity.getInstance().getPackageName(), null);
        intent.setData(uri);
        CppDemoActivity.getInstance().startActivityForResult(intent, ACC_SETTING_REQUEST_CODE);
    }

    public static boolean isShouldShowPermitRequest(int type)
    {
        boolean ret = true;
        if (android.os.Build.VERSION.SDK_INT >= 23){
            if (type == 1){
                ret = CppDemoActivity.getInstance().shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE);
            }else if (type == 2){
                ret = CppDemoActivity.getInstance().shouldShowRequestPermissionRationale(Manifest.permission.CAMERA);
            }
        }
        return ret;
    }

    public void getSDCardPermission(){
        if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                        != PackageManager.PERMISSION_GRANTED) {
            //申请WRITE_EXTERNAL_STORAGE权限
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    WR_EXTERNAL_STORAGE_REQUEST_CODE);
        }
    }


    public static void getACCCardPermission(){
        if ((android.os.Build.VERSION.SDK_INT >= 23) &&
                ActivityCompat.checkSelfPermission(CppDemoActivity.getInstance(), Manifest.permission.WRITE_EXTERNAL_STORAGE)
                        != PackageManager.PERMISSION_GRANTED) {
            //申请WRITE_EXTERNAL_STORAGE权限
            ActivityCompat.requestPermissions(CppDemoActivity.getInstance(), new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    ACC_EXTERNAL_STORAGE_REQUEST_CODE);
        }
    }


    /**
     * @param context 上下文对象
     * @param uri     当前相册照片的Uri
     * @return 解析后的Uri对应的String
     */
    @SuppressLint("NewApi")
    public static String getPath(final Context context, final Uri uri) {
        try{
            final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;
            String pathHead = "file:///";
            // 1. DocumentProvider
            if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
                // 1.1 ExternalStorageProvider
                if (isExternalStorageDocument(uri)) {
                    final String docId = DocumentsContract.getDocumentId(uri);
                    final String[] split = docId.split(":");
                    final String type = split[0];
                    if ("primary".equalsIgnoreCase(type)) {
                        return pathHead + Environment.getExternalStorageDirectory() + "/" + split[1];
                    }
                }
                // 1.2 DownloadsProvider
                else if (isDownloadsDocument(uri)) {
                    final String id = DocumentsContract.getDocumentId(uri);
                    final Uri contentUri = ContentUris.
                            withAppendedId(Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));
                    return pathHead + getDataColumn(context,
                            contentUri, null, null);
                }
                // 1.3 MediaProvider
                else if (isMediaDocument(uri)) {
                    final String docId = DocumentsContract.getDocumentId(uri);
                    final String[] split = docId.split(":");
                    final String type = split[0];

                    Uri contentUri = null;
                    if ("image".equals(type)) {
                        contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                    } else if ("video".equals(type)) {
                        contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                    } else if ("audio".equals(type)) {
                        contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                    }

                    final String selection = "_id=?";
                    final String[] selectionArgs = new String[]{split[1]};

                    return pathHead + getDataColumn(context, contentUri, selection, selectionArgs);
                }
            }
            // 2. MediaStore (and general)
            else if ("content".equalsIgnoreCase(uri.getScheme())) {
                String path = "";
                if (isGooglePhotosUri(uri)) {//判断是否是google相册图片
                    path = uri.getLastPathSegment();
                }
                // 此处处理的是未在本地存储的云数据，还是忽略掉云数据吧，会每次都去复制一张图片。
//				else if (isGooglePlayPhotosUri(uri)) {//判断是否是Google相册图片
//					path = getImageUrlWithAuthority(context, uri);
//					Uri tmpUri = Uri.parse(path);
//					if("content".equalsIgnoreCase(tmpUri.getScheme())) {
//						path = getDataColumn(context, tmpUri,null,null);
//					}
//				}
                else {//其他类似于media这样的图片，和android4.4以下获取图片path方法类似
                    path = getDataColumn(context, uri,null,null);

                }
                Log.d("ABTEST",path);
                return path;
            }
            // 3. 判断是否是文件形式 File
            else if ("file".equalsIgnoreCase(uri.getScheme())) {
                return pathHead + uri.getPath();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }
    /**
     * 获取小于api19时获取相册中图片真正的uri
     * 对于路径是：content://media/external/images/media/33517这种的，需要转成/storage/emulated/0/DCIM/Camera/IMG_20160807_133403.jpg路径，也是使用这种方法
     * */
    public static String getDataColumn(Context context, Uri uri, String selection,
                                       String[] selectionArgs) {

        //这里开始的第二部分，获取图片的路径：低版本的是没问题的，但是sdk>19会获取不到
        Cursor cursor = null;
        String path = "";
        try {
            String[] proj = {MediaStore.Images.Media.DATA};
            //好像是android多媒体数据库的封装接口，具体的看Android文档
            cursor = context.getContentResolver().query(uri, proj, selection, selectionArgs, null);
            //获得用户选择的图片的索引值
            int column_index = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
            //将光标移至开头 ，这个很重要，不小心很容易引起越界
            cursor.moveToFirst();
            //最后根据索引值获取图片路径   结果类似：/mnt/sdcard/DCIM/Camera/IMG_20151124_013332.jpg
            path = cursor.getString(column_index);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return path;
    }

    /**
     * Google相册图片获取路径
     **/
    public static String getImageUrlWithAuthority(Context context, Uri uri) {
        InputStream is = null;
        if (uri.getAuthority() != null) {
            try {
                is = context.getContentResolver().openInputStream(uri);
                Bitmap bmp = BitmapFactory.decodeStream(is);
                return writeToTempImageAndGetPathUri(context, bmp,getFileName(uri)).toString();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }finally {
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return null;
    }


    public static String getFileName(Uri uri) {

        if (uri ==null)return null;

        String fileName =null;

        String path = uri.getPath();

        int cut = path.lastIndexOf('/');

        if (cut != -1) {

            fileName = path.substring(cut +1);

        }

        return fileName;

    }

    /**
     * 将图片流读取出来保存到手机本地相册中
     **/
    public static Uri writeToTempImageAndGetPathUri(Context inContext, Bitmap inImage,String name) {
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        inImage.compress(Bitmap.CompressFormat.JPEG, 100, bytes);
        String path = MediaStore.Images.Media.insertImage(inContext.getContentResolver(), inImage, name, null);
        return Uri.parse(path);
    }

    /**
     * @param uri
     *         The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    private static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri
     *         The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    private static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri
     *         The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    private static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }
    /**
     *  判断是否是Google相册的图片，类似于content://com.google.android.apps.photos.content/...
     **/
    public static boolean isGooglePhotosUri(Uri uri) {
        return "com.google.android.apps.photos.content".equals(uri.getAuthority());
    }

    /**
     *  判断是否是Google相册的图片，类似于content://com.google.android.apps.photos.contentprovider/0/1/mediakey:/local%3A821abd2f-9f8c-4931-bbe9-a975d1f5fabc/ORIGINAL/NONE/1075342619
     **/
    public static boolean isGooglePlayPhotosUri(Uri uri) {
        return "com.google.android.apps.photos.contentprovider".equals(uri.getAuthority());
    }
}
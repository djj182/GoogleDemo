package com.im30.googledemo;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.google.android.gms.auth.api.Auth;
import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;
import com.google.android.gms.auth.api.signin.GoogleSignInResult;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.games.Games;
import com.google.android.material.snackbar.Snackbar;
import com.im30.googledemo.ads.AdsManager;
import com.im30.googledemo.ads.AdsType;
import com.im30.googledemo.ui.activitys.CppDemoActivity;

public class GoogleloginActivity extends BaseActivity implements GoogleApiClient.ConnectionCallbacks, GoogleApiClient.OnConnectionFailedListener{

    @Override
    public void onConnected(@Nullable Bundle bundle) {

    }

    @Override
    public void onConnectionSuspended(int i) {

    }

    @Override
    public void onConnectionFailed(@NonNull ConnectionResult connectionResult) {

    }

    public interface LoginCallback {
        void onSuccess(GoogleSignInResult acct);
        void onError(Exception error);
    }

    private LoginCallback mCallback = null;
    // request codes we use when invoking an external activity
    private static final int RC_SIGN_IN = 31125;
    private static final int REQUEST_CODE_TOKEN_AUTH = 31126;
    private static final int REQUEST_GOOGLE_PLAY_SERVICES = 31127;
    private static final int RC_LEADERBOARDS = 32128;
    private static final int RC_ACHIEVEMENTS = 32129;
    private static final int RC_GET_TOKEN = 32130;

    private Activity theAppMainUI;
    private GoogleApiAvailability mGoogleService;
    private GoogleApiClient mGoogleApiClient;
    private GoogleSignInClient mGoogleSignInClient;

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        init(this, new LoginCallback() {
            @Override
            public void onSuccess(GoogleSignInResult acct) {
                int a = 0;
            }

            @Override
            public void onError(Exception error) {
                int a = 0;
            }
        });
        setContentView(R.layout.activity_googlelogin);
        Button button = (Button) findViewById(R.id.button);
        signIn();
        Snackbar.make(button,R.string.app_id,Snackbar.LENGTH_LONG).setAnchorView(button).show();

        AdsManager.getInstance().init(this);
        AdsManager.static_load(AdsType.ADS_FULLSREEN_REWARD.getValue());
    }

    public void googleLoginClick(View view) {
        signIn();
    }

    public void openAds(View view) {
        Intent intent = new Intent(this,CppDemoActivity.class);
        if (intent.resolveActivity(getPackageManager()) != null) {
            startActivity(intent);
        }
//        AdsManager.getInstance().openAds(AdsType.ADS_FULLSREEN_REWARD);
    }

    public void init(Activity theActivity, LoginCallback callback) {
        theAppMainUI = theActivity;
        mCallback = callback;
        mGoogleService = GoogleApiAvailability.getInstance();
        // https://developers.google.com/identity/sign-in/android/start
        mGoogleApiClient = new GoogleApiClient.Builder(theActivity)
                .addConnectionCallbacks(this)
                .addOnConnectionFailedListener(this)
                .addApi(Auth.GOOGLE_SIGN_IN_API, GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN)
                .addApi(Games.API).addScope(Games.SCOPE_GAMES)
                .build();
        // Build GoogleAPIClient with the Google Sign-In API
        mGoogleSignInClient = GoogleSignIn.getClient(theActivity, GoogleSignInOptions.DEFAULT_SIGN_IN);
    }

    public void signIn() {
        int code = mGoogleService.isGooglePlayServicesAvailable(theAppMainUI);
        if (code == ConnectionResult.SUCCESS) {
            // Show an account picker to let the user choose a Google account from the device.
            // If the GoogleSignInOptions only asks for IDToken and/or profile and/or email then no
            // consent screen will be shown here.
            Intent signInIntent = mGoogleSignInClient.getSignInIntent();
            theAppMainUI.startActivityForResult(signInIntent, RC_GET_TOKEN);
        } else if (mGoogleService.isUserResolvableError(code) &&
                mGoogleService.showErrorDialogFragment(theAppMainUI, code, REQUEST_GOOGLE_PLAY_SERVICES)) {
            // wait for onActivityResult call (see below)
        } else {
            Toast.makeText(theAppMainUI, mGoogleService.getErrorString(code), Toast.LENGTH_SHORT).show();
        }


    }
}
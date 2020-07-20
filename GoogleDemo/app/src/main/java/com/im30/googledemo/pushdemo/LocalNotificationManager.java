package com.im30.googledemo.pushdemo;

import android.app.Notification;
import android.os.Build;

import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

class LocalNotificationManager {

    @RequiresApi(api = Build.VERSION_CODES.KITKAT_WATCH)
    public void firebasePush() {
//        Notification notification = new Notification.Builder(null)
//                .setContentTitle("")
//                .setContentText("")
//                .setLargeIcon(null)
//                .setLargeIcon(null)
//                .setGroupSummary(true).build();
//        NotificationManagerCompat.from(null).notify(0,notification);
    }
}

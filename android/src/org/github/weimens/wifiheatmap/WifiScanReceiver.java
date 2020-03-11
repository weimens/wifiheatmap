package org.github.weimens.wifiheatmap;

import android.content.Intent;
import android.content.Context;
import android.content.BroadcastReceiver;

public class WifiScanReceiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent) {
        scanFinished();
    }

    public native static void scanFinished();
}


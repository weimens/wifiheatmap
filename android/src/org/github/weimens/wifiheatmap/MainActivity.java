package org.github.weimens.wifiheatmap;

import org.qtproject.qt5.android.bindings.QtActivity;
import android.content.IntentFilter;
import android.os.Bundle;
import android.net.wifi.WifiManager;

public class MainActivity extends QtActivity
{
    WifiScanReceiver wifiScanReceiver;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        wifiScanReceiver = new WifiScanReceiver();
    }

    public void registerBroadcastReceiver() {
        registerReceiver(wifiScanReceiver, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
    }

    public void unregisterBroadcastReceiver() {
        unregisterReceiver(wifiScanReceiver);
    }
}


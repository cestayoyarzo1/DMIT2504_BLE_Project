package dmit2504.finalproject;

import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;

import java.util.logging.Handler;

public class DeviceScanActivity extends ListActivity {


    private BluetoothAdapter bluetoothAdapter;
    private boolean mScanning;
    private Handler handler;


    // Stops scanning after 10 seconds.
    private static final long SCAN_PERIOD = 10000;
    private void scanLeDevice(final boolean enable) {
        if (enable) {

            // Stops scanning after a pre-defined scan period.
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mScanning = false;
                    bluetoothAdapter.stopLeScan(leScanCallback);
                }
            }, SCAN_PERIOD);

            mScanning = true;
            bluetoothAdapter.startLeScan(leScanCallback);
        } else {
            mScanning = false;
            bluetoothAdapter.stopLeScan(leScanCallback);
        }
    }


}

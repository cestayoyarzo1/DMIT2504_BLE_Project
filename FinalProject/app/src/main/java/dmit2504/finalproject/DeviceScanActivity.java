package dmit2504.finalproject;

import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanCallback;

import java.util.logging.Handler;


public class DeviceScanActivity extends ListActivity {


    private BluetoothAdapter bluetoothAdapter;
    private boolean mScanning;
    private Handler handler;

    private LeDeviceListAdapter leDeviceListAdapter;


    private static final long SCAN_PERIOD = 10000;

    private void scanLeDevice(final boolean enable) {
        if(enable){
            bluetoothAdapter.getBluetoothLeScanner().startScan((ScanCallback) leScanCallback);
        }
        //bluetoothAdapter.getBluetoothLeScanner()
        //BluetoothDevice
    }



    private BluetoothAdapter.LeScanCallback leScanCallback =
            new BluetoothAdapter.LeScanCallback() {
                @Override
                public void onLeScan(final BluetoothDevice device, int rssi, byte[] scanRecord) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            leDeviceListAdapter.addDevice(device);
                            leDeviceListAdapter.notifyDataSetChanged();
                        }
                    });
                }
            };
}




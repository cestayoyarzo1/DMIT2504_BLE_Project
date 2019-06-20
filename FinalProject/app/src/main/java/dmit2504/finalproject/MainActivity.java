package dmit2504.finalproject;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.WindowManager;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothManager bluetoothManager;
    private BluetoothGattServer bluetoothGattServer;
    private BluetoothGatt bluetoothGatt;
    private boolean mScanning;
    private Handler handler;
    //private TextView bleTopText;
    //private ArrayAdapter<String> leAdapter;
    private boolean mCharacteristicWritten = true;
    private BluetoothGattCharacteristic customCharacteristic;

    // Stops scanning after 10 seconds.
    private static final long SCAN_PERIOD = 10000;


    //Locally-defined integer (which must be greater than 0) that the system passes back to you
    // in your onActivityResult(int, int, android.content.Intent) implementation as the requestCode parameter.
    static final int REQUEST_ENABLE_BT = 3;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //setContentView(R.layout.activity_device_scan);
        handler = new Handler();
        //this.bleTopText = findViewById(R.id.bleTopText);

        //Do not go to sleep
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


        //Initialize Bluetooth adapter
        bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        bluetoothAdapter = bluetoothManager.getAdapter();

        // Use this check to determine whether BLE is supported on the device. Then
        // you can selectively disable BLE-related features.
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Toast.makeText(this, R.string.ble_not_supported, Toast.LENGTH_SHORT).show();
            finish();
        }

        //Enable bluetooth if not yet enabled
        if(bluetoothAdapter == null || !bluetoothAdapter.isEnabled()){
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
    }



    //le Callback
    private ScanCallback leScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
        }
    };

    
}

package dmit2504.finalproject;

import android.Manifest;
import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;

import java.util.List;

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
    private static final long SCAN_PERIOD = 20000;


    //Locally-defined integer (which must be greater than 0) that the system passes back to you
    // in your onActivityResult(int, int, android.content.Intent) implementation as the requestCode parameter.
    static final int REQUEST_ENABLE_BT = 3;


    TextView statusTextView;

    public static final String APP_NAME = "Robot Control by Carlos Estay";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        statusTextView = findViewById(R.id.activity_main_status_text);
        statusTextView.setText("");

        getSupportActionBar().setTitle(APP_NAME);


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

        if (Build.VERSION.SDK_INT >= 23) {
            if (this.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(this, "Please grant location access so this app can detect peripherals.", Toast.LENGTH_SHORT).show();
//                final AlertDialog.Builder builder = new AlertDialog.Builder(this);
//                builder.setTitle("This app needs location access");
//                builder.setMessage("Please grant location access so this app can detect peripherals.");
//                builder.setPositiveButton(android.R.string.ok, null);
//                builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
//                    @RequiresApi(api = Build.VERSION_CODES.M)
//                    @Override
//                    public void onDismiss(DialogInterface dialog) {
//                        requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, Integer.parseInt(Manifest.permission.ACCESS_COARSE_LOCATION));
//                    }
//                });
//                builder.show();
            }
        }



        //scanLeDevice(true);

    }

    //Main Menu Loading

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Create an instance of the Menu inflater
        MenuInflater inflater = getMenuInflater();
        // Inflate the menu
        inflater.inflate(R.menu.main_menu, menu);
        // Return true if the menu inflated OK
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Get the id of the selected menu item
        switch (item.getItemId()) {
            case R.id.menu_control:
                // create an intent to start the ViewRemoteDataActivity
                Intent intent = new Intent(this, RemoteControlActivity.class);
                startActivity(intent);
                return true;

            default:
                return super.onOptionsItemSelected(item);
        }
    }

    //Scanning
    public void onScanPress(View view)
    {
        scanLeDevice(true);
//        Intent intent = new Intent(this, RemoteControlActivity.class);
//        startActivity(intent);
    }

    @TargetApi(21)
    private void scanLeDevice(final boolean enable) {

        final BluetoothLeScanner bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();
        //        leAdapter.clear();
        //        leAdapter.notifyDataSetChanged();
        if (enable) {
            statusTextView.setText("Scanning...");
            // Stops scanning after a pre-defined scan period.

            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mScanning = false;
                    bluetoothLeScanner.stopScan(leScanCallback);
                    statusTextView.setText("Scan stopped");
                }
            }, SCAN_PERIOD);
            mScanning = true;
            bluetoothLeScanner.startScan(leScanCallback);
        } else {
            mScanning = false;
            bluetoothLeScanner.stopScan(leScanCallback);
        }
    }

    //le Callback

    private ScanCallback leScanCallback = new ScanCallback() {

        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            Toast.makeText(getApplicationContext(), "Device Found", Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            System.out.println("BLE// onBatchScanResults");
            Toast.makeText(getApplicationContext(), "Devices Found", Toast.LENGTH_SHORT).show();
            for (ScanResult sr : results) {
                Log.i("ScanResult - Results", sr.toString());
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Toast.makeText(getApplicationContext(), "Scan Failed", Toast.LENGTH_SHORT).show();
            System.out.println("BLE// onScanFailed");
            Log.e("Scan Failed", "Error Code: " + errorCode);
        }
    };

}

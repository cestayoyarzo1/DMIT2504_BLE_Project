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
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Handler;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.List;

public class MainActivity extends AppCompatActivity {

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothManager bluetoothManager;
    private BluetoothGattServer bluetoothGattServer;
    private boolean mScanning;
    private Handler handler;
    //private TextView bleTopText;
    //private ArrayAdapter<String> leAdapter;
    private boolean mCharacteristicWritten = true;

    // Stops scanning after 10 seconds.
    private static final long SCAN_PERIOD = 20000;


    //Locally-defined integer (which must be greater than 0) that the system passes back to you
    // in your onActivityResult(int, int, android.content.Intent) implementation as the requestCode parameter.
    static final int REQUEST_ENABLE_BT = 3;


    TextView statusTextView;
    ListView devicesListView;
    LeDeviceListAdapter deviceListAdapter;

    public static final String APP_NAME = "Robot Control by Carlos Estay";

    BluetoothDevice robotDevice, connectedDevice;
    BluetoothGatt bluetoothGatt;
    private BluetoothGattCharacteristic customCharacteristic;

    TextView directionTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        statusTextView = findViewById(R.id.activity_main_status_text);
        statusTextView.setText("");

        getSupportActionBar().setTitle(APP_NAME);

        devicesListView = findViewById(R.id.activity_main_devices_listview);
        deviceListAdapter = new LeDeviceListAdapter(this);
        devicesListView.setAdapter(deviceListAdapter);

        directionTextView = findViewById(R.id.activity_main_direction_input);


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



    private MainActivity getActivity() {

        return this;
    }

    //le Callback

    private ScanCallback leScanCallback = new ScanCallback() {

        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            super.onScanResult(callbackType, result);
            //Toast.makeText(getApplicationContext(), "Device Found", Toast.LENGTH_SHORT).show();
            if (result.getDevice().getName() != null && !deviceListAdapter.contains(result)) {
                deviceListAdapter.addResult(result);//add found device to the listview

                //register event for clicking items
                devicesListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                        ScanResult selectedResult = (ScanResult) deviceListAdapter.getItem(position);
                        robotDevice = selectedResult.getDevice();
                        //Toast.makeText(getApplicationContext(), "Clicked on device:" + robotDevice.getName(), Toast.LENGTH_SHORT).show();
                        bluetoothGatt = robotDevice.connectGatt(getActivity(), false, gattCallback);
                        statusTextView.setText("Connecting to " + robotDevice.getName() + "...");

                    }
                });
            }
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

    //Gatt Callback
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            //super.onConnectionStateChange(gatt, status, newState);
            connectedDevice = gatt.getDevice();
            switch(newState ){
                case BluetoothProfile.STATE_CONNECTED:
                    Snackbar.make(findViewById(android.R.id.content), "Connected to " + connectedDevice.getName(), Snackbar.LENGTH_LONG).setAction("No action", null).show();
                    //Toast.makeText(getApplicationContext(), "Connected to " + connectedDevice.getName(), Toast.LENGTH_LONG).show();
                    statusTextView.setText("Connected to " + connectedDevice.getName());
                    bluetoothGatt.discoverServices();
                    break;

                case BluetoothProfile.STATE_DISCONNECTED:
                    Snackbar.make(findViewById(android.R.id.content), "Disconnected from " + connectedDevice.getName(), Snackbar.LENGTH_LONG).setAction("No action", null).show();
                    //Toast.makeText(getApplicationContext(), "Disconnected from " + connectedDevice.getName(), Toast.LENGTH_LONG).show();
                    break;

                    default:
                break;
            }


        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            Snackbar.make(findViewById(android.R.id.content), "Services discovered", Snackbar.LENGTH_LONG).setAction("No action", null).show();
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
        }
    };


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

    //Scanning
    public void onScanPress(View view) {
        if(!mScanning) {
            scanLeDevice(true);
        }
    }

    public void onSendDirection(View view){
        String value = directionTextView.getText().toString();
        Toast.makeText(this, "You are sending :" + value, Toast.LENGTH_LONG).show();
//        customCharacteristic.setValue(value);
//        bluetoothGatt.writeCharacteristic(customCharacteristic);
    }
}

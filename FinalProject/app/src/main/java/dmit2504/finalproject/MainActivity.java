package dmit2504.finalproject;

import android.Manifest;
import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
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
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothManager bluetoothManager;
    private BluetoothGattServer bluetoothGattServer;
    private boolean mScanning;
    private Handler handler;

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

    TextView scanFilterTextView;

    static final String SEARCH_TAG = "Robot";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getSupportActionBar().setTitle(APP_NAME);

        statusTextView = findViewById(R.id.activity_main_status_text);
        statusTextView.setText("");

        devicesListView = findViewById(R.id.activity_main_devices_listview);
        deviceListAdapter = new LeDeviceListAdapter(this);
        devicesListView.setAdapter(deviceListAdapter);

        scanFilterTextView = findViewById(R.id.activity_main_scan_filter);


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

        //Check and request permission if necessary
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {

            // Permission is not granted
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.ACCESS_COARSE_LOCATION)) {
            } else {
                // No explanation needed; request the permission
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                        1);
                // result of the request.
            }
        } else {
            // Permission has already been granted
        }

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
                Intent intent = new Intent(getApplicationContext(), RemoteControlActivity.class);
                intent.putExtra("ROBOT", robotDevice);
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
            if (result.getDevice().getName() != null && result.getDevice().getName().contains(scanFilterTextView.getText())) {
                if(!deviceListAdapter.contains(result)){
                    deviceListAdapter.addResult(result);//add found device to the listView if not already added
                }
                //register event for clicking items
                devicesListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                        ScanResult selectedResult = (ScanResult) deviceListAdapter.getItem(position);
                        robotDevice = selectedResult.getDevice();
                        bluetoothGatt = robotDevice.connectGatt(getActivity(), false, gattCallback);
                        //Stop Scanning if connecting to a device
                        scanLeDevice(false);
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
            //super.onServicesDiscovered(gatt, status);
            Snackbar.make(findViewById(android.R.id.content), "Services discovered", Snackbar.LENGTH_LONG).setAction("No action", null).show();

            List<BluetoothGattService> services = gatt.getServices();

            String serviceUUID = "D973f2E0-B19E-11E2-9E96-0800200C9A66";
            String charUUID = "D973f2E2-B19E-11E2-9E96-0800200C9A66";

            BluetoothGattService service = gatt.getService(UUID.fromString(serviceUUID));
            customCharacteristic = service.getCharacteristic(UUID.fromString(charUUID));

            if (customCharacteristic == null) {
                Snackbar.make(findViewById(android.R.id.content), "Characteristic not found", Snackbar.LENGTH_LONG).setAction("No action", null).show();
            }
            else{
                Snackbar.make(findViewById(android.R.id.content), "Characteristic: " + customCharacteristic.getInstanceId(), Snackbar.LENGTH_LONG).setAction("No action", null).show();
                //Robot connected, open remote control activity
                Intent intent = new Intent(getApplicationContext(), RemoteControlActivity.class);
                intent.putExtra("ROBOT", robotDevice);
                startActivity(intent);
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
            Snackbar.make(findViewById(android.R.id.content), "Characteristic Read", Snackbar.LENGTH_LONG).setAction("No action", null).show();
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);
            Snackbar.make(findViewById(android.R.id.content), "Characteristic Written", Snackbar.LENGTH_LONG).setAction("No action", null).show();
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
        }
    };


    @TargetApi(21)
    private void scanLeDevice(final boolean enable) {

        final BluetoothLeScanner bluetoothLeScanner = bluetoothAdapter.getBluetoothLeScanner();
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

}

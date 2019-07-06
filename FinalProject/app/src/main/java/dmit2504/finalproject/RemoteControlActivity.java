package dmit2504.finalproject;


import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.graphics.Color;
import android.os.Bundle;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import java.text.DecimalFormat;
import java.util.List;
import java.util.UUID;

public class RemoteControlActivity extends AppCompatActivity {

    ImageButton forwardButton;
    ImageButton reverseButton;
    ImageButton rightButton;
    ImageButton leftButton;

    SeekBar speedSeekBar;

    TextView statusTextView;

    BluetoothDevice robot, replyingRobot;

    BluetoothGatt bluetoothGatt;
    BluetoothGattCharacteristic customCharacteristic;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remote_control);
        forwardButton = findViewById(R.id.activity_remote_control_forward_button);
        reverseButton = findViewById(R.id.activity_remote_control_reverse_button);
        rightButton = findViewById(R.id.activity_remote_control_right_button);
        leftButton = findViewById(R.id.activity_remote_control_left_button);
        speedSeekBar = findViewById(R.id.activity_remote_control_bar);

        statusTextView = findViewById(R.id.activity_remote_control_status_textview);

        forwardButton.setOnTouchListener(buttonForwardListener);
        reverseButton.setOnTouchListener(buttonReverseListener);
        rightButton.setOnTouchListener(buttonRightListener);
        leftButton.setOnTouchListener(buttonLeftListener);
        speedSeekBar.setOnSeekBarChangeListener(seekBarChangeListener);
        speedSeekBar.setProgress(speedSeekBar.getMax()/2);

        robot = getIntent().getParcelableExtra("ROBOT");
        if(robot != null){
            Toast.makeText(this, "Remote device Connected: " + robot.getName(), Toast.LENGTH_LONG);
            bluetoothGatt = robot.connectGatt(getActivity(), false, gattCallback);
        }
        else{
            Toast.makeText(this, "Nothing to control", Toast.LENGTH_LONG);
        }

    }

    private RemoteControlActivity getActivity() {

        return this;
    }

    //Gatt Callback
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            //super.onConnectionStateChange(gatt, status, newState);
            replyingRobot = gatt.getDevice();
            switch(newState ){
                case BluetoothProfile.STATE_CONNECTED:
                    Snackbar.make(findViewById(android.R.id.content), "Connected to " + replyingRobot.getName(), Snackbar.LENGTH_LONG).setAction("No action", null).show();
                    //Toast.makeText(getApplicationContext(), "Connected to " + connectedDevice.getName(), Toast.LENGTH_LONG).show();
                    statusTextView.setText("Connected to " + replyingRobot.getName());
                    bluetoothGatt.discoverServices();
                    break;

                case BluetoothProfile.STATE_DISCONNECTED:
                    Snackbar.make(findViewById(android.R.id.content), "Disconnected from " + replyingRobot.getName(), Snackbar.LENGTH_LONG).setAction("No action", null).show();
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

            String serviceUUID = "D973f2E0-B19E-11E2-9E96-0800200C9A66";
            String charUUID = "D973f2E2-B19E-11E2-9E96-0800200C9A66";

            List<BluetoothGattService> services = gatt.getServices();
            BluetoothGattService service = gatt.getService(UUID.fromString(serviceUUID));
            customCharacteristic = service.getCharacteristic(UUID.fromString(charUUID));
//            customCharacteristic =
//                    gatt.getService(UUID.fromString(serviceUUID))
//                            .getCharacteristic(UUID.fromString(charUUID));
            if (customCharacteristic == null) {
                Snackbar.make(findViewById(android.R.id.content), "Characteristic not found", Snackbar.LENGTH_LONG).setAction("No action", null).show();
            }
            else{
                Snackbar.make(findViewById(android.R.id.content), "Characteristic: " + customCharacteristic.getInstanceId(), Snackbar.LENGTH_LONG).setAction("No action", null).show();
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
            //Snackbar.make(findViewById(android.R.id.content), "Characteristic Written", Snackbar.LENGTH_LONG).setAction("No action", null).show();
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
        }
    };



    private View.OnTouchListener buttonForwardListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    forwardButton.animate().scaleX(1.1f);
                    forwardButton.animate().scaleY(1.1f);
                    forwardButton.setColorFilter(Color.argb(100, 0,255,0));
                    //Toast.makeText(getApplicationContext(), "Button FORWARD down", Toast.LENGTH_LONG).show();
                    statusTextView.setText("FORWARD");
                    customCharacteristic.setValue("@MF0");
                    bluetoothGatt.writeCharacteristic(customCharacteristic);
                    break;

                case MotionEvent.ACTION_UP:
                    //Toast.makeText(getApplicationContext(), "Button FORWARD up", Toast.LENGTH_LONG).show();
                    forwardButton.animate().scaleX(1);
                    forwardButton.animate().scaleY(1);
                    //forwardButton.clearAnimation();
                    forwardButton.setColorFilter(Color.alpha(0));
                    statusTextView.setText("IDLE");
                    customCharacteristic.setValue("@M00");
                    bluetoothGatt.writeCharacteristic(customCharacteristic);
                    break;
                }
            return false;
        }
    };


    private SeekBar.OnSeekBarChangeListener seekBarChangeListener =  new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            Toast.makeText(getApplicationContext(), "Speed: " + progress + "0%", Toast.LENGTH_SHORT).show();
            if(robot != null){
                customCharacteristic.setValue("@S" + String.format("%02d",progress));
                bluetoothGatt.writeCharacteristic(customCharacteristic);
            }
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {

        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {

        }
    };

    private View.OnTouchListener buttonReverseListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    reverseButton.animate().scaleX(1.1f);
                    reverseButton.animate().scaleY(1.1f);
                    reverseButton.setColorFilter(Color.argb(100, 0,255,0));
                    //Toast.makeText(getApplicationContext(), "Button REVERSE down", Toast.LENGTH_LONG).show();
                    statusTextView.setText("REVERSE");
                    customCharacteristic.setValue("@MB0");
                    bluetoothGatt.writeCharacteristic(customCharacteristic);
                    break;

                case MotionEvent.ACTION_UP:
                    reverseButton.animate().scaleX(1);
                    reverseButton.animate().scaleY(1);
                    reverseButton.setColorFilter(Color.alpha(0));
                    //Toast.makeText(getApplicationContext(), "Button REVERSE up", Toast.LENGTH_LONG).show();
                    statusTextView.setText("IDLE");
                    customCharacteristic.setValue("@M00");
                    bluetoothGatt.writeCharacteristic(customCharacteristic);
                    break;
            }
            return false;
        }
    };

    private View.OnTouchListener buttonRightListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    rightButton.animate().scaleX(1.1f);
                    rightButton.animate().scaleY(1.1f);
                    rightButton.setColorFilter(Color.argb(100, 0,255,0));
                    //Toast.makeText(getApplicationContext(), "Button RIGHT down", Toast.LENGTH_LONG).show();
                    statusTextView.setText("RIGHT");
                    customCharacteristic.setValue("@MR0");
                    bluetoothGatt.writeCharacteristic(customCharacteristic);
                    break;

                case MotionEvent.ACTION_UP:
                    rightButton.animate().scaleX(1);
                    rightButton.animate().scaleY(1);
                    rightButton.setColorFilter(Color.alpha(0));
                    //Toast.makeText(getApplicationContext(), "Button RIGHT up", Toast.LENGTH_LONG).show();
                    statusTextView.setText("IDLE");
                    customCharacteristic.setValue("@M00");
                    bluetoothGatt.writeCharacteristic(customCharacteristic);
                    break;
            }
            return false;
        }
    };

    private View.OnTouchListener buttonLeftListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    leftButton.animate().scaleX(1.1f);
                    leftButton.animate().scaleY(1.1f);
                    leftButton.setColorFilter(Color.argb(100, 0,255,0));
                    //Toast.makeText(getApplicationContext(), "Button LEFT down", Toast.LENGTH_LONG).show();
                    statusTextView.setText("LEFT");
                    customCharacteristic.setValue("@ML0");
                    bluetoothGatt.writeCharacteristic(customCharacteristic);
                    break;

                case MotionEvent.ACTION_UP:
                    leftButton.animate().scaleX(1);
                    leftButton.animate().scaleY(1);
                    leftButton.setColorFilter(Color.alpha(0));
                    //Toast.makeText(getApplicationContext(), "Button LEFT up", Toast.LENGTH_LONG).show();
                    statusTextView.setText("IDLE");
                    customCharacteristic.setValue("@M00");
                    bluetoothGatt.writeCharacteristic(customCharacteristic);
                    break;
            }
            return false;
        }
    };
}

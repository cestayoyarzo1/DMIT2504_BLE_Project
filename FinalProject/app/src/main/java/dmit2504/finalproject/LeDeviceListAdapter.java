package dmit2504.finalproject;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

import java.util.ArrayList;

public class LeDeviceListAdapter extends BaseAdapter {

    private Context mContext;
    private ArrayList<BluetoothDevice> mLeDeviceList = new ArrayList<>();


    public void LeDeviceListAdapter(Context context){
        mContext = context;
        mLeDeviceList = new ArrayList<>();
    }

    public void addDevice(BluetoothDevice device) {
        mLeDeviceList.add((device));
    }


    @Override
    public int getCount() {
        return mLeDeviceList.size();
    }

    @Override
    public Object getItem(int position) {
        return mLeDeviceList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        return null;
    }
}

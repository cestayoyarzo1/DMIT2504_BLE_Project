package dmit2504.finalproject;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;

public class LeDeviceListAdapter extends BaseAdapter {

    private Context mContext;
    private ArrayList<ScanResult> mLeDeviceList;

    public LeDeviceListAdapter(Context context) {
        mContext = context;
        mLeDeviceList = new ArrayList<>();
    }

    public boolean contains(ScanResult result){
        if(mLeDeviceList.contains(result)) {
            return true;
        }else{
             return false;
        }
    }

    public void addResult(ScanResult result) {
        mLeDeviceList.add((result));
        notifyDataSetChanged();
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


    //@RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        TextView nameTextView = null;
        TextView addressTextView = null;
        TextView powerTextView = null;
        TextView dataTextView = null;

        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = inflater.inflate(R.layout.listview_item, null);
        nameTextView = convertView.findViewById(R.id.listview_item_name);
        addressTextView = convertView.findViewById(R.id.listview_item_address);
        powerTextView = convertView.findViewById(R.id.listview_item_power);
        dataTextView = convertView.findViewById(R.id.listview_item_data);


        ScanResult resultData = (ScanResult) getItem(position);
        nameTextView.setText(resultData.getDevice().getName());
        addressTextView.setText(resultData.getDevice().getAddress());
        powerTextView.setText("");
        dataTextView.setText("");

        return convertView;
    }
}

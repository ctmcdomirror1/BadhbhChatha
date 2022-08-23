package com.tea.cmcdona2.casper.Socs;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.tea.cmcdona2.casper.R;

public class SocsAdapter extends ArrayAdapter<SocItem> {

    private Context context;
    private int layoutResourceId;
    private ArrayList<SocItem> data = new ArrayList<SocItem>();

    public int numSocs;
    public boolean[] actives;
    public boolean previouslyLaunched;

    public SocsAdapter(Context context, int layoutResourceId, ArrayList<SocItem> data) {
        super(context, layoutResourceId, data);
        this.layoutResourceId = layoutResourceId;
        this.context = context;
        this.data = data;

        actives = new boolean[numSocs];
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        SharedPreferences appPrefs = getContext().getSharedPreferences("appPrefs", 0);
        Boolean previouslyLaunchedBoolTobool = appPrefs.getBoolean("previouslyLaunched", false);
        previouslyLaunched = previouslyLaunchedBoolTobool;


        if (previouslyLaunched) {
            actives = loadArray("idsActive", this.getContext());

        }

        View row = convertView;
        ViewHolder holder;

        if (row == null) {
            LayoutInflater inflater = ((Activity) context).getLayoutInflater();
            row = inflater.inflate(layoutResourceId, parent, false);
            holder = new ViewHolder();
            holder.imageTitle = (TextView) row.findViewById(R.id.text);
            holder.image = (ImageView) row.findViewById(R.id.image);
            row.setTag(holder);
        } else {
            holder = (ViewHolder) row.getTag();
        }

        if (previouslyLaunched) {
            if (actives[position])
                row.setActivated(true);
        }

        SocItem item = data.get(position);
        holder.imageTitle.setText(item.getTitle());
        holder.image.setImageBitmap(item.getImage());

        return row;
    }

    static class ViewHolder {
        TextView imageTitle;
        ImageView image;
    }

    public boolean[] loadArray(String arrayName, Context mContext) {

        SharedPreferences appPrefs = mContext.getSharedPreferences("appPrefs", 0);
        int size = appPrefs.getInt(arrayName + "_size", 0);
        boolean array[] = new boolean[size];
        for (int i = 0; i < size; i++)
            array[i] = appPrefs.getBoolean(arrayName + "_" + i, false);
        return array;
    }

    @Override

    public int getViewTypeCount() {

        return getCount();
    }

    @Override
    public int getItemViewType(int position) {

        return position;
    }

}
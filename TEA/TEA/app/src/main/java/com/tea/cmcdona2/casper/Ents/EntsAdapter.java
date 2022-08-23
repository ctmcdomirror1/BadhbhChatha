package com.tea.cmcdona2.casper.Ents;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.tea.cmcdona2.casper.R;

import java.util.ArrayList;
import java.util.List;

public class EntsAdapter extends ArrayAdapter {
    List list = new ArrayList();

    public EntsAdapter(Context context, int resource) {
        super(context, resource);

    }

    static class DataHandler {
        ImageView Poster;
        TextView title;
        TextView timing;
    }

    @Override
    public void add(Object object) {
        super.add(object);
        list.add(object);
    }

    @Override
    public int getCount() {
        return this.list.size();
    }

    @Override
    public Object getItem(int position) {
        return this.list.get(position);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View row;
        row = convertView;
        DataHandler handler;
        if (convertView == null) {
            LayoutInflater inflater = (LayoutInflater) this.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            row = inflater.inflate(R.layout.ent_item, parent, false);
            handler = new DataHandler();
            handler.Poster = (ImageView) row.findViewById(R.id.event_poster);
            handler.title = (TextView) row.findViewById(R.id.event_title);
            handler.timing = (TextView) row.findViewById(R.id.event_timming);
            row.setTag(handler);
        } else {
            handler = (DataHandler) row.getTag();
        }

        EntItem dataProvider;
        dataProvider = (EntItem) this.getItem(position);
        handler.Poster.setImageBitmap(dataProvider.getEvent_poster_resource());
        handler.title.setText(dataProvider.getEvents_title());
        handler.timing.setText(dataProvider.getEvents_timing());


        return row;

    }
}

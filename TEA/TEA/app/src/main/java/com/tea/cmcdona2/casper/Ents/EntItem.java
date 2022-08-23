package com.tea.cmcdona2.casper.Ents;

import android.graphics.Bitmap;

public class EntItem {


    private Bitmap bi;
    private String events_title;
    private String events_timing;


    public String getEvents_title() {
        return events_title;
    }

    public EntItem(Bitmap bi, String events_title, String events_timming) {
        this.setEvent_poster_resource(bi);
        this.setEvents_title(events_title);
        this.setEvents_timing(events_timming);
    }

    public void setEvents_title(String events_title) {
        this.events_title = events_title;
    }

    public String getEvents_timing() {
        return events_timing;
    }

    public void setEvents_timing(String events_timing) {
        this.events_timing = events_timing;
    }

    public Bitmap getEvent_poster_resource() {
        return bi;
    }

    public void setEvent_poster_resource(Bitmap bi) {
        this.bi = bi;
    }


}
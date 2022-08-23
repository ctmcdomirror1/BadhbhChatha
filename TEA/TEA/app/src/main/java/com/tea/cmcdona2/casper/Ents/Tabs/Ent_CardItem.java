package com.tea.cmcdona2.casper.Ents.Tabs;

import android.graphics.Bitmap;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by gargab on 18/03/16.
 */
public class Ent_CardItem {

    public String event_name;
    public String event_timing;
    public Bitmap event_image;

    Ent_CardItem(String name, String timing, Bitmap image) {
        this.event_name = name;
        this.event_timing = timing;
        this.event_image = image;
    }


}

package com.tea.cmcdona2.casper.Socs;

import android.graphics.Bitmap;

public class SocItem {
    private Bitmap image;
    private String title;

    public SocItem(Bitmap image, String title) {
        super();
        this.image = image;
        this.title = title;
    }

    public Bitmap getImage() {
        return image;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }
}

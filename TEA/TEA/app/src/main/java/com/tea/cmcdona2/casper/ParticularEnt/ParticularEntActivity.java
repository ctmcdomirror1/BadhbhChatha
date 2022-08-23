package com.tea.cmcdona2.casper.ParticularEnt;
import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.support.design.widget.CollapsingToolbarLayout;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.GravityCompat;
import android.support.v4.view.ViewPager;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.util.TypedValue;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewAnimationUtils;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.support.v7.widget.Toolbar;
import android.widget.Toast;

import com.tea.cmcdona2.casper.R;

public class ParticularEntActivity extends AppCompatActivity{
    public Boolean buttonVisible;

    private void setWindowContentOverlayCompat() {
        if (Build.VERSION.SDK_INT == Build.VERSION_CODES.JELLY_BEAN_MR2) {
            // Get the content view
            View contentView = findViewById(android.R.id.content);

            // Make sure it's a valid instance of a FrameLayout
            if (contentView instanceof FrameLayout) {
                TypedValue tv = new TypedValue();

                // Get the windowContentOverlay value of the current theme
                if (getTheme().resolveAttribute(
                        android.R.attr.windowContentOverlay, tv, true)) {

                    // If it's a valid resource, set it as the foreground drawable
                    // for the content view
                    if (tv.resourceId != 0) {
                        ((FrameLayout) contentView).setForeground(
                                getResources().getDrawable(tv.resourceId));
                    }
                }
            }
        }
    }

    //public Toolbar toolbar;

    @Override

    protected void onCreate(Bundle savedInstanceState) {

        setWindowContentOverlayCompat();
        //make the status bar translucent
        this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);

        //Remove title bar
        //this.requestWindowFeature(Window.FEATURE_NO_TITLE);

        //Remove notification bar
        //this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        //set content view AFTER ABOVE sequence (to avoid crash)
        //this.setContentView(R.layout.particular_ent_activity);

        super.onCreate(savedInstanceState);
        ViewPager viewpager;
        setContentView(R.layout.particular_ent_activity);
        FragmentPageAdapter ft;
        viewpager = (ViewPager) findViewById(R.id.pager1);
        ft = new FragmentPageAdapter(getSupportFragmentManager());
        viewpager.setAdapter(ft);

        //hide app name
        getSupportActionBar().setDisplayShowTitleEnabled(false);

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.clear();
         //Inflate the menu items for use in the action bar
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.back_button, menu);
        return super.onCreateOptionsMenu(menu);
    }

    public void backPressed(MenuItem item) {
        onBackPressed();
    }

    public class FragmentPageAdapter extends FragmentPagerAdapter {
        public FragmentPageAdapter(FragmentManager fm) {
            super(fm);
        }


        String temp = getIntent().getStringExtra("swipeEventId");
        String[] swipeEventId = temp.split(",");
        String temp1 = getIntent().getStringExtra("eventPosition");
        String[] eventPosition = temp1.split(",");
        @Override
        public Fragment getItem(int arg0) {
            // TODO Auto-generated method stub

            int i = 0;
            int count = getCount();

            for (i=0;i< count; i++)

            {
                if (arg0 == i) {
                    Log.v("DisplayDates", "" + arg0);

                    Bundle bundle = new Bundle();
                    int position = getIntent().getIntExtra("swipePosition", 0);
                    int pos = getIntent().getIntExtra("eventPosition", 0);
                    Log.v("testCase", ""+pos);
                    bundle.putInt("eventId", Integer.parseInt(swipeEventId[position + i]));
                    bundle.putInt("eventPosition", Integer.parseInt(eventPosition[position + i]));
                    Fragment fragment = new ParticularEntFrag();
                    fragment.setArguments(bundle);
                    return fragment;
                }

            }
            return null;
        }
        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            int count = getIntent().getIntExtra("swipeCount", 0);
            int swipePosition = getIntent().getIntExtra("swipePosition", 0);
            count = count - swipePosition;
            return count;
        }
    }

    @Override
    public void onBackPressed(){
        this.finish();
    }

}
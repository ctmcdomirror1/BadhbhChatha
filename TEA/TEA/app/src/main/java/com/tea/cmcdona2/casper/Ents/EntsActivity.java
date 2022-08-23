package com.tea.cmcdona2.casper.Ents;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.GravityCompat;
import android.support.v4.view.ViewPager;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Toolbar;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.tea.cmcdona2.casper.Other.Constants;
import com.tea.cmcdona2.casper.ParticularSoc.ParticularSocActivity;
import com.tea.cmcdona2.casper.Socs.SocsActivity;
import com.tea.cmcdona2.casper.Splash.SplashActivity;
import com.tea.cmcdona2.casper.Ents.Tabs.LaterFrag;
import com.tea.cmcdona2.casper.R;
import com.tea.cmcdona2.casper.Ents.Tabs.TodayFrag;
import com.tea.cmcdona2.casper.Ents.Tabs.TomorrowFrag;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class EntsActivity extends ActionBarActivity implements ActionBar.TabListener, AdapterView.OnItemClickListener {

    SectionsPagerAdapter mSectionsPagerAdapter;

    public ViewPager mViewPager;
    public int len;
    public Bitmap[] bm;
    public String[] eventName;
    public ProgressDialog loading;
    public String signal;
    public int[] receivedIDs;
    public int[] eventIDs;
    public String[] startDate;
    public String[] endDate;
    public String[] societyName;
    public String[] eventsTiming;
    public JSONArray result;
    public String[] imageTemp;
    public int numSocs;
    public ActionBar actionBar;
    private ListView listView;
    private DrawerLayout drawerLayout;
    public Toolbar toolbar;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.ents_to_frags);
        //SocsActivity.SocsActivity.finish();

        //toolbar = (Toolbar) findViewById(R.id.toolbar);
        //Toolbar will now take on default Action Bar characteristics
        //setActionBar(toolbar);
        //You can now use and reference the ActionBar


        if (getIntent().getBooleanExtra("biPassedSocsActivity", false)) {

            Intent intent = new Intent(EntsActivity.this, SplashActivity.class);
            final SharedPreferences appPrefs = this.getSharedPreferences("appPrefs", 0);
            final SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
            appPrefsEditor.putBoolean("allSocsFlag", false).commit();
            startActivity(intent);
        }


        drawerLayout = (DrawerLayout) findViewById(R.id.drawerlayout);
        String[] socs = getResources().getStringArray(R.array.societies);
        listView = (ListView) findViewById(R.id.drawer_list);
        listView.setAdapter(new ArrayAdapter<String>(this,
                                    android.R.layout.simple_list_item_1,
                                    socs) {

                                @Override
                                public View getView(int position, View convertView, ViewGroup parent) {
                                    TextView view = (TextView) super.getView(position, convertView, parent);

                                    //if(position == 0)
                                    //view.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_person_pin_black_24dp,0,0,0);
                                    //if(position == 1)
                                    //view.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_supervisor_account_black_24dp,0,0,0);
                                    if (position == 0)
                                        view.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_find_in_page_black_24dp, 0, 0, 0);
                                    if (position == 1)
                                        view.setCompoundDrawablesWithIntrinsicBounds(R.drawable.ic_settings_black_24dp, 0, 0, 0);
                                    view.setCompoundDrawablePadding(30);

                                    return view;
                                }

                            }
        );
        listView.setOnItemClickListener(this);

        actionBar = getSupportActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);


        mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                actionBar.setSelectedNavigationItem(position);
            }
        });

        Button allButton = (Button) findViewById(R.id.AllButton);
        Button myButton = (Button) findViewById(R.id.MyButton);

        allButton.setOnClickListener(
                new View.OnClickListener() {
                    public void onClick(View v) {
                        SharedPreferences appPrefs = EntsActivity.this.getSharedPreferences("appPrefs", 0);
                        SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
                        appPrefsEditor.putBoolean("allSocsFlag", true).commit();
                        Intent intent = new Intent(EntsActivity.this, EntsActivity.class);
                        intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                        startActivity(intent);
                        overridePendingTransition(0, 0); //0 for no animation
                        finish();

                    }
                }
        );

        myButton.setOnClickListener(
                new View.OnClickListener() {
                    public void onClick(View v) {
                        SharedPreferences appPrefs = EntsActivity.this.getSharedPreferences("appPrefs", 0);
                        SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
                        appPrefsEditor.putBoolean("allSocsFlag", false).commit();
                        Intent intent = new Intent(EntsActivity.this, EntsActivity.class);
                        intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                        startActivity(intent);
                        overridePendingTransition(0, 0); //0 for no animation
                        finish();

                    }
                }
        );


        final int tabPosition = getIntent().getIntExtra("tabPosition", 0);
        Log.v("tabPosition", "" + tabPosition);
        boolean[] setPos = new boolean[mSectionsPagerAdapter.getCount()];
        for (int i = 0; i < mSectionsPagerAdapter.getCount(); i++) {

            setPos[i] = i == tabPosition;

            actionBar.addTab(
                    actionBar.newTab().setText(mSectionsPagerAdapter.getPageTitle(i)).setTabListener(this), i, setPos[i]);
        }

        //actionBar.setIcon(R.drawable.app_icon);

        actionBar.setDisplayUseLogoEnabled(true);
        actionBar.setDisplayHomeAsUpEnabled(true);

        actionBar.setDisplayShowHomeEnabled(true);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
            actionBar.setHomeButtonEnabled(true);
        }
        getSupportActionBar().setHomeButtonEnabled(true);


        Boolean recursive = getIntent().getBooleanExtra("recursive", false);

        //        Boolean recursive = getIntent().getBooleanExtra("recursive", false);

        //    if (!recursive) {
        //        establishConnection(new EntsActivity.VolleyCallback() {
        //            @Override
        //            public void handleData(String response) throws JSONException {
//===============================================File handling===============================================================//
//-----------------------------------------------Common setup for reading or writing-----------------------------------------//
        String filename = "getEvents.php";      //what the getEvents.php will be saved as, can be anything

        String contents ="";
        File file;

        file= new File(getApplicationContext().getApplicationContext().getFilesDir(),filename); //This points to the file we will use. It's in the apps directory, should be accessible to all activities. May not exist, so could do with adding code to handle thee file not existing for loading from it.
//----------------------------------------------------------------------------------------------------------------------------//
//-----------------------------------------------Writing to the file----------------------------------------------------------//
/*                    try {
                        FileOutputStream outputStream = new FileOutputStream(file);
                        outputStream.write(response.getBytes()); //write the response string to the file
                        outputStream.close();
                    } catch (Exception e) {
                        //put stuff here for if saving fails
                        e.printStackTrace();
                    }
*/
//---------------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------Reading from the file-----------------------------------------------------//
        try{
            int length = (int) file.length();
            byte[] bytes = new byte[length];
            FileInputStream in = new FileInputStream(file);
            try {
                in.read(bytes);
            } finally {
                in.close();
            }
            contents = new String(bytes);//contents is the same as "response" which we got from the server
        } catch (Exception e) {
            //could put stuff here for what to do if loading fails (set a bool loadingFailed=true and display a toast or something)
            e.printStackTrace();
        }
//--------------------------------------------------------------------------------------------------------------------------//
//==========================================================================================================================//
        JSONObject entsData;

        try {

            JSONObject jsonObject = new JSONObject(contents);//using contents (from the file) instead of response (from the internet)
            result = jsonObject.getJSONArray(Constants.JSON_ARRAY);
            len = result.length();

            societyName = new String[len];
            eventsTiming = new String[len];
            String[] name = new String[len];
            endDate = new String[len];
            startDate = new String[len];

            imageTemp = new String[len];
            receivedIDs = new int[len];
            bm = new Bitmap[len];
            eventName = new String[len];
            for (int i = 0; i < len; i++) {

                entsData = result.getJSONObject(i);
                receivedIDs[i] = entsData.getInt(Constants.KEY_ID);
                name[i] = entsData.getString(Constants.KEY_EVENTNAME);
                imageTemp[i] = entsData.getString(Constants.KEY_LOWRES);
                societyName[i] = entsData.getString(Constants.KEY_SOCIETYNAME);
                startDate[i] = entsData.getString(Constants.KEY_STARTDATE);
                endDate[i] = entsData.getString(Constants.KEY_ENDDATE);

                String endTime = endDate[i].split(" ")[1];

                startDate[i] = startDate[i].substring(0, Math.min(startDate[i].length(), 16));
                endTime = endTime.substring(0, Math.min(endTime.length(), 5));
                eventName[i] = name[i];
                eventsTiming[i] = startDate[i] + "-" + endTime;
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }


        boolean[] idsActive;

        SharedPreferences appPrefs = EntsActivity.this.getSharedPreferences("appPrefs", 0);
        SharedPreferences.Editor appPrefsEditor = appPrefs.edit();

        boolean allSocsFlag = appPrefs.getBoolean("allSocsFlag", false);

        if (allSocsFlag) {

            int length = appPrefs.getInt("numSocs", 0);
            idsActive = new boolean[length];
            for (int i = 0; i < length; i++) {
                idsActive[i] = true;
            }
            appPrefsEditor.putBoolean("allSocsFlag", false).commit();

        } else {
            idsActive = loadArray1(EntsActivity.this);
            //
        }

        String temp;
        temp = appPrefs.getString("societyName", "NULL");
        String[] SelectedSociety = temp.split(",");

        numSocs = appPrefs.getInt("numSocs", 0);

        int counter = 0;
        int[] imgPos = new int[numSocs];
        eventIDs = new int[len];
        for (int i = 0; i < numSocs; i++) {

            if (idsActive[i]) {
                imgPos[counter] = i;
                counter++;
            }
        }

        int i;
        StringBuilder stringBuilder = new StringBuilder();
        String[] stringIDs = new String[len];

        int count = 0;
        for (i = 0; i < len; i++) {
            for (int j = 0; j < counter; j++) {
                int pos = imgPos[j];
                if (societyName[i].equals(SelectedSociety[pos])) {
                    eventIDs[count] = receivedIDs[i];
                    stringIDs[count] = Integer.toString(eventIDs[count]);
                    appPrefsEditor.putString("societyName" + eventIDs[count], societyName[i]);
                    appPrefsEditor.putString("eventName" + eventIDs[count], eventName[i]);
                    appPrefsEditor.putString("imageTemp" + eventIDs[count], imageTemp[i]);
                    appPrefsEditor.putString("eventIDsAndTimes" + eventIDs[count], eventsTiming[i]).commit();
                    stringBuilder.append(stringIDs[count]).append(",");
                    count++;
                }
            }

        }

        appPrefsEditor.putString("IDs", stringBuilder.toString());
        appPrefsEditor.putInt("count", count);
        appPrefsEditor.commit();

        mViewPager.setAdapter(mSectionsPagerAdapter);
        mViewPager.setCurrentItem(tabPosition);

    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        //noinspection SimplifiableIfStatement
        final DrawerLayout drawer = (DrawerLayout) EntsActivity.this.findViewById(R.id.drawerlayout);
        if (id == android.R.id.home) {
            // Toast.makeText(this, "Clicked", Toast.LENGTH_LONG).show();

            if (drawerLayout.isDrawerOpen(GravityCompat.START)) {
                drawer.closeDrawer(listView);
            } else
                drawer.openDrawer(listView);
            return true;
        }


        final SharedPreferences appPrefs = this.getSharedPreferences("appPrefs", 0);
        final SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
        switch (item.getItemId()){
            case R.id.action_refresh:
                int position;
                position = appPrefs.getInt("position", 1);
                if (position == 0)
                    appPrefsEditor.putBoolean("allSocsFlag", false).commit();

                if (position == 1)
                    appPrefsEditor.putBoolean("allSocsFlag", true).commit();

                Intent intent = new Intent(this, EntsActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                intent.putExtra("fromFrag", true);
                //  intent.putExtra("recursive", true);

                intent.putExtra("tabPosition", mViewPager.getCurrentItem());

                startActivity(intent);
                this.overridePendingTransition(R.anim.fadein,R.anim.fadeout);
                this.finish();
                return true;

        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onTabSelected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
        mViewPager.setCurrentItem(tab.getPosition());
    }

    @Override
    public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }

    @Override
    public void onTabReselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        //Toast.makeText(EntsActivity.this, "hello", Toast.LENGTH_LONG).show();
        SharedPreferences appPrefs = EntsActivity.this.getSharedPreferences("appPrefs", 0);
        SharedPreferences.Editor appPrefsEditor = appPrefs.edit();

        appPrefsEditor.putInt("position", position).commit();

        if (position == 0) {

            appPrefsEditor.putBoolean("fromEntsActivity", true).commit();

            Intent intent = new Intent(EntsActivity.this, ParticularSocActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            startActivity(intent);
            finish();

        } else if (position == 1) {

            appPrefsEditor.putBoolean("fromEntsActivity", true).commit();

            Intent intent = new Intent(EntsActivity.this, SocsActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            startActivity(intent);
            finish();

        }
    }

    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
     * one of the sections/tabs/pages.
     */
    public class SectionsPagerAdapter extends FragmentPagerAdapter {

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int position) {


            switch (position) {
                case 0:
                    return TodayFrag.newInstance("1");
                case 1:
                    return TomorrowFrag.newInstance("2");
                case 2:
                    return LaterFrag.newInstance("3");
                default:
                    return TodayFrag.newInstance(Integer.toString(position + 1));
            }

        }

        @Override
        public int getCount() {
            // Show 3 total pages.
            return 3;
        }

        @Override
        public CharSequence getPageTitle(int position) {
            Locale l = Locale.getDefault();
            switch (position) {
                case 0:
                    return getString(R.string.title_section1).toUpperCase(l);
                case 1:
                    return getString(R.string.title_section2).toUpperCase(l);
                case 2:
                    return getString(R.string.title_section3).toUpperCase(l);
            }
            return null;
        }
    }

    /**
     * A placeholder fragment containing a simple view.
     */

    private String establishConnection(final VolleyCallback callback) {

        final boolean fromFrag = getIntent().getBooleanExtra("fromFrag", false);

        if (!fromFrag) {
            loading = ProgressDialog.show(this, "Please wait...", "Fetching data...", false, false);
        }
        String url = Constants.EVENTS_URL;

        StringRequest stringRequest = new StringRequest(url, new Response.Listener<String>() {
            @Override
            public void onResponse(String response) throws JSONException {
                if (!fromFrag) {
                    loading.dismiss();
                }
                callback.handleData(response);
            }
        },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        //Toast.makeText(EntsActivity.this, error.getMessage(), Toast.LENGTH_LONG).show();
                        Toast.makeText(EntsActivity.this, "Please turn on wifi", Toast.LENGTH_LONG).show();
                    }
                });

        RequestQueue requestQueue = Volley.newRequestQueue(this);
        requestQueue.add(stringRequest);

        return signal;
    }

    public interface VolleyCallback {
        void handleData(String response) throws JSONException;
    }

    public boolean[] loadArray(Context mContext) {
        SharedPreferences appPrefs = mContext.getSharedPreferences("appPrefs", 0);

        int size = appPrefs.getInt("idsActive" + "_size", 0);
        boolean array[] = new boolean[size];
        final int chosenSociety = appPrefs.getInt("chosenSociety", -1);

        if(chosenSociety >= 0)  //Was this activity started from ParticularSocActivity?
        {
            /*
            If this activity was started by ParticularSocActivity, chosenSociety will be a positive number,
            corresponding to the list position of the society clicked on by the user
            */

            SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
            appPrefsEditor.putInt("chosenSociety", -1); //Set this flag back to -1, to prevent the particularSoc selection from being permanent
            appPrefsEditor.commit();

            //Set all societies to have their events hidden, except the one clicked on by the user in ParticularSocActivity
            for (int i = 0; i < size; i++)
                array[i] = false;

            array[chosenSociety] = true;
        }
        else //Did not come from ParticularSocActivity - set society events to appear as normal
        {
            for (int i = 0; i < size; i++)
                array[i] = appPrefs.getBoolean("idsActive" + "_" + i, false);
        }

        return array;   //Return the array of societies whose events are to be displayed
    }

    public boolean[] loadArray1(Context mContext) {
        SharedPreferences appPrefs = mContext.getSharedPreferences("appPrefs", 0);

        int size = appPrefs.getInt("idsActive" + "_size", 0);
        boolean array[] = new boolean[size];
        final int chosenSociety = appPrefs.getInt("chosenSociety", -1);

        if (chosenSociety >= 0)  //Was this activity started from ParticularSocActivity?
        {
            /*
            If this activity was started by ParticularSocActivity, chosenSociety will be a positive number,
            corresponding to the list position of the society clicked on by the user
            */

            SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
            appPrefsEditor.putInt("chosenSociety", -1); //Set this flag back to -1, to prevent the particularSoc selection from being permanent
            appPrefsEditor.commit();

            //Set all societies to have their events hidden, except the one clicked on by the user in ParticularSocActivity
            for (int i = 0; i < size; i++)
                array[i] = false;

            array[chosenSociety] = true;
        } else //Did not come from ParticularSocActivity - set society events to appear as normal
        {
            //String subbos = appPrefs.getString("subbys", "NULL");

            getSubs();
            String subbos = appPrefs.getString("subbys", "NULL");

            int len = subbos.length();
            char testChar;

            for (int i = 0; i < len; i++) {
                testChar = subbos.charAt(i);
                if (testChar == '1') {
                    array[i] = true;
                } else array[i] = false;
            }
        }
            return array;   //Return the array of societies whose events are to be displayed


    }



    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater menuInflater = getMenuInflater();
        menuInflater.inflate(R.menu.activity_main_actions, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public void onBackPressed() {
        if(this.drawerLayout.isDrawerOpen(GravityCompat.START)) {
            this.drawerLayout.closeDrawer(GravityCompat.START);
        }
        else {
            super.onBackPressed();
        }
    }

    private void getSubs() {
        //email = etEmail.getText().toString().trim();
        //password = etPassword.getText().toString().trim();
        SharedPreferences appPrefs = EntsActivity.this.getSharedPreferences("appPrefs", 0);

        //final String email = appPrefs.getString("loggedInUser", "NULL");
        final String email = "hello9";
        final String password = "pass9";

        StringRequest stringRequest = new StringRequest(Request.Method.GET, Constants.GETSUBS_URL,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        if(response.trim().equals("Failure")){
                            //Toast.makeText(EntsActivity.this,response,Toast.LENGTH_LONG).show();


                        }else{
                            //Toast.makeText(EntsActivity.this,response,Toast.LENGTH_LONG).show();
                            /*
                            SharedPreferences appPrefs1 = EntsActivity.this.getSharedPreferences("appPrefs", 0);
                            final SharedPreferences.Editor appPrefsEditor1 = appPrefs1.edit();
                            String subscriptions = response;
                            appPrefsEditor1.putString("subbys", subscriptions).commit(); */

                        }
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Toast.makeText(EntsActivity.this,error.toString(),Toast.LENGTH_LONG ).show();
                    }
                }){
            @Override
            protected Map<String,String> getParams(){
                Map<String,String> params = new HashMap<String, String>();
                params.put(Constants.KEY_EMAIL, email);
                params.put(Constants.KEY_PASSWORD, password);
                return params;
            }
        };

        RequestQueue requestQueue = Volley.newRequestQueue(this);
        requestQueue.add(stringRequest);

    }


}

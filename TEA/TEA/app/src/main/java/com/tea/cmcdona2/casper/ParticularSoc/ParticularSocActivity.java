/*
Displays a list of all societies to the user. If the user clicks on one, they are brought to
EntsActivity, which will display only the events corresponding to that society.
 */

package com.tea.cmcdona2.casper.ParticularSoc;

import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.tea.cmcdona2.casper.Ents.EntsActivity;
import com.tea.cmcdona2.casper.Other.Constants;
import com.tea.cmcdona2.casper.R;
import com.tea.cmcdona2.casper.Socs.SocsActivity;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;


public class ParticularSocActivity extends ActionBarActivity implements AdapterView.OnItemClickListener {

    public int len;
    public String[] str;
    public Bitmap[] bm;
    public ActionBar actionBar;
    private ListView listView;
    private ListView societies_listView;
    private DrawerLayout drawerLayout;
    public ProgressDialog loading;
    public String signal;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.particular_soc_activity);

        //Pull information from database, then generate ListView of societies
//            establishConnection(new VolleyCallback() {
//                @Override
//                public void handleData(String response) {
//===============================================File handling===============================================================//
//-----------------------------------------------Common setup for reading or writing-----------------------------------------//
        String filename = "getInfo.php";      //what the getEvents.php will be saved as, can be anything

        String contents ="";
        File file;

        file= new File(getApplicationContext().getApplicationContext().getFilesDir(),filename); //This points to the file we will use. It's in the apps directory, should be accessible to all activities. May not exist, so could do with adding code to handle thee file not existing for loading from it.
//----------------------------------------------------------------------------------------------------------------------------//
//-----------------------------------------------Writing to the file----------------------------------------------------------//
/*
                    try {
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
                                    JSONObject socData;
                                    byte[] data;
                                    Bitmap bitmap;


                                    try {

                                        JSONObject jsonObject = new JSONObject(contents);
                                        JSONArray result = jsonObject.getJSONArray(Constants.JSON_ARRAY);
                                        len = result.length();

                                        String[] name = new String[len];
                                        String[] imageTemp = new String[len];
                                        bm = new Bitmap[len];
                                        str = new String[len];

                                        for (int i = 0; i < len; i++) {

                                            socData = result.getJSONObject(i);
                                            name[i] = socData.getString(Constants.KEY_NAME);
                                            imageTemp[i] = socData.getString(Constants.KEY_IMAGE);
                                            data = Base64.decode(imageTemp[i], Base64.DEFAULT);
                                            bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
                                            bm[i] = bitmap;
                                            str[i] = name[i];
                                        }
                                    } catch (JSONException e) {
                                        e.printStackTrace();
                                    }

                                    //Generate the ListView of societies to be displayed
                                    ArrayList<ParticularSocItem> societyItems = new ArrayList<ParticularSocItem>();
                                    ParticularSocAdapter particularSocAdapter = new ParticularSocAdapter(getApplicationContext(), getSocItems());
                                    societies_listView = (ListView) findViewById(R.id.societies_list);
                                    societies_listView.setAdapter(particularSocAdapter);
                                    societies_listView.setOnItemClickListener(new SocietyList_ClickHandler());

        //Generate the navigation drawer
        drawerLayout = (DrawerLayout) findViewById(R.id.drawerlayout);
        String[] socs = getResources().getStringArray(R.array.societies);
        listView = (ListView) findViewById(R.id.drawer_list);
        listView.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, socs));
        listView.setOnItemClickListener(this);

        //Generate the action bar
        actionBar = getSupportActionBar();
        actionBar.setDisplayUseLogoEnabled(true);
        actionBar.setDisplayShowHomeEnabled(true);
        actionBar.setDisplayHomeAsUpEnabled(true);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
            actionBar.setHomeButtonEnabled(true);
        }
        getSupportActionBar().setHomeButtonEnabled(true);
    }

    //Item click handler for the societies list
    public class SocietyList_ClickHandler implements AdapterView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView parent, View view,
                                int position, long id) {

            //Go to EntsActivity - save data to cache
            SharedPreferences appPrefs = ParticularSocActivity.this.getSharedPreferences("appPrefs", 0);
            SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
            appPrefsEditor.putBoolean("previouslyLaunched", true).commit();
            appPrefsEditor.putBoolean("allSocsFlag", false).commit();
            appPrefsEditor.putInt("chosenSociety", position).commit();  //Tell EntsActivity which society in the list was clicked on

            //Pass intent
            Intent intent = new Intent(ParticularSocActivity.this, EntsActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            startActivity(intent);
            overridePendingTransition(0, 0); //0 for no animation
            finish();

        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        //noinspection SimplifiableIfStatement
        final DrawerLayout drawer = (DrawerLayout) ParticularSocActivity.this.findViewById(R.id.drawerlayout);
        if (id == android.R.id.home) {

            if (drawerLayout.isDrawerOpen(GravityCompat.START)) {
                drawer.closeDrawer(listView);
            } else
                drawer.openDrawer(listView);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    //OnItemClick listener for the navigation drawer
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        //Toast.makeText(ParticularSocActivity.this, "hello", Toast.LENGTH_LONG).show();
        SharedPreferences appPrefs = ParticularSocActivity.this.getSharedPreferences("appPrefs", 0);
        SharedPreferences.Editor appPrefsEditor = appPrefs.edit();

        appPrefsEditor.putInt("position", position).commit();

        if (position == 0) { //My Societies
            Log.v("positionOne", "" + position);
            appPrefsEditor.putBoolean("allSocsFlag", false).commit();
            Intent intent = new Intent(ParticularSocActivity.this, EntsActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            Log.v("positionTwo", "" + position);
            startActivity(intent);
            Log.v("positionThree", "" + position);
            overridePendingTransition(0, 0); //0 for no animation
            finish();
        } else if (position == 1) {    //All Societies
            appPrefsEditor.putBoolean("allSocsFlag", true).commit();
            Intent intent = new Intent(ParticularSocActivity.this, EntsActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            startActivity(intent);
            overridePendingTransition(0, 0); //0 for no animation
            finish();
        } else if (position == 2) { //ParticularSoc

            appPrefsEditor.putBoolean("fromEntsActivity", true).commit();

            Intent intent = new Intent(ParticularSocActivity.this, ParticularSocActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            startActivity(intent);
            finish();
        } else if (position == 3) { //Edit

            appPrefsEditor.putBoolean("fromEntsActivity", true).commit();

            Intent intent = new Intent(ParticularSocActivity.this, SocsActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
            startActivity(intent);
            finish();
        }
    }

    //Pull information from database
    private String establishConnection(final VolleyCallback callback) {

        loading = ProgressDialog.show(this, "Please wait...", "Fetching data...", false, false);

        String url = Constants.DATA_URL;

        StringRequest stringRequest;
        stringRequest = new StringRequest(url, new Response.Listener<String>() {
            @Override
            public void onResponse(String response) {
                loading.dismiss();
                callback.handleData(response);
            }
        },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        //Toast.makeText(ParticularSocActivity.this, error.getMessage().toString(), Toast.LENGTH_LONG).show();
                        Toast.makeText(ParticularSocActivity.this, "Please turn on wifi", Toast.LENGTH_LONG).show();
                    }
                });

        RequestQueue requestQueue = Volley.newRequestQueue(this);
        requestQueue.add(stringRequest);

        return signal;
    }

    public interface VolleyCallback {
        void handleData(String response);
    }

    //Add society items (consisting of an image and a string) to the imageItems array, used to generate the ListView of societies
    private ArrayList<ParticularSocItem> getSocItems() {

        final ArrayList<ParticularSocItem> imageItems = new ArrayList<>();

        for (int i = 0; i < len; i++) {
            imageItems.add(new ParticularSocItem(bm[i], str[i]));
        }

        return imageItems;
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

}


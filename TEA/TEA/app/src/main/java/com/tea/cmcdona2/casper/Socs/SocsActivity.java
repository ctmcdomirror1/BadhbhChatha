package com.tea.cmcdona2.casper.Socs;

import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.util.Base64;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.GridView;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.tea.cmcdona2.casper.LogReg.LogIn;
import com.tea.cmcdona2.casper.Other.Constants;
import com.tea.cmcdona2.casper.Ents.EntsActivity;
import com.tea.cmcdona2.casper.R;
import com.tea.cmcdona2.casper.Splash.SplashActivity;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class SocsActivity extends ActionBarActivity {

    public int len;
    public boolean[] idsActive;
    public String signal;
    public String[] str;
    public Bitmap[] bm;
    public static Activity SocsActivity;
    public GridView gridView;
    public SocsAdapter gridAdapter;
    public ProgressDialog loading;
    public Boolean previouslyLaunched;
    public Boolean alreadyRegistered;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        //caching for previosulyLaunched and fromEntsActivity

        SharedPreferences appPrefs = SocsActivity.this.getSharedPreferences("appPrefs", 0);
        final SharedPreferences.Editor appPrefsEditor = appPrefs.edit();

        final Boolean fromEntsActivity = appPrefs.getBoolean("fromEntsActivity", false);

        setContentView(R.layout.socs_activity);
        SocsActivity = this; //remember this activity to be finished in next activity

        // actionBar.setIcon(R.drawable.app_icon);
        ActionBar actionBar;
        actionBar = getSupportActionBar();
        actionBar.setDisplayUseLogoEnabled(true);
        actionBar.setDisplayShowHomeEnabled(true);

        previouslyLaunched = appPrefs.getBoolean("previouslyLaunched", false);
        alreadyRegistered = appPrefs.getBoolean("alreadyRegistered", false);

        if (fromEntsActivity || !previouslyLaunched) {

            if (!fromEntsActivity) {

                //Intent intent = new Intent(SocsActivity.this, SplashActivity.class);
                appPrefsEditor.putBoolean("allSocsFlag", false).commit();
                //startActivity(intent);

                //if(!alreadyRegistered) {
                   // Intent loginIntent = new Intent(this, LogIn.class);
                    //startActivity(loginIntent);
                //}

            }

            //else fromEntsActivity so no need for splash, then run activity


            //after establishing a connection and receiving data, do the following

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
//--------------------------------------------------------------------------------------------------------------------------//
//==========================================================================================================================//
            //code from here is the regular stuff loading the JSONObject from contents instread of from response
            JSONObject socData;
            byte[] data;
            Bitmap bitmap;

            try {

                JSONObject jsonObject = new JSONObject(contents);//using contents (from the file) instead of response (from the internet)
                JSONArray result = jsonObject.getJSONArray(Constants.JSON_ARRAY);
                len = result.length();

                String[] name = new String[len];
                String[] imageTemp = new String[len];
                bm = new Bitmap[len];
                str = new String[len];

                appPrefsEditor.putInt("idsActive_size", len).commit();

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

                    idsActive = new boolean[len];

                    if (!fromEntsActivity) {
                        for (int i = 0; i < len; i++) {
                            idsActive[i] = false;
                        }
                    }

                    else{
                        idsActive = loadArray("idsActive", SocsActivity.this);
                    }

                    gridView = (GridView) findViewById(R.id.gridView);
                    gridAdapter = new SocsAdapter(SocsActivity.this, R.layout.soc_item, getSocItems());
                    gridView.setAdapter(gridAdapter);

                    gridView.setOnItemClickListener(new OnItemClickListener() {
                        public void onItemClick(AdapterView<?> parent, View v, int position, long id) {

                            Integer i = (int) (long) id;

                            //Toast.makeText(SocsActivity.this, ""+position, Toast.LENGTH_LONG).show();

                            if (v.isActivated()) {
                                v.setActivated(false);
                                idsActive[i] = false;
                            } else {
                                v.setActivated(true);
                                idsActive[i] = true;
                            }

                            storeArray(idsActive, "idsActive", SocsActivity.this);
                        }
                    });



            appPrefsEditor.putBoolean("fromEntsActivity", false);
            appPrefsEditor.commit();
        } else {
            //splash and bipass
            Intent intent = new Intent(SocsActivity.this, EntsActivity.class);
            intent.putExtra("biPassedSocsActivity", true);
            appPrefsEditor.putBoolean("allSocsFlag", false).commit();
            startActivity(intent);
        }

    }

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
                        //Toast.makeText(SocsActivity.this, error.getMessage().toString(), Toast.LENGTH_LONG).show();
                        Toast.makeText(SocsActivity.this, "Please turn on wifi", Toast.LENGTH_LONG).show();
                    }
                });

        RequestQueue requestQueue = Volley.newRequestQueue(this);
        requestQueue.add(stringRequest);

        return signal;
    }

    public interface VolleyCallback {
        void handleData(String response);
    }

    //used for creating the checkbox

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.clear();
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.checkbox, menu);
        return true;
    }

    public void tickOnClick(MenuItem item) {

        Boolean atLeastOneHasBeenSelected = false;

        for(int i = 0; i <len; i++){
            if(idsActive[i]){
                atLeastOneHasBeenSelected = true;
                break;
            }
        }

        if(atLeastOneHasBeenSelected){
            tickOnClickCallback();
        }
        else{
            Toast.makeText(SocsActivity.this, "Please select a society", Toast.LENGTH_LONG).show();
        }
    }

    public void tickOnClickCallback() {

        String subs = booleanArrayToString(idsActive);

        StringBuilder sb1 = new StringBuilder();
        sb1.append("");



        sb1.append(subs);
        String subbys = sb1.toString();



        SharedPreferences appPrefs1 = SocsActivity.this.getSharedPreferences("appPrefs", 0);
        final SharedPreferences.Editor appPrefsEditor1 = appPrefs1.edit();
        String subscriptions = subbys;
        appPrefsEditor1.putString("subbys", subscriptions).commit();

        sendSubs(subscriptions);



        Intent intent = new Intent(SocsActivity.this, EntsActivity.class);

        SharedPreferences appPrefs = SocsActivity.this.getSharedPreferences("appPrefs", 0);
        SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
        appPrefsEditor.putBoolean("previouslyLaunched", true);


        appPrefsEditor.putInt("numSocs", len);

        StringBuilder sb = new StringBuilder();

        for (int i = 0; i < len; i++) {
            sb.append(str[i]).append(",");
        }

        appPrefsEditor.putString("societyName", sb.toString());
        appPrefsEditor.commit();

        startActivity(intent);
        finish();
    }

    //adding SocItems, which are an image (bitmap) and a string, to an array

    private ArrayList<SocItem> getSocItems() {

        final ArrayList<SocItem> imageItems = new ArrayList<>();

        for (int i = 0; i < len; i++) {
            imageItems.add(new SocItem(bm[i], str[i]));
        }

        return imageItems;
    }

    //functions for storing and loading a boolean array

    public boolean storeArray(boolean[] array, String arrayName, Context mContext) {
        SharedPreferences prefs = mContext.getSharedPreferences("appPrefs", 0);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putInt(arrayName + "_size", array.length);
        for (int i = 0; i < array.length; i++)
            editor.putBoolean(arrayName + "_" + i, array[i]);
        return editor.commit();
    }

    //test comment

    public boolean[] loadArray(String arrayName, Context mContext) {

        SharedPreferences appPrefs = mContext.getSharedPreferences("appPrefs", 0);
        int size = appPrefs.getInt(arrayName + "_size", 0);
        boolean array[] = new boolean[size];
        for (int i = 0; i < size; i++)
            array[i] = appPrefs.getBoolean(arrayName + "_" + i, false);
        return array;
    }

    public String booleanArrayToString (boolean array[]){
        String n;
        len = array.length;

        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("");

        for (int i = 0; i < len; i++) {
            if(array[i]) {
                stringBuilder.append("1");
            }
            else stringBuilder.append("0");
        }

        n = stringBuilder.toString();
        return n;
    }

    private void sendSubs(String subos) {
        //email = etEmail.getText().toString().trim();
        //password = etPassword.getText().toString().trim();
        SharedPreferences appPrefs = SocsActivity.this.getSharedPreferences("appPrefs", 0);
        final SharedPreferences.Editor appPrefsEditor = appPrefs.edit();

        final String email = appPrefs.getString("loggedInUser", "NULL");
        final String subs = subos;
        //final String email = "tester";
        //final String subs = "tester";

        StringRequest stringRequest = new StringRequest(Request.Method.POST, Constants.SUBS_URL,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        if(response.trim().equals("success")){
                            //Toast.makeText(SocsActivity.this,response,Toast.LENGTH_LONG).show();

                        }else{
                            Toast.makeText(SocsActivity.this,response,Toast.LENGTH_LONG).show();
                        }
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Toast.makeText(SocsActivity.this,error.toString(),Toast.LENGTH_LONG ).show();
                    }
                }){
            @Override
            protected Map<String,String> getParams(){
                Map<String,String> params = new HashMap<String, String>();
                params.put(Constants.KEY_EMAIL, email);
                params.put(Constants.KEY_SUBS, subs);
                return params;
            }
        };

        RequestQueue requestQueue = Volley.newRequestQueue(this);
        requestQueue.add(stringRequest);
    }


}


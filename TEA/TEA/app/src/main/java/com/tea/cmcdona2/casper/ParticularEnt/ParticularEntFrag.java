package com.tea.cmcdona2.casper.ParticularEnt;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.ColorStateList;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.shapes.Shape;
import android.media.Image;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.CalendarContract;
import android.support.annotation.Nullable;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.app.Fragment;
import android.support.v7.graphics.Palette;
import android.util.AttributeSet;
import android.util.Base64;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewAnimationUtils;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.RotateAnimation;
import android.view.inputmethod.InputMethodManager;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.support.v7.widget.Toolbar;

import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.tea.cmcdona2.casper.Other.Constants;
import com.tea.cmcdona2.casper.R;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.w3c.dom.Text;

import java.io.File;
import java.io.FileInputStream;
import java.util.GregorianCalendar;


public class ParticularEntFrag extends Fragment {

    public Bitmap bm;
    public ProgressDialog loading;
    public String signal;
    public String EventDescription;
    public String EventName;
    public String Location;
    public String StartDate;
    public String EndDate;
    public int StartYear;
    public int StartMonth;
    public int StartDay;
    public int StartHour;
    public int StartMinute;
    public int EndYear;
    public int EndMonth;
    public int EndDay;
    public int EndHour;
    public int EndMinute;
    public String societyName;
    public View view;
    public int eventId;
    public Boolean buttonVisible = true;
    public LinearLayout buttonHolder;

    public FloatingActionButton imgBtn;

    private static TextView eventName;

    private static TextView eventDescription;

    private static ImageView imgview;

    public void hideEditText(final LinearLayout view) {
        int cx = view.getRight() - 30;
        int cy = view.getBottom() - 60;
        int initialRadius = view.getWidth();
        Animator anim = ViewAnimationUtils.createCircularReveal(view, cx, cy, initialRadius, 0);
        anim.addListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                view.setVisibility(View.INVISIBLE);
            }
        });
        buttonVisible = false;
        anim.start();
    }

    public void revealEditText(final LinearLayout view) {
        int cx = view.getRight() - 30;
        int cy = view.getBottom() - 60;
        int finalRadius = Math.max(view.getWidth(), view.getHeight());
        Animator anim = ViewAnimationUtils.createCircularReveal(view, cx, cy, 0, finalRadius);
        view.setVisibility(View.VISIBLE);
        buttonVisible = true;
        anim.start();
    }


    @Nullable

    @Override

    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,

                             @Nullable Bundle savedInstanceState) {


        view = inflater.inflate(R.layout.particular_ent_activity_fragment, container, false);

        buttonHolder = (LinearLayout) view.findViewById(R.id.buttonHolder);

        buttonHolder.setVisibility(View.INVISIBLE);
        buttonVisible = false;

        Bundle bundle = this.getArguments();
        if (bundle != null) {
            eventId = bundle.getInt("eventId", 0);
        }

        int pos = bundle.getInt("eventPosition",0);
        SharedPreferences appPrefs = ParticularEntFrag.this.getContext().getSharedPreferences("appPrefs", 0);
        SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
        appPrefsEditor.putInt("ID", eventId).apply();
        //        establishConnection(new ParticularEntActivity.VolleyCallback() {
//            @Override
//            public void handleData(String response) {
//===============================================File handling===============================================================//
//-----------------------------------------------Common setup for reading or writing-----------------------------------------//
        String filename = "getEvents.php";      //what the getEvents.php will be saved as, can be anything

        String contents ="";
        File file;

        file= new File(getContext().getApplicationContext().getFilesDir(),filename); //This points to the file we will use. It's in the apps directory, should be accessible to all activities. May not exist, so could do with adding code to handle thee file not existing for loading from it.
//----------------------------------------------------------------------------------------------------------------------------//
//-----------------------------------------------Writing to the file----------------------------------------------------------//
/*                try {
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
        JSONObject particularEntData;
        byte[] data;
        Bitmap bitmap;

        try {

            JSONObject jsonObject = new JSONObject(contents);//using contents (from the file) instead of response (from the internet)
            JSONArray result = jsonObject.getJSONArray(Constants.JSON_ARRAY);
            String imageTemp;

            particularEntData = result.getJSONObject(pos);
            imageTemp = particularEntData.getString(Constants.KEY_LOWRES);
            EventName = particularEntData.getString(Constants.KEY_EVENTNAME);
            EventDescription = particularEntData.getString(Constants.KEY_EVENTDESCRIPTION);
            StartDate = particularEntData.getString(Constants.KEY_STARTDATE);
            EndDate = particularEntData.getString(Constants.KEY_ENDDATE);
            Location = particularEntData.getString(Constants.KEY_LOCATION);
            societyName = particularEntData.getString(Constants.KEY_SOCIETYNAME);
            data = Base64.decode(imageTemp, Base64.DEFAULT);
            bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
            bm = bitmap;
            StartDay = Integer.parseInt(StartDate.substring(8, 10));
            StartMonth = Integer.parseInt(StartDate.substring(5, 7));
            StartYear = Integer.parseInt(StartDate.substring(0, 4));
            StartHour = Integer.parseInt(StartDate.substring(11, 13));
            StartMinute = Integer.parseInt(StartDate.substring(14, 16));
            EndDay = Integer.parseInt(EndDate.substring(8, 10));
            EndMonth = Integer.parseInt(EndDate.substring(5, 7));
            EndYear = Integer.parseInt(EndDate.substring(0, 4));
            EndHour = Integer.parseInt(EndDate.substring(11, 13));
            EndMinute = Integer.parseInt(EndDate.substring(14, 16));


        } catch (JSONException e) {
            e.printStackTrace();
        }

                ImageView imgview = (ImageView) view.findViewById(R.id.myImgView);
        final LinearLayout placenameHolder = (LinearLayout) view.findViewById(R.id.placeNameHolder);
        final TextView eventName = (TextView) view.findViewById(R.id.eventName);
        imgBtn = (FloatingActionButton) view.findViewById(R.id.btn_add);
        final FloatingActionButton getDirections = (FloatingActionButton) view.findViewById(R.id.getDirections);
        final FloatingActionButton fav = (FloatingActionButton) view.findViewById(R.id.fav);
        final FloatingActionButton going = (FloatingActionButton) view.findViewById(R.id.going);


        final TextView event_date = (TextView) view.findViewById(R.id.eventDate);
        final TextView event_time = (TextView) view.findViewById(R.id.eventTime);
        final TextView event_about = (TextView) view.findViewById(R.id.about);
        final TextView event_date_des = (TextView) view.findViewById(R.id.eventDateDescription);
        final TextView event_time_des = (TextView) view.findViewById(R.id.eventTimeDescription);

        imgview.setImageBitmap(bm);

        Palette.generateAsync(bm, new Palette.PaletteAsyncListener() {
            @Override
            public void onGenerated(Palette palette) {
                // Here's your generated palette
                int bgColor = palette.getLightVibrantColor(getContext().getResources().getColor(android.R.color.background_light));
                placenameHolder.setBackgroundColor(bgColor);
                int bgColor1 = palette.getDarkMutedColor(getContext().getResources().getColor(android.R.color.darker_gray));
                int bgColor2 = palette.getDarkVibrantColor(getContext().getResources().getColor(android.R.color.black));
                int bgColor3 = palette.getDarkMutedColor(getContext().getResources().getColor(android.R.color.black));
                //imgBtn.setBackgroundColor(bgColor1);
                imgBtn.setBackgroundTintList(new ColorStateList(new int[][]{new int[]{0}}, new int[]{bgColor1}));
                imgBtn.setRippleColor(bgColor);
                going.setBackgroundTintList(new ColorStateList(new int[][]{new int[]{0}}, new int[]{bgColor1}));
                getDirections.setBackgroundTintList(new ColorStateList(new int[][]{new int[]{0}}, new int[]{bgColor1}));
                fav.setBackgroundTintList(new ColorStateList(new int[][]{new int[]{0}}, new int[]{bgColor1}));
                eventName.setTextColor(bgColor3);
                event_date.setTextColor(bgColor2);
                event_time.setTextColor(bgColor2);
                event_about.setTextColor(bgColor2);

            }
        });

                TextView eventDescription = (TextView) view.findViewById(R.id.eventDescription);
                eventDescription.setText(EventDescription);
                eventName.setText(EventName);

        String sStartday = Integer.toString(StartDay);
        String sStartMonth = Integer.toString(StartMonth);
        String sStartYear = Integer.toString(StartYear);
        String date = sStartday + "/" + sStartMonth + "/" + sStartYear;
        event_date_des.setText(date);

        String sStartHour = Integer.toString(StartHour);
        String sStartMinute = StartDate.substring(14, 16);
        String time = sStartHour + ":" + sStartMinute;
        event_time_des.setText(time);



        going.setOnClickListener(
                new View.OnClickListener() {
                    public void onClick(View v) {
                        Calendar();
                    }
                }
        );
        getDirections.setOnClickListener(
                new View.OnClickListener() {
                    public void onClick(View v)
                   {
                        GMAP();
                    }
                }
        );

        imgBtn.setOnClickListener(
                new View.OnClickListener() {
                    public void onClick(View v) {
                        //Toast.makeText(getContext(), "hello", Toast.LENGTH_LONG).show();
                        if (!buttonVisible) {
                            revealEditText(buttonHolder);
                            rotate(45);
                            //mEditTextTodo.requestFocus();
                            //mInputManager.showSoftInput(mEditTextTodo, InputMethodManager.SHOW_IMPLICIT);

                        } else {
                            //mInputManager.hideSoftInputFromWindow(mEditTextTodo.getWindowToken(), 0);
                            hideEditText(buttonHolder);
                            rotate(90);

                        }
                    }
                }
        );





        return view;

    }

    public void GMAP() {

        Uri gmmIntentUri = Uri.parse("geo:0,0?q=" + Location);
        Intent mapIntent = new Intent(Intent.ACTION_VIEW, gmmIntentUri);
        mapIntent.setPackage("com.google.android.apps.maps");
        startActivity(mapIntent);


    }

    public void Calendar() {
        Intent calIntent = new Intent(Intent.ACTION_INSERT);
        calIntent.setType("vnd.android.cursor.item/event");
        calIntent.putExtra(CalendarContract.Events.TITLE, EventName + " by- " + societyName);
        calIntent.putExtra(CalendarContract.Events.EVENT_LOCATION, Location);
        calIntent.putExtra(CalendarContract.Events.DESCRIPTION, EventDescription);

        GregorianCalendar calstartDate = new GregorianCalendar(StartYear, StartMonth - 1, StartDay, StartHour, StartMinute);
        GregorianCalendar calendDate = new GregorianCalendar(EndYear, EndMonth - 1, EndDay, EndHour, EndMinute);
        calIntent.putExtra(CalendarContract.EXTRA_EVENT_BEGIN_TIME,
                calstartDate.getTimeInMillis());
        calIntent.putExtra(CalendarContract.EXTRA_EVENT_END_TIME,
                calendDate.getTimeInMillis());

        startActivity(calIntent);
    }




    private String establishConnection(final VolleyCallback callback) {

        SharedPreferences appPrefs = ParticularEntFrag.this.getContext().getSharedPreferences("appPrefs", 0);
        //SharedPreferences.Editor appPrefsEditor = appPrefs.edit();

        //loading = ProgressDialog.show(this.getContext(), "Please wait...", "Fetching data...", false, false);

        int eventID = appPrefs.getInt("ID",0);
        String urlExtension = Integer.toString(eventID);
        String url = Constants.HIRES_URL + urlExtension;

        StringRequest stringRequest;
        stringRequest = new StringRequest(url, new Response.Listener<String>() {
            @Override
            public void onResponse(String response) {
                //loading.dismiss();
                callback.handleData(response);
            }
        },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        //Toast.makeText(ParticularEntFrag, error.getMessage(), Toast.LENGTH_LONG).show();
                    }
                });

        RequestQueue requestQueue = Volley.newRequestQueue(this.getContext());
        requestQueue.add(stringRequest);

        return signal;
    }

    public interface VolleyCallback {
        void handleData(String response);
    }

    private void rotate(float degree) {
        final RotateAnimation rotateAnim = new RotateAnimation(0.0f, degree, RotateAnimation.RELATIVE_TO_SELF, 0.5f, RotateAnimation.RELATIVE_TO_SELF, 0.5f);
        rotateAnim.setDuration(100);
        rotateAnim.setFillAfter(true);
        imgBtn.startAnimation(rotateAnim);
    }
    
}
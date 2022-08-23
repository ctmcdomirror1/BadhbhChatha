package com.tea.cmcdona2.casper.Ents.Tabs;

import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Base64;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;

import com.tea.cmcdona2.casper.Ents.EntItem;
import com.tea.cmcdona2.casper.Ents.EntsAdapter;
import com.tea.cmcdona2.casper.ParticularEnt.ParticularEntActivity;
import com.tea.cmcdona2.casper.R;

import org.joda.time.DateTime;
import org.joda.time.Interval;
import org.joda.time.format.DateTimeFormat;
import org.joda.time.format.DateTimeFormatter;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Paddy on 03/12/2015.
 */

public class LaterFrag extends android.support.v4.app.Fragment {

    List Ent_Cards;

    public static LaterFrag newInstance(String text) {
        LaterFrag fragment = new LaterFrag();
        Bundle args = new Bundle();
        args.putString("message", text);
        return fragment;
    }

    public LaterFrag() {
        // Required empty public constructor
    }
    int counter1 = 0;

    String loadedID;
    String[] stringIDs;
    int numOfEventsPassed;
    String[] societyName;
    String[] eventName ;
    String[] imageTemp ;
    String[] eventTimes;
    String[] eventDisplayDates ;
    String[] eventDisplayTimes ;
    String[] startTimes ;
    String[] endTimes ;
    String[] splitEventIDsAndTimes;
    String eventIDsAndTimes;
    String additive;


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment

        View v = inflater.inflate(R.layout.card_ents_activity, container, false);

        RecyclerView rv = (RecyclerView)v.findViewById(R.id.rv);
        final SharedPreferences appPrefs = this.getActivity().getSharedPreferences("appPrefs", 0);
        final SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
         loadedID = appPrefs.getString("IDs", "null");
         stringIDs = loadedID.split(",");
         numOfEventsPassed = stringIDs.length;
        societyName = new String[numOfEventsPassed];
        eventName = new String[numOfEventsPassed];
         imageTemp = new String[numOfEventsPassed];
         eventTimes = new String[numOfEventsPassed];
         eventDisplayDates = new String[numOfEventsPassed];
         eventDisplayTimes = new String[numOfEventsPassed];
         startTimes = new String[numOfEventsPassed];
         endTimes = new String[numOfEventsPassed];


        final int[] EventId = new int[numOfEventsPassed];
        final int[] EventPositions = new int[numOfEventsPassed];

        LinearLayoutManager llm = new LinearLayoutManager(getContext());
        rv.setLayoutManager(llm);

        int counter = 0;
        int eventPosition = 0;

        for (int i = 0; i < numOfEventsPassed; i++) {

            additive = stringIDs[i];
            eventIDsAndTimes = appPrefs.getString("eventIDsAndTimes" + additive, "");
            splitEventIDsAndTimes = eventIDsAndTimes.split(" ");

            //the interval is exclusive, so not including tomorrow (1)
            DateTime fiveDays = DateTime.now().plusDays(5);
            DateTime tomorrow = DateTime.now().plusDays(1);
            DateTimeFormatter fmt = DateTimeFormat.forPattern("yyyy-MM-dd");
            if(eventIDsAndTimes != "") {
                DateTime particularDate = fmt.parseDateTime(splitEventIDsAndTimes[0].trim());


                if (new Interval(tomorrow, fiveDays).contains(particularDate)) {
                    societyName[counter] = appPrefs.getString("societyName" + additive, "");
                    eventName[counter] = appPrefs.getString("eventName" + additive, "");
                    imageTemp[counter] = appPrefs.getString("imageTemp" + additive, "");
                    eventTimes[counter] = eventIDsAndTimes;
                    eventDisplayDates[counter] = eventTimes[counter].split("-")[2].split(" ")[0] + "/" + eventTimes[counter].split("-")[1].trim();
                    Log.v("DisplayDates", ""+eventDisplayDates[counter]);
                    eventDisplayTimes[counter] = eventTimes[counter].split(" ")[1];
                    startTimes[counter] = eventDisplayTimes[counter].split("-")[0].trim();
                    endTimes[counter] = eventDisplayTimes[counter].split("-")[1].trim();
                    if(startTimes[counter].equals(endTimes[counter]))
                        eventDisplayTimes[counter] = eventDisplayTimes[counter].split("-")[0];
                    Log.v("displayTime", "" + eventDisplayTimes[counter]);
                    EventId[counter] = Integer.parseInt(stringIDs[i]);
                    EventPositions[counter] = eventPosition;
                    Log.v("event_id", ""+EventId);
                    counter++;
                }
            }
            eventPosition++;
        }

        counter1 = counter;

        initializeData();

        //Pass filtered IDs to RVAdapter
        StringBuilder sb = new StringBuilder();
        for(int i = 0; i < counter1; i++){
            sb.append(EventId[i]).append(',');
        }

        //Pass positions
        StringBuilder sb1 = new StringBuilder();
        for(int i = 0; i < counter1; i++){
            sb1.append(EventPositions[i]).append(',');
        }

        String filteredIds = sb.toString();
        String eventPositions = sb1.toString();

        RVAdapter RVadapter = new RVAdapter(Ent_Cards, filteredIds, eventPositions);
        rv.setAdapter(RVadapter);

        return v;
    }

    public void initializeData() {
        Ent_Cards = new ArrayList<>();

        for (int i = 0; i < counter1; i++) {
            byte[] eventsData;
            Bitmap bm;

            eventsData = Base64.decode(imageTemp[i], Base64.DEFAULT);
            bm = BitmapFactory.decodeByteArray(eventsData, 0, eventsData.length);
            Ent_Cards.add(new Ent_CardItem(eventName[i], eventDisplayDates[i] + '\n' + startTimes[i], bm));
        }

    }

}

package com.tea.cmcdona2.casper.Admin;


import android.app.DialogFragment;
import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.tea.cmcdona2.casper.R;

import java.text.SimpleDateFormat;
import java.util.Calendar;

import java.util.Locale;

public class AdminActivity extends AppCompatActivity implements DatePickerFragment.TheDateListener, TimePickerFragment.TheTimeListener{
    TextView startDate, endDate;
    TextView startTime, endTime;
    EditText image;
    private static final int PICKFILE_RESULT_CODE =1;
    int clicked = 5;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.admin_activity_main);
        //Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        //setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
                Toast.makeText(getApplicationContext(), "Event Added", Toast.LENGTH_SHORT).show();
                Intent hIntent = new Intent(AdminActivity.this, AdminActivity.class);
                startActivity(hIntent);
            }
        });

        Calendar cal = Calendar.getInstance();
        startDate = (TextView) findViewById(R.id.startDate);
        SimpleDateFormat date = new SimpleDateFormat("EEE, MMM dd, yyyy", Locale.UK);
        startDate.setText(date.format(cal.getTime()));

        startDate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                clicked = 0;
                DialogFragment newFragment = new DatePickerFragment();
                newFragment.show(getFragmentManager(), "datepicker");


            }
        });

        startTime = (TextView) findViewById(R.id.startTime);

        SimpleDateFormat time = new SimpleDateFormat("hh:mm a", Locale.UK);
        startTime.setText(time.format(cal.getTime()));

        startTime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                clicked = 0;
                DialogFragment newFragment = new TimePickerFragment();
                newFragment.show(getFragmentManager(), "timePicker");

            }
        });


        endDate = (TextView) findViewById(R.id.endDate);
        endDate.setText(date.format(cal.getTime()));

        endDate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                clicked = 1;
                DialogFragment newFragment = new DatePickerFragment();
                newFragment.show(getFragmentManager(), "datepicker");

            }
        });

        endTime = (TextView) findViewById(R.id.endTime);
        cal.add(Calendar.HOUR_OF_DAY, 1);
        endTime.setText(time.format(cal.getTime()));

        endTime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                clicked = 1;
                DialogFragment newFragment = new TimePickerFragment();
                newFragment.show(getFragmentManager(), "timePicker");

            }
        });


        EditText description = (EditText) findViewById(R.id.description);
        description.setMovementMethod(new ScrollingMovementMethod());

        image = (EditText) findViewById(R.id.image);
        image.setOnClickListener(new View.OnClickListener() {


            @Override
            public void onClick(View v) {

                Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.setType("file/*");
                startActivityForResult(intent,PICKFILE_RESULT_CODE);
            }


        });


    }




    @Override
    public void returnDate(String date) {
        // TODO Auto-generated method stub

        if (clicked ==0)
            startDate.setText(date);

        else if (clicked==1)
            endDate.setText(date);
    }
    @Override
    public void returnTime(String date) {
        // TODO Auto-generated method stub
        if (clicked ==0)
            startTime.setText(date);

        else if (clicked==1)
            endTime.setText(date);
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        switch (requestCode) {
            case PICKFILE_RESULT_CODE:
                if (resultCode == RESULT_OK) {
                    String FilePath = data.getData().getPath();
                    image.setText(FilePath);
                }
                break;

        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
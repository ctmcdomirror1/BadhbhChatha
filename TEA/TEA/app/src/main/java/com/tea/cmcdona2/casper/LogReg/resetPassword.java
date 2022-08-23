package com.tea.cmcdona2.casper.LogReg;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.tea.cmcdona2.casper.Ents.EntsActivity;
import com.tea.cmcdona2.casper.Other.Constants;
import com.tea.cmcdona2.casper.R;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

public class resetPassword extends AppCompatActivity implements View.OnClickListener {

    Button press;
    //Button reglink;
    public EditText etEmail, etPassword;
    TextView reglink;
    boolean userExists;
    String email, password;
    String encryptedEmail;
    String encryptedPassword;

    //DatabaseHelper databaseHelper;
    //LocalUserHelpeRr localUserHelper;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_reset_password);

        press = (Button) findViewById(R.id.breset);
        etEmail = (EditText) findViewById(R.id.reEmail);
        reglink = (TextView) findViewById(R.id.tvRegisterLink);

        press.setOnClickListener(this);
        reglink.setOnClickListener(this);

        //databaseHelper = new DatabaseHelper(this);
        //localUserHelper = new LocalUserHelper(this);
    }


    @Override
    public void onClick(View view) {

        email = etEmail.getText().toString().trim();



        switch (view.getId()) {

            case R.id.breset:

                //String email = etEmail.getText().toString();
                //String password = etPassword.getText().toString();




                if (!email.equals("")) {

                    reset();


                }

                break;

            case R.id.tvRegisterLink:
                Intent registerIntent = new Intent(this, SocietyorStudent.class);
                startActivity(registerIntent);

                break;
        }

    }

    private void reset() {
        email = etEmail.getText().toString().trim();
        //password = etPassword.getText().toString().trim();

        StringRequest stringRequest = new StringRequest(Request.Method.POST, Constants.SENDEMAIL_URL,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        if (response.trim().equals("success")) {
;

                            Intent hIntent = new Intent(resetPassword.this, LogIn.class);
                            startActivity(hIntent);
                        } else {
                            Toast.makeText(resetPassword.this, response, Toast.LENGTH_LONG).show();
                        }
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Toast.makeText(resetPassword.this, error.toString(), Toast.LENGTH_LONG).show();
                    }
                }) {
            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                Map<String, String> map = new HashMap<String, String>();
                map.put(Constants.KEY_EMAIL, email);
                return map;
            }
        };

        RequestQueue requestQueue = Volley.newRequestQueue(this);
        requestQueue.add(stringRequest);
    }




}

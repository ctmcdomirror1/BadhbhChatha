package com.tea.cmcdona2.casper.LogReg;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
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
import com.tea.cmcdona2.casper.R;
import com.tea.cmcdona2.casper.Other.Constants;


import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

public class LogIn extends AppCompatActivity implements View.OnClickListener {

    Button press;
    //Button reglink;
    public EditText etEmail, etPassword;
    TextView reglink;
    TextView resetlink;
    boolean userExists;
    String email, password;
    String encryptedEmail;
    String encryptedPassword;

    //DatabaseHelper databaseHelper;
    //LocalUserHelpeRr localUserHelper;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_log_in);

        press = (Button) findViewById(R.id.thing);
        etEmail = (EditText) findViewById(R.id.etEmail);
        etPassword = (EditText) findViewById(R.id.etPassword);
        reglink = (TextView) findViewById(R.id.tvRegisterLink);
        resetlink = (TextView) findViewById(R.id.resetpass);

        press.setOnClickListener(this);
        reglink.setOnClickListener(this);
        resetlink.setOnClickListener(this);

        //databaseHelper = new DatabaseHelper(this);
        //localUserHelper = new LocalUserHelper(this);
    }


    @Override
    public void onClick(View view) {

        email = etEmail.getText().toString().trim();
        password = etPassword.getText().toString().trim();
        encryptedEmail = md5(email);
        encryptedPassword = md5(password);


        switch (view.getId()) {

            case R.id.thing:

                //String email = etEmail.getText().toString();
                //String password = etPassword.getText().toString();

                if (email.equals("")) {
                    Toast.makeText(this, "Please enter email", Toast.LENGTH_SHORT).show();
                }
                if (password.equals("")) {
                    Toast.makeText(this, "Please enter password", Toast.LENGTH_SHORT).show();
                }
                if (!email.equals("") && !password.equals("")) {

                    userLogin();



                }

                break;

            case R.id.tvRegisterLink:
                Intent registerIntent = new Intent(this, SocietyorStudent.class);
                startActivity(registerIntent);

                break;
            case R.id.resetpass:
                Intent resetIntent = new Intent(this, resetPassword.class);
                startActivity(resetIntent);

                break;
        }

    }

    private void userLogin() {
        email = etEmail.getText().toString().trim();
        //password = etPassword.getText().toString().trim();

        StringRequest stringRequest = new StringRequest(Request.Method.POST, Constants.LOGIN_URL,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        if(response.trim().equals("success")){

                            SharedPreferences appPrefs = LogIn.this.getSharedPreferences("appPrefs", 0);
                            final SharedPreferences.Editor appPrefsEditor = appPrefs.edit();
                            //boolean alreadyRegistered = true;
                            appPrefsEditor.putString("loggedInUser", email).commit();

                            Intent hIntent = new Intent(LogIn.this, EntsActivity.class);
                            startActivity(hIntent);
                        }else{
                            Toast.makeText(LogIn.this,response,Toast.LENGTH_LONG).show();
                        }
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Toast.makeText(LogIn.this,error.toString(),Toast.LENGTH_LONG ).show();
                    }
                }){
            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                Map<String,String> map = new HashMap<String,String>();
                map.put(Constants.KEY_EMAIL,email);
                map.put(Constants.KEY_PASSWORD, encryptedPassword);
                return map;
            }
        };

        RequestQueue requestQueue = Volley.newRequestQueue(this);
        requestQueue.add(stringRequest);
    }

    public String md5(String s) {
        try {
            // Create MD5 Hash
            MessageDigest digest = java.security.MessageDigest.getInstance("MD5");
            digest.update(s.getBytes());
            byte messageDigest[] = digest.digest();

            // Create Hex String
            StringBuffer hexString = new StringBuffer();
            for (int i=0; i<messageDigest.length; i++)
                hexString.append(Integer.toHexString(0xFF & messageDigest[i]));
            return hexString.toString();

        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return "";
    }

    @Override
    public void onBackPressed() {
        //Include the code here
        return;
    }
}


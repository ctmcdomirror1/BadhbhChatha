package com.tea.cmcdona2.casper.LogReg;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.Button;

import com.tea.cmcdona2.casper.R;

public class SocietyorStudent extends AppCompatActivity implements View.OnClickListener {

    Button studentPress, societyPress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_societyor_student);

        studentPress = (Button) findViewById(R.id.studentButton);
        societyPress = (Button) findViewById(R.id.societyButton);

        studentPress.setOnClickListener(this);
        societyPress.setOnClickListener(this);

    }

    @Override
    public void onClick(View view) {

        switch (view.getId()) {

            case R.id.studentButton:
                Intent registerIntent = new Intent(this, Register.class);
                startActivity(registerIntent);
                break;
            case R.id.societyButton:
                Intent societyRegisterIntent = new Intent(this, SocietyRegister.class);
                startActivity(societyRegisterIntent);
                break;
        }
    }
}

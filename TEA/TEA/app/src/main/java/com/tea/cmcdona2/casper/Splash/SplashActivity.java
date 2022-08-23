package com.tea.cmcdona2.casper.Splash;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.Looper;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.widget.Toast;

import com.tea.cmcdona2.casper.Ents.EntsActivity;
import com.tea.cmcdona2.casper.LogReg.LogIn;
import com.tea.cmcdona2.casper.R;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.logging.Handler;

/**
 * Created by Paddy on 18/11/2015.
 */
public class SplashActivity extends Activity {

    public Boolean alreadyRegistered;
    public Boolean firstTimeToRun;
    private Thread downloadThread[] = new Thread[2];
    public Boolean loop = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        final SharedPreferences appPrefs = SplashActivity.this.getSharedPreferences("appPrefs", 0);
        final SharedPreferences.Editor appPrefsEditor = appPrefs.edit();

        alreadyRegistered = appPrefs.getBoolean("alreadyRegistered", false);
        firstTimeToRun = appPrefs.getBoolean("firstTimeToRun", true);
        // TODO Auto-generated method stub

        super.onCreate(savedInstanceState);
        setContentView(R.layout.splash);

        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

        int screenWidth = displayMetrics.widthPixels;
        appPrefsEditor.putInt("screenWidth", screenWidth).apply();

        final String eventsUrl = "http://clontarfguitarlessons.com/getEvents.php";
        final String infoUrl = "http://clontarfguitarlessons.com/getInfo.php";
        final ConnectivityManager connMgr = (ConnectivityManager)
                getSystemService(Context.CONNECTIVITY_SERVICE);
        final NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();
        if (networkInfo != null && networkInfo.isConnected()) {
            //new DownloadWebpageTask().execute(stringUrl);
            doDownload(eventsUrl, "getEvents.php",0);
            if(firstTimeToRun) {
                doDownload(infoUrl, "getInfo.php", 1);
            }

            Thread timerThread = new Thread(){
                public void run(){
                    try{
                        downloadThread[0].join();
                        if(firstTimeToRun) {
                            downloadThread[1].join();
                        }
                    }
                    catch(InterruptedException e){
                        e.printStackTrace();
                    }
                    finally{

                        appPrefsEditor.putBoolean("firstTimeToRun",false).commit();

                        if(!alreadyRegistered) {
                            Intent loginIntent = new Intent(SplashActivity.this, LogIn.class);
                            startActivity(loginIntent);
                            finish();
                        }

                        else{
                            Intent entsIntent = new Intent(SplashActivity.this, EntsActivity.class);
                            startActivity(entsIntent);
                            finish();
                        }
                    }
                }

            };
            timerThread.start();
        } else {
            // display error

            final Thread timerThread = new Thread() {
                public void run() {
                    try {
                        //final NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();
                        if (networkInfo != null && networkInfo.isConnected()) {
                            sleep(1250);
                        }
                        else{
                            sleep(2500);
                        }
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    } finally {
                        if (!firstTimeToRun) {
                            Intent entsIntent = new Intent(SplashActivity.this, EntsActivity.class);
                            startActivity(entsIntent);
                            finish();
                        } else {
                            finish();
                        }
                    }
                }

            };
            timerThread.start();

            if(firstTimeToRun) {
                Toast.makeText(SplashActivity.this, "Wifi required for setup", Toast.LENGTH_LONG).show();
            }


        }


    }


    protected void doDownload(final String urlLink, final String fileName, final int threadNo) {
        downloadThread[threadNo] = new Thread() {
            public void run() {
                File file= new File(getApplicationContext().getApplicationContext().getFilesDir(),fileName);
                try {
                    URL url = new URL(urlLink);
                    //Log.i("FILE_NAME", "File name is "+fileName);
                    //Log.i("FILE_URLLINK", "File URL is "+url);
                    URLConnection connection = url.openConnection();
                    connection.connect();
                    // this will be useful so that you can show a typical 0-100% progress bar
                    int fileLength = connection.getContentLength();

                    // download the file
                    InputStream input = new BufferedInputStream(url.openStream());
                    OutputStream output = new FileOutputStream(file);

                    byte data[] = new byte[1024];
                    long total = 0;
                    int count;
                    while ((count = input.read(data)) != -1) {
                        total += count;

                        output.write(data, 0, count);
                    }

                    output.flush();
                    output.close();
                    input.close();
                } catch (Exception e) {
                    e.printStackTrace();
                    Log.i("Download", "ERROR IS" + e);
                }
            }
        };
        downloadThread[threadNo].start();
    }
}

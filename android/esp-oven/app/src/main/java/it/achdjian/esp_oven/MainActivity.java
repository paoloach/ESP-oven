package it.achdjian.esp_oven;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import it.achdjian.esp_oven.runnable.Connect;
import it.achdjian.esp_oven.runnable.RetrieveTemperature;
import it.achdjian.esp_oven.runnable.RetrieveThreshold;
import it.achdjian.esp_oven.runnable.SetThreshold;

public class MainActivity extends AppCompatActivity  implements View.OnClickListener {
    private static final String ADDRESS_OVEN = "OVEN ADDRESS";
    private static final String TAG = MainActivity.class.getName();
    private Button connectButton;
    private String address=null;
    private Connect connect;
    private ScheduledExecutorService scheduleAtFixedRate;
    private ScheduledFuture<?> retrieveTemperatureFuture;
    private ScheduledFuture<?> retrieveThresholdFuture;
    private TextView txTemp;
    private TextView tempView;
    private TextView txThreshold;
    private TextView thresholdView;
    private TextView newThresholdView;
    private EditText newThresholdEdit;
    private Button   newThresholdSet;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        connectButton = (Button)findViewById(R.id.btConnection);
        connectButton.setOnClickListener(this);

        txTemp = (TextView)findViewById(R.id.txTemp);
        tempView = (TextView)findViewById(R.id.temperature);
        txThreshold = (TextView)findViewById(R.id.txThreshold);
        thresholdView = (TextView)findViewById(R.id.thresholdView);
        newThresholdView = (TextView)findViewById(R.id.txNewThreshold);
        newThresholdEdit = (EditText)findViewById(R.id.newThreshold);
        newThresholdSet = (Button)findViewById(R.id.setNewThreshold);
        newThresholdSet.setOnClickListener(this);
        this.enable(false);
        scheduleAtFixedRate =  Executors.newSingleThreadScheduledExecutor();
        if (savedInstanceState != null){
            address = savedInstanceState.getString(ADDRESS_OVEN);
            reconnect();
        }
    }

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        savedInstanceState.putString(ADDRESS_OVEN, address);
        super.onSaveInstanceState(savedInstanceState);
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        removeThreads();
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

    @Override
    public void onClick(View v) {
        if (v == connectButton){
            reconnect();
        } else if (v == newThresholdSet){
            setThreshold();
        }
    }

    private void setThreshold() {
        int threshold = Integer.parseInt(newThresholdEdit.getText().toString());
        SetThreshold setThreshold = new SetThreshold(address, this, threshold);
        new Thread(setThreshold).start();
    }

    public void reconnect(){
        enable(false);
        connect = new Connect(this, address);
        new Thread(connect).start();
    }

    public void enable() {
        address = connect.getLastAddress();
        Log.d(TAG,"retrieveTemperatureFuture = " + retrieveTemperatureFuture);
        if (retrieveTemperatureFuture == null) {
            RetrieveTemperature retrieveTemperature = new RetrieveTemperature(address, this);
            retrieveTemperatureFuture = scheduleAtFixedRate.scheduleAtFixedRate(retrieveTemperature, 0, 1, TimeUnit.SECONDS);
        }
        if (retrieveThresholdFuture == null) {
            RetrieveThreshold retrieveThreshold = new RetrieveThreshold(address, this);
            retrieveThresholdFuture = scheduleAtFixedRate.scheduleAtFixedRate(retrieveThreshold, 0, 10, TimeUnit.SECONDS);
        }
        enable(true);
    }

    public void enable(boolean status) {
        if (!status) {
            removeThreads();
        }
        txTemp.setEnabled(status);
        tempView.setEnabled(status);
        txThreshold.setEnabled(status);
        thresholdView.setEnabled(status);
        newThresholdView.setEnabled(status);
        newThresholdEdit.setEnabled(status);
        newThresholdSet.setEnabled(status);
        connectButton.setEnabled(!status);
    }

    private void removeThreads() {
        if (retrieveTemperatureFuture != null){
            Log.d(TAG, "cancel temperature thread");
            retrieveTemperatureFuture.cancel(true);
            retrieveTemperatureFuture=null;
        }
        if (retrieveThresholdFuture != null){
            retrieveThresholdFuture.cancel(true);
            retrieveThresholdFuture=null;
        }
    }

    public void setTemperature(int temperature) {
        tempView.setText(Integer.toString(temperature));
    }

    public void setThresholdView(int threshold) {
        thresholdView.setText(Integer.toString(threshold));
    }
}

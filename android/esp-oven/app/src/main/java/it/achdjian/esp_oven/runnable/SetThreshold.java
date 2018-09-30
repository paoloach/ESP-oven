package it.achdjian.esp_oven.runnable;

import android.util.Log;

import org.springframework.http.HttpEntity;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.http.client.HttpComponentsClientHttpRequestFactory;
import org.springframework.web.client.RestTemplate;

import it.achdjian.esp_oven.MainActivity;

/**
 * Created by paolo on 02/01/16.
 */
public class SetThreshold implements Runnable {
    private static final String TAG = RetrieveThreshold.class.getName();
    private String address;
    private MainActivity mainActivity;
    private int newThreshold;
    RestTemplate restTemplate;

    public SetThreshold(String address, MainActivity mainActivity, int netThreshold) {
        this.address = address;
        this.mainActivity = mainActivity;
        this.newThreshold = netThreshold;
        HttpComponentsClientHttpRequestFactory factory = new HttpComponentsClientHttpRequestFactory();
        factory.setReadTimeout(2000);
        factory.setConnectTimeout(2000);
        restTemplate = new RestTemplate(factory);
    }

    @Override
    public void run() {
        String url = "http://" + address + "/threshold";
        Log.d(TAG, "set new threshold at " + newThreshold);
        HttpEntity httpEntity = new HttpEntity(Integer.toString(newThreshold));
        try {
            ResponseEntity<String> response = restTemplate.exchange(url, HttpMethod.PUT, httpEntity, String.class);
            HttpStatus statusCode = response.getStatusCode();
            if (statusCode != HttpStatus.OK)
                Log.d(TAG, "statusCode: " + statusCode.value());
            mainActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mainActivity.reconnect();
                }
            });
        } catch (Exception e){
            mainActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mainActivity.reconnect();
                }
            });
        }

    }
}

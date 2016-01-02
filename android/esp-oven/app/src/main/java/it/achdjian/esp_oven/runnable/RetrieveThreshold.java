package it.achdjian.esp_oven.runnable;

import android.util.Log;

import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.http.client.HttpComponentsClientHttpRequestFactory;
import org.springframework.web.client.RestTemplate;

import it.achdjian.esp_oven.MainActivity;

/**
 * Created by paolo on 01/01/16.
 */
public class RetrieveThreshold implements Runnable {
    private static final String TAG = RetrieveThreshold.class.getName();
    private String address;
    private MainActivity mainActivity;
    static private int counter = 0;
    RestTemplate restTemplate;

    public RetrieveThreshold(String address, MainActivity mainActivity) {
        this.address = address;
        this.mainActivity = mainActivity;
        HttpComponentsClientHttpRequestFactory factory = new HttpComponentsClientHttpRequestFactory();
        factory.setReadTimeout(2000);
        factory.setConnectTimeout(2000);
        restTemplate = new RestTemplate(factory);
    }

    @Override
    public void run() {
        counter++;
        String url = "http://" + address + "/threshold";
        HttpHeaders headers = new HttpHeaders();
        headers.set("xxx-achdjian-counter", Integer.toString(counter));
        HttpEntity<String> entity = new HttpEntity<>(headers);
        ResponseEntity<String> response = restTemplate.exchange(url, HttpMethod.GET, entity, String.class);
        HttpStatus statusCode = response.getStatusCode();
        if (statusCode == HttpStatus.OK) {
            final int threshold = Integer.parseInt(response.getBody());
            mainActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mainActivity.setThresholdView(threshold);
                }
            });
        } else {
            Log.d(TAG, "statusCode: " + statusCode.value());
            mainActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mainActivity.reconnect();
                }
            });

        }
    }
}

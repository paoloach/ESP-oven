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
public class RetrieveTemperature implements Runnable {
    private static final String TAG = RetrieveTemperature.class.getName();
    private static int counter=0;
    private String address;
    private MainActivity mainActivity;
    RestTemplate restTemplate = new RestTemplate();

    public RetrieveTemperature(String address, MainActivity mainActivity) {
        this.address = address;
        this.mainActivity = mainActivity;
        HttpComponentsClientHttpRequestFactory factory = new HttpComponentsClientHttpRequestFactory();
        factory.setReadTimeout(2000);
        factory.setConnectTimeout(2000);
        restTemplate = new RestTemplate(factory);
    }

    @Override
    public void run() {
        String url = "http://" + address + "/temperature";
        Log.d(TAG, "Requesting temperature");
        HttpStatus statusCode=HttpStatus.BAD_REQUEST;
        ResponseEntity<String> response=null;
        try {
            counter++;
            HttpHeaders headers = new HttpHeaders();
            headers.set("xxx-achdjian-counter", Integer.toString(counter));
            HttpEntity<String> entity = new HttpEntity<>(headers);
            response = restTemplate.exchange(url, HttpMethod.GET, entity, String.class);

            statusCode = response.getStatusCode();

        } catch(Exception e){
            Log.d(TAG, "SocketException: " + e.getMessage());
            e.printStackTrace();
            mainActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mainActivity.reconnect();
                }
            });
        }
        Log.d(TAG, "Requested temperature");
        if (statusCode == HttpStatus.OK) {
            final int temp = Integer.parseInt(response.getBody());
            mainActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mainActivity.setTemperature(temp);
                }
            });
        } else {
            Log.d(TAG, "statusCode: " + statusCode.value());
            mainActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mainActivity.enable(false);
                }
            });
        }
    }
}

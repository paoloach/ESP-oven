package it.achdjian.esp_oven.rest.get;

import android.util.Log;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.RestTemplate;

/**
 * Created by paolo on 01/01/16.
 */
public class OvenVersion {
    private static final String TAG = OvenVersion.class.getName();

    private final static String PREFIX = "I am ESP-OVEN";

    private int majorVersion;
    private int minorVersion;
    private boolean valid;

    public OvenVersion(String target) {
        valid = false;
        majorVersion =0;
        minorVersion=0;
        RestTemplate restTemplate = new RestTemplate();
        String url = "http://" + target + "/whoareyou";
        try {
            ResponseEntity<String> response = restTemplate.getForEntity(url, String.class);
            HttpStatus statusCode = response.getStatusCode();
            Log.d(TAG, "statusCode: " + statusCode.value());
            if (statusCode == HttpStatus.OK) {
                String body = response.getBody();
                Log.d(TAG, "response: " + body);
                parseResponse(body);
            }
        } catch (Exception e) {
            String message = e.getMessage();
            if (message != null) {
                Log.d(TAG, message);
            }
            valid=false;
        }
    }

    private void parseResponse(String version) {
        if (version.length() >= PREFIX.length()) {
            if (version.substring(0, PREFIX.length()).equals(PREFIX)) {
 /*               String strVersion = version.substring(PREFIX.length(), version.length());
                String[] split = strVersion.split("[.]");
                if (split.length == 3) {
                    majorVersion = Integer.parseInt(split[0].trim());
                    minorVersion = Integer.parseInt(split[1].trim());
                    valid = true;
                }
                */
                valid=true;
                majorVersion=1;
                minorVersion=0;
            }
        }
    }


    public int getMajorVersion() {
        return majorVersion;
    }

    public int getMinorVersion() {
        return minorVersion;
    }

    public boolean isValid() {
        return valid;
    }

    @Override
    public String toString() {
        if (valid)
            return Integer.toString(majorVersion) + "." + minorVersion;
        else
            return "invalid";
    }
}

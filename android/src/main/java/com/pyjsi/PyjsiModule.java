package com.pyjsi;

import android.util.Log;
import com.pyjsi.bridge.*;
import androidx.annotation.NonNull;

import com.facebook.react.bridge.JavaScriptContextHolder;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.module.annotations.ReactModule;

import org.json.JSONObject;
import org.json.JSONException;

@ReactModule(name = PyJsiModule.NAME)
public class PyJsiModule extends ReactContextBaseJavaModule {
  public static final String NAME = "PyJsiModule";

  public PyJsiModule(ReactApplicationContext reactContext) {
    super(reactContext);
  }

  @Override
  @NonNull
  public String getName() {
    return NAME;
  }

  /**
   * Initializes the Python interpreter.
   *
   * @param datapath the location of the extracted python files
   * @return error code
   */
//  public static native int start(String datapath);

  /**
   * Stops the Python interpreter.
   *
   * @return error code
   */
//  public static native int stop();

  /**
   * Sends a string payload to the Python interpreter.
   *
   * @param payload the payload string
   * @return a string with the result
   */
//  public static native String call(String payload);

  /**
   * Sends a JSON payload to the Python interpreter.
   *
   * @param payload JSON payload
   * @return JSON response
   */
//  public static JSONObject call(JSONObject payload) {
//    String result = call(payload.toString());
//    try {
//      return new JSONObject(result);
//    } catch (JSONException e) {
//      e.printStackTrace();
//      return null;
//    }
//  }

  @ReactMethod(isBlockingSynchronousMethod = true)
  public boolean install() {
    JavaScriptContextHolder jsContext = getReactApplicationContext().getJavaScriptContextHolder();
    // Extract python files from assets
    AssetExtractor assetExtractor = new AssetExtractor(getReactApplicationContext());
    assetExtractor.removeAssets("python");
    assetExtractor.copyAssets("python");

    // Get the extracted assets directory
    String pythonPath = assetExtractor.getAssetsDataDir() + "python";

    // Start the Python interpreter
//    PyJsiModule.start(pythonPath);
    try {
      Log.i(NAME, "Loading pyjsi C++ library...");
      System.loadLibrary("pyjsi");
      nativeInstall(jsContext.get(), pythonPath);
      Log.i(NAME, "Successfully installed pyjsi JSI Bindings!");
      return true;
    } catch (Exception exception) {
      Log.e(NAME, "Failed to install pyjsi Bindings!", exception);
      return false;
    }
  }

  private static native void nativeInstall(long jsiPtr, String pythonPath);
}

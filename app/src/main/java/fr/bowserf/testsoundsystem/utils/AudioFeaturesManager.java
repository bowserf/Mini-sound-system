package fr.bowserf.testsoundsystem.utils;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.support.annotation.NonNull;

/**
 * Class used to get audio feature of the current device. Used to optimize sound system performance.
 */
public class AudioFeaturesManager {

    @SuppressWarnings("unused")
    private static final String TAG = "AudioFeaturesManager";

    private static AudioFeaturesManager sInstance;

    public static AudioFeaturesManager init(@NonNull final Context context){
        if(sInstance == null){
            sInstance = new AudioFeaturesManager(context);
        }
        return sInstance;
    }

    public static AudioFeaturesManager getInstance(){
        if(sInstance == null){
            throw new IllegalStateException("AudioFeaturesManager#init(Context) must be call before getting instance of AudioFeaturesManager");
        }
        return sInstance;
    }

    private boolean mHasLowLatencyFeature;
    private boolean mHasProFeature;
    private int mSampleRate;
    private int mFramesPerBufferInt;

    private AudioFeaturesManager(@NonNull final Context context){
        getAudioInformation(context);
    }

    private void getAudioInformation(@NonNull final Context context) {
        mHasLowLatencyFeature = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_AUDIO_LOW_LATENCY);

        mHasProFeature = false;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            mHasProFeature = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_AUDIO_PRO);
        }

        final android.media.AudioManager am = (android.media.AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        // get native sample rate
        final String sampleRateStr = am.getProperty(android.media.AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        mSampleRate = Integer.parseInt(sampleRateStr);
        if (mSampleRate == 0) { // Use a default value if property not found
            mSampleRate = 44100;
        }

        // get native buffer size
        final String framesPerBuffer = am.getProperty(android.media.AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        mFramesPerBufferInt = Integer.parseInt(framesPerBuffer);
        if (mFramesPerBufferInt == 0) { // Use default
            mFramesPerBufferInt = 256;
        }
    }

    public boolean isHasLowLatencyFeature() {
        return mHasLowLatencyFeature;
    }

    public boolean isHasProFeature() {
        return mHasProFeature;
    }

    public int getSampleRate() {
        return mSampleRate;
    }

    public int getFramesPerBufferInt() {
        return mFramesPerBufferInt;
    }
}

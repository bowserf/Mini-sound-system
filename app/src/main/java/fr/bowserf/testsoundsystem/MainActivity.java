package fr.bowserf.testsoundsystem;

import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.ToggleButton;

import fr.bowserf.soundsystem.SoundSystem;
import fr.bowserf.soundsystem.listener.SSExtractionObserver;
import fr.bowserf.soundsystem.listener.SSPlayingStatusObserver;

/**
 * Simple activity launching the sound system.
 */
public class MainActivity extends AppCompatActivity {

    @SuppressWarnings("unused")
    private static final String TAG = "MainActivity";

    /**
     * Audio params
     */
    private boolean mHasLowLatencyFeature;
    private boolean mHasProFeature;
    private int mSampleRate;
    private int mFramesPerBufferInt;

    /**
     * UI
     */
    private Button mToggleStop;
    private Button mBtnExtractFile;
    private TextView mTvSoundSystemStatus;
    private ToggleButton mTogglePlayPause;

    /**
     * Sound system
     */
    private SoundSystem mSoundSystem;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getAudioInformation();

        mSoundSystem = SoundSystem.getInstance(this);
        mSoundSystem.initSoundSystem(
                mSampleRate,
                mFramesPerBufferInt);

        initUI();

        attachToListeners();
    }

    private void initUI() {
        // stop button
        mToggleStop = (Button) findViewById(R.id.btn_stop);
        mToggleStop.setOnClickListener(mOnClickListener);
        mToggleStop.setEnabled(false);

        //extract button
        mBtnExtractFile = (Button) findViewById(R.id.toggle_extract_file);
        mBtnExtractFile.setOnClickListener(mOnClickListener);

        //tv sound system status
        mTvSoundSystemStatus = (TextView) findViewById(R.id.tv_sound_system_status);

        //play pause button
        mTogglePlayPause = (ToggleButton) findViewById(R.id.toggle_play_pause);
        mTogglePlayPause.setOnCheckedChangeListener(mOnCheckedChangeListener);
        mTogglePlayPause.setEnabled(false);
    }

    @Override
    protected void onDestroy() {
        detachListeners();
        mSoundSystem.release();
        super.onDestroy();
    }

    private void attachToListeners() {
        mSoundSystem.addPlayingStatusObserver(mSSPlayingStatusObserver);
        mSoundSystem.addExtractionObserver(mSSExtractionObserver);
    }

    private void detachListeners() {
        mSoundSystem.removePlayingStatusObserver(mSSPlayingStatusObserver);
        mSoundSystem.removeExtractionObserver(mSSExtractionObserver);
    }

    private SSExtractionObserver mSSExtractionObserver = new SSExtractionObserver() {
        @Override
        public void onExtractionStarted() {
            mBtnExtractFile.setEnabled(false);
            mTvSoundSystemStatus.setText("Extraction started");
        }

        @Override
        public void onExtractionCompleted() {
            mTogglePlayPause.setEnabled(true);
            mToggleStop.setEnabled(true);
            mTvSoundSystemStatus.setText("Extraction ended");
        }
    };

    private SSPlayingStatusObserver mSSPlayingStatusObserver = new SSPlayingStatusObserver() {
        @Override
        public void onPlayingStatusDidChange(final boolean playing) {
            mTvSoundSystemStatus.setText(playing
                    ? "Playing"
                    : "Pause");
        }

        @Override
        public void onEndOfMusic() {
            mTogglePlayPause.setChecked(false);
            mTvSoundSystemStatus.setText("Track finished");
        }

        @Override
        public void onStopTrack() {
            mTogglePlayPause.setChecked(false);
            mTvSoundSystemStatus.setText("Track stopped");
        }
    };

    private View.OnClickListener mOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.toggle_extract_file:
                    mSoundSystem.loadFile("/sdcard/Music/test.mp3");
                    break;
                case R.id.btn_stop:
                    mSoundSystem.stopMusic();
                    break;
            }
        }
    };

    private CompoundButton.OnCheckedChangeListener mOnCheckedChangeListener = new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            switch (buttonView.getId()) {
                case R.id.toggle_play_pause:
                    mSoundSystem.playMusic(isChecked);
                    break;
            }
        }
    };

    private void getAudioInformation() {
        mHasLowLatencyFeature = getPackageManager().hasSystemFeature(PackageManager.FEATURE_AUDIO_LOW_LATENCY);

        mHasProFeature = false;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            mHasProFeature = getPackageManager().hasSystemFeature(PackageManager.FEATURE_AUDIO_PRO);
        }

        final AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

        // get native sample rate
        final String sampleRateStr = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        mSampleRate = Integer.parseInt(sampleRateStr);
        if (mSampleRate == 0) { // Use a default value if property not found
            mSampleRate = 44100;
        }

        // get native buffer size
        final String framesPerBuffer = am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        mFramesPerBufferInt = Integer.parseInt(framesPerBuffer);
        if (mFramesPerBufferInt == 0) { // Use default
            mFramesPerBufferInt = 256;
        }
    }

}

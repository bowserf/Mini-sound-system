package fr.bowserf.soundsystem;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Handler;

import java.util.ArrayList;
import java.util.List;

import fr.bowserf.soundsystem.listener.SSExtractionObserver;
import fr.bowserf.soundsystem.listener.SSPlayingStatusObserver;

/**
 * Class used to communicate with the sound system.
 */
public class SoundSystem {

    @SuppressWarnings("unused")
    private static final String TAG = "SoundSystem";

    /**
     * Load native library
     */
    static {
        System.loadLibrary("soundsystem");
    }

    /**
     * Private instance of this class.
     */
    private static SoundSystem sInstance;

    /**
     * Used to get an instance of this class. Will instantiate this class the first time this method
     * is called.
     *
     * @return Get the instance of this class.
     */
    public static SoundSystem getInstance(final Context context) {
        if (sInstance == null) {
            sInstance = new SoundSystem(context);
        }
        return sInstance;
    }

    /**
     * List of all playing status observer.
     */
    private final List<SSPlayingStatusObserver> mPlayingStatusObservers;

    /**
     * List of all observer listening for the extraction state.
     */
    private final List<SSExtractionObserver> mExtractionObservers;

    /**
     * Handler attach to the main thread.
     */
    private final Handler mMainHandler;

    /**
     * Private constructor.
     */
    private SoundSystem(final Context context) {
        mMainHandler = new Handler(context.getMainLooper());

        mPlayingStatusObservers = new ArrayList<>();
        mExtractionObservers = new ArrayList<>();
    }

    /**
     * Initialize the sound system.
     *
     * @param nativeFrameRate       Native value tf the device frame rate
     * @param nativeFramesPerBuf    Native value of the number of frames per buffer.
     */
    public void initSoundSystem(final int nativeFrameRate, final int nativeFramesPerBuf) {
        native_init_soundsystem(nativeFrameRate, nativeFramesPerBuf);
    }

    public boolean isSoundSystemInit(){
        return native_is_soundsystem_init();
    }

    /**
     * Load track file into the RAM.
     *
     * @param filePath Path of the file on the hard disk.
     */
    public void loadFile(final String filePath) {
        native_load_file(filePath);
    }

    public void playSong(final Context context){
        native_extract_from_assets_and_play(context.getAssets(), "sound.mp3");
    }

    /**
     * Play music if params is true, otherwise, pause the music.
     *
     * @param isPlaying True if music is playing.
     */
    public void playMusic(final boolean isPlaying) {
        native_play(isPlaying);
    }

    /**
     * Get if a track is currently playing or not.
     * @return  True if a track is played.
     */
    public boolean isPlaying(){
        return native_is_playing();
    }

    /**
     * Stop music.
     */
    public void stopMusic() {
        native_stop();
    }

    /**
     * Release native objects and this object.
     */
    public void release() {
        native_release_soundsystem();
        sInstance = null;
    }

    //---------------
    // - Listeners -
    //---------------

    public boolean addPlayingStatusObserver(final SSPlayingStatusObserver observer) {
        synchronized (mPlayingStatusObservers) {
            //noinspection SimplifiableIfStatement
            if (observer == null || mPlayingStatusObservers.contains(observer)) {
                return false;
            }
            return mPlayingStatusObservers.add(observer);
        }
    }

    public boolean removePlayingStatusObserver(final SSPlayingStatusObserver observer) {
        synchronized (mPlayingStatusObservers) {
            return mPlayingStatusObservers.remove(observer);
        }
    }

    /**
     * Notify that the track is finished
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyPlayingStatusObserversEndTrack() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mPlayingStatusObservers) {
                    for (final SSPlayingStatusObserver observer : mPlayingStatusObservers) {
                        observer.onEndOfMusic();
                    }
                }
            }
        });
    }

    /**
     * Notify of the new player state.
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyPlayingStatusObserversPlayPause(final boolean isPlaying) {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mPlayingStatusObservers) {
                    for (final SSPlayingStatusObserver observer : mPlayingStatusObservers) {
                        observer.onPlayingStatusDidChange(isPlaying);
                    }
                }
            }
        });
    }

    /**
     * Notify track has stopped.
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyStopTrack() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mPlayingStatusObservers) {
                    for (final SSPlayingStatusObserver observer : mPlayingStatusObservers) {
                        observer.onStopTrack();
                    }
                }
            }
        });
    }

    public boolean addExtractionObserver(final SSExtractionObserver observer) {
        synchronized (mExtractionObservers) {
            //noinspection SimplifiableIfStatement
            if (observer == null || mExtractionObservers.contains(observer)) {
                return false;
            }
            return mExtractionObservers.add(observer);
        }
    }

    public boolean removeExtractionObserver(final SSExtractionObserver observer) {
        synchronized (mExtractionObservers) {
            return mExtractionObservers.remove(observer);
        }
    }

    /**
     * Notify that the extraction has started.
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyExtractionStarted() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mExtractionObservers) {
                    for (final SSExtractionObserver observer : mExtractionObservers) {
                        observer.onExtractionStarted();
                    }
                }
            }
        });
    }

    /**
     * Notify that the extraction has finished.
     * Called from native code.
     */
    @SuppressWarnings("unused")
    public void notifyExtractionCompleted() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                synchronized (mExtractionObservers) {
                    for (final SSExtractionObserver observer : mExtractionObservers) {
                        observer.onExtractionCompleted();
                    }
                }
            }
        });
    }

    //--------------------
    // - Native methods -
    //--------------------

    private native void native_init_soundsystem(int nativeFrameRate, int nativeFramesPerBuf);

    private native boolean native_is_soundsystem_init();

    private native void native_load_file(String filePath);

    private native void native_release_soundsystem();

    private native void native_play(boolean play);

    private native boolean native_is_playing();

    private native void native_stop();

    private native void native_extract_and_play(String filePath);

    private native void native_extract_from_assets_and_play(AssetManager assetManager, String filename);
}

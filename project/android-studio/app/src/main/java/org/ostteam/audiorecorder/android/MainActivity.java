package org.ostteam.audiorecorder.android;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.ContentFrameLayout;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Timer;
import java.util.TimerTask;

import android.Manifest;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.provider.MediaStore;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.ImageButton;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
//import androidx.activity;

public class MainActivity  extends AppCompatActivity {

    //Recorder
    private org.ostteam.audiorecorder.Core _rec = new org.ostteam.audiorecorder.Core();
    private boolean _recError = false;

    //Timer
    private Timer _recTimer = new Timer();
    private int _buffersPerSec = 60;
    private int _recTimerMsPerTick = 1000 / 30;
    private Handler _recHandler = new Handler();
    private String _encFilename = "";

    //Permission request
    private static final int _PERM_REQUEST_ID = 987;

    //UI elements
    private TextView _txtTittle     = null;
    private TextView _txtTime       = null;
    private float    _txtTimeValue  = 0;
    private SamplesGraph _graphSamples = null;
    private ImageButton _btnMic     = null;
    private ImageButton _btnPlayBig = null;
    private TimelineGraph _playTimeline = null;
    private ImageButton _btnPlay    = null;
    private ImageButton _btnDone    = null;
    private ImageButton _btnStop    = null;
    private ImageButton _btnDelete  = null;

    //Colors
    private final int _colorMicGreen   = Color.argb(255, 82, 178, 64);
    private final int _colorMicRed     = Color.argb(255, 178, 68, 64);
    private final int _colorMicGray    = Color.argb(255, 143, 143, 143);
    private final int _colorMicYellow  = Color.argb(255, 230, 197, 16);

    //---------------
    //Activity events
    //---------------

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //
        _txtTittle  = (TextView) findViewById(R.id.txtTittle);
        _txtTime    = (TextView) findViewById(R.id.txtTime);
        _graphSamples = (SamplesGraph)findViewById(R.id.graphSamples);
        _btnMic     = (ImageButton)findViewById(R.id.micButton);
        _btnPlayBig = (ImageButton)findViewById(R.id.btnPlayBig);
        _playTimeline = (TimelineGraph)findViewById(R.id.playTimeline);
        _btnPlay    = (ImageButton)findViewById(R.id.btnPlay);
        _btnDone    = (ImageButton)findViewById(R.id.btnDone);
        _btnStop    = (ImageButton)findViewById(R.id.btnStop);
        _btnDelete  = (ImageButton)findViewById(R.id.btnDelete);
        {
            ViewParent parent = _btnMic.getParent();
            if (parent == null) {
                Log.i("OSAR", "_btnMic.getParent() is null");
            } else {
                if (!(parent instanceof View)) {
                    Log.i("OSAR", "_btnMic.getParent() is not a View nor RelativeLayout");
                } else {
                    View layout = (View)parent;
                    layout.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {
                        @Override
                        public void onLayoutChange(View v, int left, int top, int right, int bottom, int oldLeft, int oldTop, int oldRight, int oldBottom) {
                            if (left != 0 || top != 0 || right != 0 || bottom != 0) {
                                syncUserInterface();
                            }
                        }
                    });
                }
            }
        }
        //
        if(!_rec.engineInit()){
            Log.e("OSAR", "engineInit failed.");
        }
        //Timer
        _recTimer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                _recHandler.post(new Runnable() {
                    public void run() {
                        //tick
                        _rec.tick();
                        //Update time
                        if(_rec.isRecording()){
                            float secs = _rec.samplesSecs();
                            if((int)_txtTimeValue != (int)secs){
                                updateTimeTag(secs);
                                _txtTimeValue = secs;
                            }
                            //update audio graphical ref
                            {
                                float avg = _rec.samplesRelAvg(true);
                                _graphSamples.addSampleRelAvg(avg);
                                //Log.i("OSAR", "avg(" + avg + ")");
                            }
                        } else if(_rec.playerIsLoaded()){
                            float secsTotal = _rec.playerSecsTotal();
                            float relProg = _rec.playerRelProgress();
                            float secs = secsTotal * relProg;
                            if((int)_txtTimeValue != (int)secs){
                                updateTimeTag(secs);
                                _txtTimeValue = secs;
                            }
                            _playTimeline.setRelPos(relProg);
                            //
                            {
                                float relPosReq = _playTimeline.getRelPosRequest(true);
                                if(relPosReq >= 0.0f && relPosReq <= 1.0f) {
                                    _rec.playerSeekRelPos(relPosReq);
                                }
                            }
                        }
                        //Encoder
                        if(_rec.encoderIsLoaded()){
                            float relProgress = _rec.encoderRelProgress();
                            if(relProgress == 1.0f){
                                _rec.encoderFinish();
                                Toast.makeText(getBaseContext(), getString(R.string.enc_done), Toast.LENGTH_SHORT).show();
                                btnShare(null, _encFilename);
                                syncUserInterface();
                            } else {
                                _playTimeline.setRelPos(relProgress);
                            }
                        }
                    }
                });
            }
        }, _recTimerMsPerTick, _recTimerMsPerTick);
    }

    @Override
    protected void onStart(){
        super.onStart();
    }

    @Override
    protected void onRestart(){
        super.onRestart();
        this.btnResume(null);
    }

    @Override
    protected void onResume(){
        super.onResume();
        this.btnResume(null);
    }

    @Override
    protected void onPause(){
        super.onPause();
        this.btnStop(null);
        Log.i("OSAR", "_rec.stop (at onPause).");
    }

    @Override
    protected void onStop(){
        this.btnStop(null);
        super.onStop();
        Log.i("OSAR", "_rec.stop (at onStop).");
    }

    @Override
    protected void onDestroy(){
        _rec.engineRelease();
        super.onDestroy();
    }

    //----------
    //Permission
    //----------

    private boolean isRecordPermGranted() {
        return (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED);
    }

    private void requestRecordPermission(){
        //this.requestPermissionLauncher.launch(Manifest.permission.RECORD_AUDIO);
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO}, _PERM_REQUEST_ID);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        //
        if (_PERM_REQUEST_ID != requestCode) {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
            return;
        }
        if (grantResults.length != 1 || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
            //Permission denied
            _recError = true;
            this.syncUserInterface();
            Toast.makeText(getBaseContext(), getString(R.string.perm_not_granted), Toast.LENGTH_SHORT).show();
        } else {
            // Permission was granted, start echoing
            btnStart(null, false);
        }
    }

    //----------
    //Actions
    //----------

    public void btnTime(View view){
        this.syncUserInterface();
    }

    public void btnStart(View view, boolean reqPermIfNecesary){
        boolean exec = true;
        if(reqPermIfNecesary){
            if(!this.isRecordPermGranted()){
                this.requestRecordPermission();
                exec = false;
            }
        }
        //Start
        if(exec) {
            if (!_rec.isActive()) {
                //Start recording
                File file = new File(this.getExternalCacheDir(), "OSSAudioRecorder.wav");
                //File file = new File(getExternalFilesDir(null), "Test.wav");
                if (!_rec.start(file.getAbsolutePath(), _buffersPerSec, true)) {
                    Log.e("OSAR", "_rec.start failed for path '" + file.getAbsolutePath() + "'.");
                    _recError = true;
                    this.syncUserInterface();
                } else {
                    Log.i("OSAR", "_rec.start for path '" + file.getAbsolutePath() + "'.");
                    _recError = false;
                    this._graphSamples.resetGraph();
                    //Stop player
                    if(_rec.playerIsLoaded()){
                        _rec.playerStop();
                    }
                    this.updateTimeTag(0);
                    this.syncUserInterface();
                }
            }
        }
    }

    public void btnStop(View view){
        if(_rec.isActive()) {
            if(_rec.stop()) {
                Log.i("OSAR", "_rec.stop.");
                this.syncUserInterface();
            }
        } else if(_rec.playerIsLoaded()) {
            if(_rec.playerStop()){
                Log.i("OSAR", "_rec.playerStop.");
                this.syncUserInterface();
            }
        }
    }

    public void btnDelete(View view){
        if(!_rec.isActive()) {
            if(_rec.reset()){
                Log.i("OSAR", "_rec.reset.");
                _graphSamples.resetGraph();
                this.updateTimeTag(0);
                this.syncUserInterface();
            }
        }
    }

    public void btnPause(View view){
        if(_rec.isActive() && _rec.isRecording()) {
            if(_rec.pause()){
                Log.i("OSAR", "_rec.pause.");
                this.syncUserInterface();
            }
        } else if(_rec.playerIsLoaded() && !_rec.playerIsPaused()) {
            if(_rec.playerPause()){
                Log.i("OSAR", "_rec.playerPause.");
                this.syncUserInterface();
            }
        }
    }

    public void btnResume(View view){
        if(_rec.isActive() && !_rec.isRecording()) {
            if(_rec.resume()){
                Log.i("OSAR", "_rec.resume.");
                this.syncUserInterface();
            }
        } else if(_rec.playerIsLoaded() && _rec.playerIsPaused()) {
            if(_rec.playerPlay()){
                Log.i("OSAR", "_rec.playerPlay.");
                this.syncUserInterface();
            }
        }
    }

    public void btnShare(View view){
        if(_rec.samplesCount() > 0) {
            String[] formats = { getString(R.string.fmt_desc_wav), getString(R.string.fmt_desc_flac), getString(R.string.fmt_desc_opus) };
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Seleccione un formato");
            builder.setItems(formats, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    switch (which){
                        case 1:
                            if(_rec.encoderIsLoaded()) {
                                if(_rec.encoderRelProgress() == 1.0f){
                                    _rec.encoderFinish();
                                }
                            }
                            if(_rec.encoderIsLoaded()){
                                Log.e("OSAR", "other encoder is active");
                                Toast.makeText(getBaseContext(), getString(R.string.enc_bussy), Toast.LENGTH_SHORT).show();
                            } else {
                                try {
                                    File fileIn     = new File(getExternalCacheDir(), "OSSAudioRecorder.wav");
                                    File fileOut    = File.createTempFile("OSSAudioRecorder", ".flac", getExternalCacheDir());
                                    if(!_rec.encoderStart(fileIn.getAbsolutePath(), fileOut.getAbsolutePath(), "flac")){
                                        Log.e("OSAR", "encoder failed");
                                        Toast.makeText(getBaseContext(), getString(R.string.enc_err_start), Toast.LENGTH_SHORT).show();
                                    } else {
                                        _encFilename = fileOut.getName();
                                        syncUserInterface();
                                    }
                                } catch (Exception e){
                                    Toast.makeText(getBaseContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
                                }
                            }
                            break;
                        case 2:
                            if(_rec.encoderIsLoaded()) {
                                if(_rec.encoderRelProgress() == 1.0f){
                                    _rec.encoderFinish();
                                }
                            }
                            if(_rec.encoderIsLoaded()){
                                Log.e("OSAR", "other encoder is active");
                                Toast.makeText(getBaseContext(), getString(R.string.enc_bussy), Toast.LENGTH_SHORT).show();
                            } else {
                                try {
                                    File fileIn     = new File(getExternalCacheDir(), "OSSAudioRecorder.wav");
                                    File fileOut    = File.createTempFile("OSSAudioRecorder", ".opus", getExternalCacheDir());
                                    if(!_rec.encoderStart(fileIn.getAbsolutePath(), fileOut.getAbsolutePath(), "opus")){
                                        Log.e("OSAR", "encoder failed");
                                        Toast.makeText(getBaseContext(), getString(R.string.enc_err_start), Toast.LENGTH_SHORT).show();
                                    } else {
                                        _encFilename = fileOut.getName();
                                        syncUserInterface();
                                    }
                                } catch (Exception e){
                                    Toast.makeText(getBaseContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
                                }
                            }
                            break;
                        default:
                            try {
                                File fileIn     = new File(getExternalCacheDir(), "OSSAudioRecorder.wav");
                                File fileOut    = File.createTempFile("OSSAudioRecorder", ".wav", getExternalCacheDir());
                                //Copy file
                                {
                                    InputStream in = new FileInputStream(fileIn);
                                    try {
                                        OutputStream out = new FileOutputStream(fileOut);
                                        try {
                                            // Transfer bytes from in to out
                                            byte[] buf = new byte[1024];
                                            int len;
                                            while ((len = in.read(buf)) > 0) {
                                                out.write(buf, 0, len);
                                            }
                                        } catch (Exception e){
                                            Toast.makeText(getBaseContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
                                        } finally {
                                            out.close();
                                        }
                                    } catch (Exception e){
                                        Toast.makeText(getBaseContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
                                    } finally {
                                        in.close();
                                    }
                                }
                                btnShare(null, fileOut.getName());
                            } catch (Exception e){
                                Toast.makeText(getBaseContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
                            }
                            break;
                    }
                }
            });
            builder.show();
        }
    }

    public void btnShare(View view, String fileName) {
        Intent reqIntent = null;
        Intent srcIntent = getIntent();
        if(srcIntent != null){
            if(srcIntent.getAction() == MediaStore.Audio.Media.RECORD_SOUND_ACTION){
                reqIntent = srcIntent;
            }
        }
        try {
            File file = new File(getExternalCacheDir(), fileName);
            if(reqIntent != null){
                //Response to Intent
                Intent replyIntent = new Intent();
                replyIntent.setData(Uri.fromFile(file));
                setResult(RESULT_OK, replyIntent);
                finish();
            } else {
                //Start chooser
                Intent sendIntent = new Intent();
                sendIntent.setAction(Intent.ACTION_SEND);
                sendIntent.setType("audio/wav");
                sendIntent.putExtra("android.intent.extra.STREAM", Uri.fromFile(file));
                Intent shareIntent = Intent.createChooser(sendIntent, null);
                startActivity(shareIntent);
            }
        } catch (Exception e) {
            Toast.makeText(getBaseContext(), e.getMessage(),Toast.LENGTH_SHORT).show();
        }
    }

    public void btnPlay(View view){
        if(!_rec.isActive() && _rec.samplesCount() > 0) {
            if(_rec.playerIsLoaded()){
                _rec.playerStop();
            }
            if(!_rec.playerIsLoaded()) {
                File file = new File(this.getExternalCacheDir(), "OSSAudioRecorder.wav");
                if(!_rec.playerLoad(file.getAbsolutePath(), _buffersPerSec)) {
                    Log.e("OSAR", "_rec.playerLoad failed.");
                } else if(!_rec.playerPlay()) {
                    Log.e("OSAR", "_rec.playerPlay failed.");
                } else {
                    Log.i("OSAR", "_rec.btnPlay.");
                    this.syncUserInterface();
                }
            }
        }
    }

    public void btnMicAction(View view) {
        if(!_rec.isActive()){
            this.btnStart(view, true);
        } else if(_rec.isRecording()){
            this.btnPause(view);
        } else {
            this.btnResume(view);
        }
    }

    public void btnPlayBig(View view) {
        if(_rec.playerIsLoaded()){
            if(_rec.playerIsPaused()){
                Log.i("OSAR", "_rec.playerPlay.");
                _rec.playerPlay();
                this.syncUserInterface();
            } else {
                Log.i("OSAR", "_rec.playerPause.");
                _rec.playerPause();
                this.syncUserInterface();
            }
        }
    }

    private void updateTimeTag(float secs){
        int recMins = (int)secs / 60;
        secs -= (recMins * 60);
        {
            int recSecs = (int)secs;
            secs -= recSecs;
            {
                int recMs = (int)(secs * 1000);
                _txtTime.setText(
                        (recMins < 10 ? "0" : "") + Integer.toString(recMins) +
                        ":" +
                        (recSecs < 10 ? "0" : "") + Integer.toString(recSecs) /*+
                        "." +
                        (recMs < 100 ? "0" : "") + (recMs < 10 ? "0" : "") + Integer.toString(recMs)*/
                );
            }
        }
        _txtTimeValue = secs;
    }

    private void syncUserInterface(){
        int micVsb = View.VISIBLE;
        int playBigVsb = View.INVISIBLE;
        int timeVsb = View.VISIBLE;
        int playVsb = View.INVISIBLE;
        int stopVsb = View.INVISIBLE;
        int doneVsb = View.INVISIBLE;
        int deleteVsb = View.INVISIBLE;
        String title = "";
        //Collect props
        if(_rec.playerIsLoaded()){
            micVsb = View.INVISIBLE;
            playBigVsb = View.VISIBLE;
            stopVsb = View.VISIBLE;
            doneVsb = View.VISIBLE;
            if(_rec.playerIsPaused()){
                Drawable btnDraw = _btnPlayBig.getDrawable();
                btnDraw.setTint(this._colorMicYellow);
                title = getString(R.string.tit_paused);
            } else {
                Drawable btnDraw = _btnPlayBig.getDrawable();
                btnDraw.setTint(this._colorMicGreen);
                title = getString(R.string.tit_playing);
            }
        } else {
            if (_recError) {
                Drawable btnDraw = _btnMic.getDrawable();
                btnDraw.setTint(this._colorMicGray);
                timeVsb = View.INVISIBLE;
                title = getString(R.string.tit_err_retry);
            } else if (!_rec.isActive()) {
                Drawable btnDraw = _btnMic.getDrawable();
                btnDraw.setTint(this._colorMicGreen);
                if (_rec.samplesCount() <= 0) {
                    title = getString(R.string.tit_rec_start);
                } else {
                    title = getString(R.string.tit_rec_again);
                    playVsb = View.VISIBLE;
                    doneVsb = View.VISIBLE;
                    deleteVsb = View.VISIBLE;
                }
            } else if (!_rec.isRecording()) {
                Drawable btnDraw = _btnMic.getDrawable();
                btnDraw.setTint(this._colorMicYellow);
                title = getString(R.string.tit_rec_paused);
                stopVsb = View.VISIBLE;
            } else {
                Drawable btnDraw = _btnMic.getDrawable();
                btnDraw.setTint(this._colorMicRed);
                title = getString(R.string.tit_rec_active);
                stopVsb = View.VISIBLE;
            }
        }
        if(_rec.encoderIsLoaded()) {
            title = getString(R.string.tit_enc_active);
        }
        //Apply
        if(!_txtTittle.getText().equals(title)){
            _txtTittle.setText(title);
        }
        if(_btnMic.getVisibility() != micVsb){
            _btnMic.setVisibility(micVsb);
        }
        if(_graphSamples.getVisibility() != micVsb){
            _graphSamples.setVisibility(micVsb);
        }
        if(_btnPlayBig.getVisibility() != playBigVsb){
            _btnPlayBig.setVisibility(playBigVsb);
        }
        {
            int playTimelineVsb = playBigVsb;
            if(_rec.encoderIsLoaded()){
                playTimelineVsb = View.VISIBLE;
            }
            if(_playTimeline.getVisibility() != playTimelineVsb){
                _playTimeline.setVisibility(playTimelineVsb);
            }
        }
        if(_txtTime.getVisibility() != timeVsb){
            _txtTime.setVisibility(timeVsb);
        }
        if(_btnPlay.getVisibility() != playVsb){
            _btnPlay.setVisibility(playVsb);
        }
        if(_btnStop.getVisibility() != stopVsb){
            _btnStop.setVisibility(stopVsb);
        }
        if(_btnDone.getVisibility() != doneVsb){
            _btnDone.setVisibility(doneVsb);
        }
        if(_btnDelete.getVisibility() != deleteVsb){
            _btnDelete.setVisibility(deleteVsb);
        }
        //Lower buttons (layout visibles)
        {
            ImageButton btns[] = { _btnDone, _btnPlay, _btnStop, _btnDelete};
            int i, visibleCount = 0, visibleWidth = 0;
            //Count visibles
            for(i = 0; i < btns.length; i++){
                ImageButton btn = btns[i];
                if(btn.getVisibility() == View.VISIBLE) {
                    int width2 = btn.getWidth();
                    visibleWidth += width2;
                    visibleCount++;
                }
            }
            Log.i("OSAR", "visibleCount("+visibleCount+") visibleWidth("+visibleWidth+")");
            //Layout visibles
            if(visibleCount > 0){
                View layout = null;
                //Get parent layout
                {
                    ViewParent parent = _btnMic.getParent();
                    if (parent == null) {
                        Log.i("OSAR", "_btnMic.getParent() is null");
                    } else {
                        if (!(parent instanceof View)) {
                            Log.i("OSAR", "_btnMic.getParent() is not a View nor RelativeLayout");
                        } else {
                            layout = (View)parent;
                        }
                    }
                }
                //Layout
                if(layout != null) {
                    boolean requestLayout = false;
                    int width = layout.getWidth();
                    int marginH = (width * 20 / 100);
                    int xLeft = (width - visibleWidth - ((visibleCount - 1) * marginH)) / 2;
                    Log.i("", "width(" + width + ")");
                    for (i = 0; i < btns.length; i++) {
                         ImageButton btn = btns[i];
                         if(btn.getVisibility() == View.VISIBLE) {
                             int width2 = btn.getWidth();
                             android.widget.RelativeLayout.LayoutParams params = (android.widget.RelativeLayout.LayoutParams)btn.getLayoutParams();
                             if(params.leftMargin != xLeft){
                                 params.leftMargin = xLeft;
                                 requestLayout = true;
                                 Log.i("OSAR", "["+i+"].setLayoutParams(.leftMargin=" + xLeft + ") (applied)");
                             } else {
                                 Log.i("OSAR", "["+i+"].setLayoutParams(.leftMargin=" + xLeft + ") (ignored)");
                             }
                             xLeft += width2 + marginH;
                         }
                     }
                    //Request layout
                    if(requestLayout){
                        layout.requestLayout();
                        Log.i("OSAR", "requestLayout called");
                    }
                }
            }
        }
    }

}

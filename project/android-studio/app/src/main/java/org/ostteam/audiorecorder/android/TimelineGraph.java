package org.ostteam.audiorecorder.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

public class TimelineGraph extends View {

    private float _relPos = 0.0f;
    private float _relPosRequest = 0.0f;
    private boolean _relPosReqActive = false;

    private Paint _paintLine = null;
    private Paint _paintCursor = null;

    public TimelineGraph(Context context, AttributeSet attrs){
        super(context, attrs);
        //
        _paintLine = new Paint(0);
        _paintLine.setColor(Color.argb(255, 100, 100, 100));
        //
        _paintCursor = new Paint(0);
        _paintCursor.setColor(Color.argb(255, 82, 178, 64));
    }

    public void setRelPos(float relPos) {
        if(_relPos != relPos){
            _relPos = relPos;
            this.invalidate();
        }
    }

    public float getRelPosRequest(boolean reset){
        float r = -1.0f;
        if(_relPosReqActive){
            r = _relPosRequest;
            if(reset){
                _relPosReqActive = false;
            }
        }
        return r;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int width = canvas.getWidth();
        int height = canvas.getHeight();
        //Draw horizontal line
        {
            int lineHeight = height / 20;
            int lineTop = (height - lineHeight) / 2;
            canvas.drawRect(new Rect(0, lineTop, width, lineTop + lineHeight), _paintLine);
        }
        //Draw cursor
        {
            int cursorHeight = height * 8 / 10;
            int cursorWidth = width / 100;
            int cursorTop = (height - cursorHeight) / 2;
            int cursorScroll = (width - cursorWidth);
            int cursorLeft = (int)(cursorScroll * _relPos);
            canvas.drawRect(new Rect(cursorLeft, cursorTop, cursorLeft + cursorWidth, cursorTop + cursorHeight), _paintCursor);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        // MotionEvent reports input details from the touch screen
        // and other input controls. In this case, you are only
        // interested in events where the touch position changed.
        float x = e.getX();
        int width = this.getWidth();
        switch (e.getAction()) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_MOVE:
                float relX = ((float)x / (float)width);
                if(relX < 0.0f){
                    _relPosRequest = 0.0f;
                } else if(relX > 1.0f) {
                    _relPosRequest = 1.0f;
                } else {
                    _relPosRequest = relX;
                }
                _relPosReqActive = true;
                break;
        }
        return super.onTouchEvent(e);
    }
}

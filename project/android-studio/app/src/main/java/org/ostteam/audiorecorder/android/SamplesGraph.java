package org.ostteam.audiorecorder.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class SamplesGraph extends View {

    private final int _linesWidth = 8;
    private final int _linesSeparation = 2;
    private int _samplesFirst = 0;
    private int _samplesCount = 0;
    private float _samples[] = null; //circular array

    private Paint _paint = null;

    public SamplesGraph(Context context, AttributeSet attrs){
        super(context, attrs);
        //
        _paint = new Paint(0);
        _paint.setColor(Color.argb(255, 82, 178, 64));
    }

    public void resetGraph() {
        _samplesFirst = 0;
        _samplesCount = 0;
        this.invalidate();
    }

    public void addSampleRelAvg(float sampleRelAvg){
        if(_samples != null){
            if(_samplesCount == 0){
                _samplesFirst   = 0;
                _samplesCount   = 1;
                _samples[0]     = sampleRelAvg;
            } else {
                int iNext = (_samplesFirst + _samplesCount) % _samples.length;
                _samples[iNext] = sampleRelAvg;
                _samplesCount++;
                //Adjust circular bounds
                if(_samplesCount > _samples.length){
                    _samplesCount = _samples.length;
                    _samplesFirst++;
                    if(_samplesFirst >= _samples.length){
                        _samplesFirst = 0;
                    }
                }
            }
            this.invalidate();
        }
    }

    @Override
    protected void onSizeChanged (int w, int h, int oldw, int oldh){
        if(w != oldw || _samples == null) {
            if (w <= 0) {
                _samples = null;
            } else {
                float[] arrNew = new float[(w / (_linesWidth + _linesSeparation)) + 1];
                int i = arrNew.length;
                //Copy old samples
                if (_samples != null && _samplesCount > 0) {
                    int iOld = (_samplesFirst + _samplesCount) % _samples.length;
                    while (iOld != _samplesFirst && i > 0) {
                        arrNew[--i] = _samples[iOld];
                        iOld--;
                        if (iOld < 0) {
                            iOld = 0;
                        }
                    }
                }
                //Set new value
                _samplesCount = (arrNew.length - i);
                if (_samplesCount == 0) {
                    _samplesFirst = 0;
                } else {
                    _samplesFirst = i;
                }
                _samples = arrNew;
            }
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        //Draw lines
        if(_samples != null && _samplesCount > 0){
            int height = canvas.getHeight();
            int linesMidPoint = height / 2;
            int linesHeight = height * 4 / 9;
            float magnifyFactor = 8.0f;
            int x = canvas.getWidth() - _linesWidth, i = (_samplesFirst + _samplesCount - 1) % _samples.length;
            while (x >= 0) {
                float ss = _samples[i] * magnifyFactor;
                if(ss > 1) ss = 1;
                if(ss < -1) ss = -1;
                if(ss >= 0){
                    canvas.drawRect(new Rect(x, linesMidPoint + (int)(linesHeight * ss), x + _linesWidth, linesMidPoint), _paint);
                } else {
                    canvas.drawRect(new Rect(x, linesMidPoint, x + _linesWidth, linesMidPoint + (int)(linesHeight * ss)), _paint);
                }
                x -= _linesWidth + _linesSeparation;
                if(i == _samplesFirst){
                    break;
                }
                i--;
                if (i < 0) {
                    i = (_samples.length - 1);
                }
            }
        }
    }
}

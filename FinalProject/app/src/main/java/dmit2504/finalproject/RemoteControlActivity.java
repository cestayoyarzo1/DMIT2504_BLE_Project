package dmit2504.finalproject;


import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageButton;

public class RemoteControlActivity extends AppCompatActivity {

    ImageButton forwardButton;
    ImageButton reverseButton;
    ImageButton rightButton;
    ImageButton leftButton;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remote_control);
        forwardButton = findViewById(R.id.activity_remote_control_forward_button);
        reverseButton = findViewById(R.id.activity_remote_control_reverse_button);
        rightButton = findViewById(R.id.activity_remote_control_right_button);
        leftButton = findViewById(R.id.activity_remote_control_left_button);

        forwardButton.setOnTouchListener(buttonForwardListener);
        reverseButton.setOnTouchListener(buttonReverseListener);
        rightButton.setOnTouchListener(buttonRightListener);
        leftButton.setOnTouchListener(buttonLeftListener);

    }



    private View.OnTouchListener buttonForwardListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    forwardButton.animate().scaleX(1.1f);
                    forwardButton.animate().scaleY(1.1f);
                    forwardButton.setColorFilter(Color.argb(100, 0,255,0));
                    //Toast.makeText(getApplicationContext(), "Button FORWARD down", Toast.LENGTH_LONG).show();
                    break;

                case MotionEvent.ACTION_UP:
                    //Toast.makeText(getApplicationContext(), "Button FORWARD up", Toast.LENGTH_LONG).show();
                    forwardButton.animate().scaleX(1);
                    forwardButton.animate().scaleY(1);
                    //forwardButton.clearAnimation();
                    forwardButton.setColorFilter(Color.alpha(0));
                    break;
                }
            return false;
        }
    };

    private View.OnTouchListener buttonReverseListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    reverseButton.animate().scaleX(1.1f);
                    reverseButton.animate().scaleY(1.1f);
                    reverseButton.setColorFilter(Color.argb(100, 0,255,0));
                    //Toast.makeText(getApplicationContext(), "Button REVERSE down", Toast.LENGTH_LONG).show();
                    break;

                case MotionEvent.ACTION_UP:
                    reverseButton.animate().scaleX(1);
                    reverseButton.animate().scaleY(1);
                    reverseButton.setColorFilter(Color.alpha(0));
                    //Toast.makeText(getApplicationContext(), "Button REVERSE up", Toast.LENGTH_LONG).show();
                    break;
            }
            return false;
        }
    };

    private View.OnTouchListener buttonRightListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    rightButton.animate().scaleX(1.1f);
                    rightButton.animate().scaleY(1.1f);
                    rightButton.setColorFilter(Color.argb(100, 0,255,0));
                    //Toast.makeText(getApplicationContext(), "Button RIGHT down", Toast.LENGTH_LONG).show();
                    break;

                case MotionEvent.ACTION_UP:
                    rightButton.animate().scaleX(1);
                    rightButton.animate().scaleY(1);
                    rightButton.setColorFilter(Color.alpha(0));
                    //Toast.makeText(getApplicationContext(), "Button RIGHT up", Toast.LENGTH_LONG).show();
                    break;
            }
            return false;
        }
    };

    private View.OnTouchListener buttonLeftListener = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    leftButton.animate().scaleX(1.1f);
                    leftButton.animate().scaleY(1.1f);
                    leftButton.setColorFilter(Color.argb(100, 0,255,0));
                    //Toast.makeText(getApplicationContext(), "Button LEFT down", Toast.LENGTH_LONG).show();
                    break;

                case MotionEvent.ACTION_UP:
                    leftButton.animate().scaleX(1);
                    leftButton.animate().scaleY(1);
                    leftButton.setColorFilter(Color.alpha(0));
                    //Toast.makeText(getApplicationContext(), "Button LEFT up", Toast.LENGTH_LONG).show();
                    break;
            }
            return false;
        }
    };
}
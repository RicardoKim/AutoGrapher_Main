package com.clj.autographer.camera;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.clj.autographer.R;

public class CameraMainActivity2 extends AppCompatActivity implements View.OnClickListener {
    ImageView imageViewPose;
    LinearLayout btnRefresh;
    TextView angleTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.camera_main_2);

        imageViewPose = findViewById(R.id.imageView2);
        angleTextView = findViewById(R.id.angleText);
        btnRefresh = findViewById(R.id.refresh);

        btnRefresh.setOnClickListener(this);

        Intent intent = getIntent();
        String text = intent.getStringExtra("Text");
        angleTextView.setText(text);

        Singleton singleton = Singleton.getInstance();
        imageViewPose.setImageBitmap(singleton.getMyImage());
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.refresh:
                backMain();
                break;
        }
    }

    private void backMain() {
        Intent intent = new Intent(CameraMainActivity2.this, CameraMainActivity.class);
        startActivity(intent);
    }
}

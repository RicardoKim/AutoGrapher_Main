package com.clj.autographer.camera;

import static com.clj.autographer.operation.CharacteristicOperationFragment.hexToString;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothSocket;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PointF;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import com.clj.autographer.R;
import com.clj.fastble.BleManager;
import com.clj.fastble.bluetooth.BleBluetooth;
import com.clj.fastble.callback.BleReadCallback;
import com.clj.fastble.callback.BleWriteCallback;
import com.clj.fastble.data.BleDevice;
import com.clj.fastble.exception.BleException;
import com.clj.fastble.utils.HexUtil;
import com.github.ybq.android.spinkit.style.FadingCircle;
import com.google.android.gms.tasks.OnFailureListener;
import com.google.android.gms.tasks.OnSuccessListener;
import com.google.mlkit.vision.common.InputImage;
import com.google.mlkit.vision.pose.Pose;
import com.google.mlkit.vision.pose.PoseDetection;
import com.google.mlkit.vision.pose.PoseDetector;
import com.google.mlkit.vision.pose.PoseLandmark;
import com.google.mlkit.vision.pose.accurate.AccuratePoseDetectorOptions;
import com.wonderkiln.camerakit.CameraKitError;
import com.wonderkiln.camerakit.CameraKitEvent;
import com.wonderkiln.camerakit.CameraKitEventCallback;
import com.wonderkiln.camerakit.CameraKitEventListener;
import com.wonderkiln.camerakit.CameraKitImage;
import com.wonderkiln.camerakit.CameraKitVideo;
import com.wonderkiln.camerakit.CameraView;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.List;

public class CameraMainActivity  extends AppCompatActivity implements View.OnClickListener {
    LinearLayout cameraButton, galleryButton;
    CameraView cameraViewPose;
    ProgressBar progressBar;
    Intent intent;
    BleDevice bleDevice;
    BluetoothGattCharacteristic characteristic;
    BluetoothGattService characteristic_service;
    String IMAGES_FOLDER_NAME = "Camera";
    private int image_width = 0;
    private int image_height = 0;
    private InputStream inputStream;

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.camera_main);
        intent = getIntent();
        bleDevice = (BleDevice) intent.getExtras().get("bleDevice");
        characteristic = (BluetoothGattCharacteristic) intent.getExtras().get("characteristic");
        characteristic_service = (BluetoothGattService) intent.getExtras().get("characteristic_service");
        cameraButton = findViewById(R.id.cameraBtn);
        galleryButton = findViewById(R.id.galleryBtn);
        cameraViewPose = findViewById(R.id.poseCamera);

        // Progress Bar
        progressBar = findViewById(R.id.spin_kit);
        FadingCircle fadingCircle = new FadingCircle();
        progressBar.setIndeterminateDrawable(fadingCircle);
        progressBar.setVisibility(View.INVISIBLE);

        // OnClick
        cameraButton.setOnClickListener(this);
        galleryButton.setOnClickListener(this);


        cameraViewPose.addCameraKitListener(new CameraKitEventListener() {
            @Override
            public void onEvent(CameraKitEvent cameraKitEvent) {

            }

            @Override
            public void onError(CameraKitError cameraKitError) {

            }

            @Override
            public void onImage(CameraKitImage cameraKitImage) {
                progressBar.setVisibility(View.VISIBLE);
                Bitmap bitmap = cameraKitImage.getBitmap();
                image_height = cameraViewPose.getHeight();;
                image_width = cameraViewPose.getWidth();
                bitmap = Bitmap.createScaledBitmap(bitmap, cameraViewPose.getWidth(), cameraViewPose.getHeight(), false);
                cameraViewPose.stop();
                runPose(bitmap);
                progressBar.setVisibility(View.INVISIBLE);
            }

            @Override
            public void onVideo(CameraKitVideo cameraKitVideo) {

            }
        });
        BackgroundThread thread = new BackgroundThread();
        thread.start();

    }

    // OnClick
    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.cameraBtn:
                poseDetect();
                break;
            case R.id.galleryBtn:
                galleryAdd();
                break;
        }
    }


    final int REQUEST_GALLERY = 1;
    private void galleryAdd() {
        Intent mediaIntent = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(mediaIntent, REQUEST_GALLERY);
    }

    Bitmap galleryImage;
    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        switch (requestCode){
            case REQUEST_GALLERY:
                try {
                    Uri imageUri = data.getData();
                    InputStream inputStream = getContentResolver().openInputStream(imageUri);
                    galleryImage = BitmapFactory.decodeStream(inputStream);
                    runPose(galleryImage);
                    progressBar.setVisibility(View.VISIBLE);
                }catch (Exception e){
                    Toast.makeText(CameraMainActivity.this, "Görsel Galeriden Alınamadı",Toast.LENGTH_SHORT).show();
                }
                break;
        }
    }


    // camera pos Detect
    private void poseDetect() {
        cameraViewPose.start();
        cameraViewPose.captureImage();
    }

    @Override
    protected void onResume() {
        super.onResume();
        cameraViewPose.start();
    }

    @Override
    protected void onStop() {
        super.onStop();
        cameraViewPose.stop();
    }

    @Override
    protected void onPause() {
        super.onPause();
        cameraViewPose.stop();
    }

    // Pose Detect
    AccuratePoseDetectorOptions options =
            new AccuratePoseDetectorOptions.Builder()
                    .setDetectorMode(AccuratePoseDetectorOptions.SINGLE_IMAGE_MODE)
                    .build();

    PoseDetector poseDetector = PoseDetection.getClient(options);

    Bitmap resizedBitmap;

    private void runPose(Bitmap bitmap) {

        int rotationDegree = 0;

        int width = bitmap.getWidth();
        int height = bitmap.getHeight();

        resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, width, height);
        Log.e( "runPose: ", String.valueOf(width) + "|" + String.valueOf(height));

        InputImage image = InputImage.fromBitmap(resizedBitmap, rotationDegree);

        poseDetector.process(image)
                .addOnSuccessListener(
                        new OnSuccessListener<Pose>() {
                            @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
                            @Override
                            public void onSuccess(Pose pose) {
                                processPose(pose);
                            }
                        })
                .addOnFailureListener(
                        new OnFailureListener() {
                            @Override
                            public void onFailure(@NonNull Exception e) {
                                Toast.makeText(CameraMainActivity.this, "Pose Tespit Edilemedi",Toast.LENGTH_SHORT).show();
                            }
                        });
    }

    private void takephoto(){
        cameraViewPose.captureImage(
                new CameraKitEventCallback<CameraKitImage>() {
                    @Override
                    public void callback(CameraKitImage cameraKitImage) {

                        Bitmap result = cameraKitImage.getBitmap();
                        try {
                            saveImage(result, "카메라");
                            cameraViewPose.start();
                        } catch (IOException e) {
                            e.printStackTrace();
                            cameraViewPose.start();
                        }
                    }
                }
        );
    }
    private void saveImage(Bitmap bitmap, @NonNull String name) throws IOException {
        boolean saved;
        OutputStream fos;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            ContentResolver resolver = getContentResolver();
            ContentValues contentValues = new ContentValues();
            contentValues.put(MediaStore.MediaColumns.DISPLAY_NAME, name);
            contentValues.put(MediaStore.MediaColumns.MIME_TYPE, "image/png");
            contentValues.put(MediaStore.MediaColumns.RELATIVE_PATH, "DCIM/Camera");
            Uri imageUri = resolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, contentValues);
            fos = resolver.openOutputStream(imageUri);
        } else {
            String imagesDir = Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DCIM).toString() + File.separator + IMAGES_FOLDER_NAME;

            File file = new File(imagesDir);

            if (!file.exists()) {
                file.mkdir();
            }

            File image = new File(imagesDir, name + ".png");
            fos = new FileOutputStream(image);

        }

        saved = bitmap.compress(Bitmap.CompressFormat.PNG, 100, fos);
        fos.flush();
        fos.close();
    }

    // Pose Detect
    String angleText;

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    private void processPose(Pose pose) {
        try {
            PoseLandmark leftShoulder = pose.getPoseLandmark(PoseLandmark.LEFT_SHOULDER);
            PoseLandmark rightShoulder = pose.getPoseLandmark(PoseLandmark.RIGHT_SHOULDER);


            PoseLandmark leftElbow = pose.getPoseLandmark(PoseLandmark.LEFT_ELBOW);
            PoseLandmark rightElbow = pose.getPoseLandmark(PoseLandmark.RIGHT_ELBOW);


            PoseLandmark leftWrist = pose.getPoseLandmark(PoseLandmark.LEFT_WRIST);
            PoseLandmark rightWrist = pose.getPoseLandmark(PoseLandmark.RIGHT_WRIST);


            PoseLandmark leftHip = pose.getPoseLandmark(PoseLandmark.LEFT_HIP);
            PoseLandmark rightHip = pose.getPoseLandmark(PoseLandmark.RIGHT_HIP);


            PoseLandmark leftKnee = pose.getPoseLandmark(PoseLandmark.LEFT_KNEE);
            PoseLandmark rightKnee = pose.getPoseLandmark(PoseLandmark.RIGHT_KNEE);


            PoseLandmark leftAnkle = pose.getPoseLandmark(PoseLandmark.LEFT_ANKLE);
            PoseLandmark rightAnkle = pose.getPoseLandmark(PoseLandmark.RIGHT_ANKLE);

            PointF leftShoulderP = leftShoulder.getPosition();

            float lShoulderX = leftShoulderP.x;
            System.out.println(lShoulderX);
            System.out.println(image_width);
            lShoulderX = (int) (lShoulderX *100/ image_width);


            int lShoulderY = (int) leftShoulderP.y;
            lShoulderY = (int) (lShoulderY *100/ image_height);


            PointF rightSoulderP = rightShoulder.getPosition();

            int rShoulderX = (int)rightSoulderP.x;
            rShoulderX = (int) (rShoulderX *100/ image_width);

            int rShoulderY = (int)rightSoulderP.y;
            rShoulderY = (int) (rShoulderY * 100 / image_height);

            PointF leftAnkleP = leftAnkle.getPosition();
            int lAnkleX = (int) leftAnkleP.x;

            lAnkleX = (int) (lAnkleX * 100 / image_width);

            int lAnkleY = (int) leftAnkleP.y;

            lAnkleY = (int) (lAnkleY * 100 / image_height);

            PointF rightAnkleP = rightAnkle.getPosition();
            int rAnkleX = (int) rightAnkleP.x;
            rAnkleX = (int) (rAnkleX * 100 / image_width);

            int rAnkleY = (int) rightAnkleP.y;

            rAnkleY = (int) (rAnkleY * 100 / image_height);

            String data = lShoulderX + "," + lShoulderY +"," + rShoulderX +"," + rShoulderY +"," + lAnkleX +"," + lAnkleY +"," + rAnkleX +"," + rAnkleY;
            System.out.println("Processed data : " + data);
            sendingDataToHM10(data);
            cameraViewPose.start();

        }catch (Exception e){
            Toast.makeText(CameraMainActivity.this, "Pose Estimation Finish",Toast.LENGTH_SHORT).show();
            progressBar.setVisibility(View.INVISIBLE);
            cameraViewPose.start();
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    private void sendingDataToHM10(String sendingValue){
        List<BluetoothGattCharacteristic> characteristicList = BleManager.getInstance().getBluetoothGattCharacteristics(characteristic_service);
        BluetoothGattCharacteristic bluetoothGattCharacteristic = characteristicList.get(0);
        byte[] jsonByte = sendingValue.getBytes();
        BleManager.getInstance().write(
                bleDevice,
                bluetoothGattCharacteristic.getService().getUuid().toString(),
                bluetoothGattCharacteristic.getUuid().toString(),
                jsonByte,
                new BleWriteCallback() {

                    @Override
                    public void onWriteSuccess(final int current, final int total, final byte[] justWrite) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {

                            }
                        });
                    }

                    @Override
                    public void onWriteFailure(final BleException exception) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                System.out.println(exception.toString());
                            }
                        });
                    }
                });
    }


    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    private byte[] JSONToByteArray(JSONObject poseInformation){
        return poseInformation.toString().getBytes(StandardCharsets.UTF_8);
    }


    class BackgroundThread extends Thread {
        boolean running = false;
        @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
        public void run() {
            running = true;
            while(running) {
                List<BluetoothGattCharacteristic> characteristicList = BleManager.getInstance().getBluetoothGattCharacteristics(characteristic_service);
                final BluetoothGattCharacteristic bluetoothGattCharacteristic = characteristicList.get(0);
                BleManager.getInstance().read(
                        bleDevice,
                        bluetoothGattCharacteristic.getService().getUuid().toString(),
                        bluetoothGattCharacteristic.getUuid().toString(),
                        new BleReadCallback() {
                            @Override
                            public void onReadSuccess(final byte[] data) {
                                runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                        String command = hexToString(HexUtil.formatHexString(data, false));

                                        try{
                                            if(command == " 2" || Integer.parseInt(command) == 2){
                                                poseDetect();
                                            }
                                            else if(command == " 3" || Integer.parseInt(command) == 3){
                                                takephoto();
                                            }
                                            System.gc();
                                        }catch(Exception e){
                                            System.out.println(e.toString());
                                        }

                                    }

                                });
                            }

                            @Override
                            public void onReadFailure(final BleException exception) {
                                runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                    }
                                });
                            }


                        });
                try{
                    Thread.sleep(1000);
                }catch(Exception e){

                }

            }
        }
    }
}
